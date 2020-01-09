#!/usr/bin/env python
# -*- coding: utf-8 -*-

#############################################################################
##
## Copyright 2007-2009 Canonical Ltd
## Copyright 2008-2009 Richard Birnie <arbyuk@googlemail.com>
## Author: Jonathan Riddell <jriddell@ubuntu.com>
##         Richard Birnie <arbyuk@googlemail.com>
##
## Includes code from System Config Printer:
## Copyright (C) 2006, 2007, 2008 Red Hat, Inc.
## Copyright (C) 2006, 2007 Florian Festi <ffesti@redhat.com>
## Copyright (C) 2006, 2007, 2008 Tim Waugh <twaugh@redhat.com>
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2 of
## the License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##
#############################################################################

MIN_REFRESH_INTERVAL = 1 # seconds
import locale

import sys, os, time, traceback, re, tempfile, httplib, thread

#load modules from system-config-printer-common (debug, smburi), change path here if you have it installed elsewhere
SYSTEM_CONFIG_PRINTER_DIR = "/usr/share/system-config-printer"
if os.path.exists(SYSTEM_CONFIG_PRINTER_DIR + "/debug.py"):
    sys.path.append(SYSTEM_CONFIG_PRINTER_DIR)

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import uic

from PyKDE4.kdecore import *
from PyKDE4.kdeui import *
from PyKDE4.kio import *

#use _() to keep code the same as gnome system-config-printer
def _(string):
    return unicode(i18n(string), "utf-8")

def translate(self, prop):
    """reimplement method from uic to change it to use gettext"""
    if prop.get("notr", None) == "true":
        return self._cstring(prop)
    else:
        if prop.text is None:
            return ""
        text = prop.text.encode("UTF-8")
        return i18n(text)

uic.properties.Properties._string = translate

import cups
cups.require ("1.9.27")

# These come from system-config-printer
import config
import cupshelpers, options
from optionwidgets import OptionWidget
from smburi import SMBURI
from debug import *

import dbus
import dbus.mainloop.qt
import dbus.service
import urllib

ellipsis = unichr(0x2026)

try:
    try_CUPS_SERVER_REMOTE_ANY = cups.CUPS_SERVER_REMOTE_ANY
except AttributeError:
    # cups module was compiled with CUPS < 1.3
    try_CUPS_SERVER_REMOTE_ANY = "_remote_any"

def validDeviceURI (uri):
    """Returns True is the provided URI is valid."""
    if uri.find (":/") > 0:
        return True
    return False

if os.path.exists("system-config-printer.ui"):
    APPDIR = QDir.currentPath()
else:
    file = KStandardDirs.locate("data", "system-config-printer-kde/system-config-printer.ui")
    APPDIR = file.left(file.lastIndexOf("/"))

class PyKcm(KCModule):
    def __init__(self, component_data, parent, gui):
        KCModule.__init__(self, component_data, parent)

        uic.loadUi(APPDIR + "/" + "system-config-printer.ui", self)
        self.setButtons(KCModule.Apply)
        self.gui = gui
        
    def changed(self, state):
        """a setting has changed, activate the Apply button"""
        self.emit(SIGNAL("changed(bool)"), state)

    def save(self):
        self.gui.on_btnPrinterPropertiesApply_clicked()

    def load(self):
        self.gui.on_btnRevert_clicked()

class GUI(QWidget):
    """our main class is the main window"""

    printer_states = { cups.IPP_PRINTER_IDLE: i18nc("Printer state", "Idle"),
                       cups.IPP_PRINTER_PROCESSING: i18nc("Printer state", "Processing"),
                       cups.IPP_PRINTER_BUSY: i18nc("Printer state", "Busy"),
                       cups.IPP_PRINTER_STOPPED: i18nc("Printer state", "Stopped") }


    def makeui(self, component_data, parent):
        self.ui = PyKcm(component_data, parent, self)
        start_printer = None
        change_ppd = False
        
        try:
            self.language = locale.getlocale(locale.LC_MESSAGES)
            self.encoding = locale.getlocale(locale.LC_CTYPE)
        except:
            nonfatalException()
            os.environ['LC_ALL'] = 'C'
            locale.setlocale (locale.LC_ALL, "")
            self.language = locale.getlocale(locale.LC_MESSAGES)
            self.encoding = locale.getlocale(locale.LC_CTYPE)

        self.printer = None
        self.conflicts = set() # of options
        self.connect_server = (self.printer and self.printer.getServer()) \
                               or cups.getServer()        
        self.connect_user = cups.getUser()
        self.password = '' #FIXME not in Gnome version
        self.passwd_retry = False #FIXME not in Gnome version
        self.widget_data_setting = {} #FIXME not in Gnome version
        #FIXMEcups.setPasswordCB(self.cupsPasswdCallback)        
        ##self.server_is_publishing = False #FIXME new in Gnome version

        self.changed = set() # of options

        self.servers = set((self.connect_server,))

        try:
            self.cups = cups.Connection()
        except RuntimeError:
            self.cups = None

        # New Printer Dialog
        self.newPrinterGUI = np = NewPrinterGUI(self)
        
        self.setConnected()

        self.ui.connect(self.ui.mainlist, SIGNAL("itemSelectionChanged()"), self.on_tvMainList_cursor_changed)
        self.ui.connect(self.ui.mainlist, SIGNAL("currentItemChanged (QTreeWidgetItem*, QTreeWidgetItem*)"), self.on_tvMainList_changed)
        self.ui.connect(self.ui.chkServerBrowse, SIGNAL("stateChanged(int)"), self.on_server_widget_changed)
        self.ui.connect(self.ui.chkServerShare, SIGNAL("stateChanged(int)"), self.on_server_widget_changed)
        self.ui.connect(self.ui.chkServerShareAny, SIGNAL("stateChanged(int)"), self.on_server_widget_changed)
        self.ui.connect(self.ui.chkServerRemoteAdmin, SIGNAL("stateChanged(int)"), self.on_server_widget_changed)
        self.ui.connect(self.ui.chkServerAllowCancelAll, SIGNAL("stateChanged(int)"), self.on_server_widget_changed)
        self.ui.connect(self.ui.chkServerLogDebug, SIGNAL("stateChanged(int)"), self.on_server_widget_changed)

        self.ui.connect(self.ui.btnChangePPD, SIGNAL("clicked()"), self.on_btnChangePPD_clicked)
        self.ui.connect(self.ui.btnNewClass, SIGNAL("clicked()"), self.on_new_class_activate)
        self.ui.connect(self.ui.btnNewPrinterNetwork, SIGNAL("clicked()"), self.on_new_printer_activate)
        self.ui.connect(self.ui.btnPrintTestPage, SIGNAL("clicked()"), self.on_btnPrintTestPage_clicked)
        self.ui.connect(self.ui.btnSelfTest, SIGNAL("clicked()"), self.on_btnSelfTest_clicked)
        self.ui.connect(self.ui.btnCleanHeads, SIGNAL("clicked()"), self.on_btnCleanHeads_clicked)

        self.ui.connect(self.ui.btnNewJobOption, SIGNAL("clicked()"), self.on_btnNewJobOption_clicked)
        self.ui.connect(self.ui.entNewJobOption, SIGNAL("textChanged(QString)"), self.on_entNewJobOption_changed)

        self.ui.connect(self.ui.entPDescription, SIGNAL("textEdited(const QString&)"), self.on_printer_changed)
        self.ui.connect(self.ui.entPLocation, SIGNAL("textEdited(const QString&)"), self.on_printer_changed)
        self.ui.connect(self.ui.chkPMakeDefault, SIGNAL("stateChanged(int)"), self.chkPMakeDefault_stateChanged)
        self.ui.connect(self.ui.btnDelete, SIGNAL("clicked()"), self.btnDelete_clicked)
        self.ui.connect(self.ui.entPDevice, SIGNAL("textEdited(const QString&)"), self.on_printer_changed)
        self.ui.connect(self.ui.chkPEnabled, SIGNAL("stateChanged(int)"),self.on_printer_changed)
        self.ui.connect(self.ui.chkPAccepting, SIGNAL("stateChanged(int)"), self.on_printer_changed)
        self.ui.connect(self.ui.chkPShared, SIGNAL("stateChanged(int)"), self.on_printer_changed)
        self.ui.connect(self.ui.cmbPStartBanner, SIGNAL("currentIndexChanged(int)"), self.on_printer_changed)
        self.ui.connect(self.ui.cmbPEndBanner, SIGNAL("currentIndexChanged(int)"), self.on_printer_changed)
        self.ui.connect(self.ui.rbtnPAllow, SIGNAL("toggled(bool)"), self.on_printer_changed)
        self.ui.connect(self.ui.btnPAddUser, SIGNAL("clicked()"), self.btnPAddUser_clicked)
        self.ui.connect(self.ui.btnPDelUser, SIGNAL("clicked()"), self.btnPDelUser_clicked)

        self.ui.connect(self.ui.btnClassAddMember, SIGNAL("clicked()"), self.on_btnClassAddMember_clicked)
        self.ui.connect(self.ui.btnClassDelMember, SIGNAL("clicked()"), self.on_btnClassDelMember_clicked)

        self.ui.mainlist.header().hide()

        self.policiesTabWidget = self.ui.ntbkPrinter.widget(1)
        self.memberTabWidget = self.ui.ntbkPrinter.widget(2)
        self.optionsTabWidget = self.ui.ntbkPrinter.widget(3)

        self.ui.btnClassAddMember.setIcon(KIcon("arrow-left"))
        self.ui.btnClassDelMember.setIcon(KIcon("arrow-right"))
        self.mainListSelectedName = None
        self.mainListSelectedType = None

        # Job Options widgets.
        opts = [ options.OptionAlwaysShown ("copies", int, 1,
                                            self.ui.sbJOCopies,
                                            self.ui.btnJOResetCopies),
                 options.OptionAlwaysShownSpecial \
                 ("orientation-requested", int, 3,
                  self.ui.cmbJOOrientationRequested,
                  self.ui.btnJOResetOrientationRequested,
                  combobox_map = [3, 4, 5, 6],
                  special_choice=_("Automatic rotation")),

                 options.OptionAlwaysShown ("fitplot", bool, False,
                                            self.ui.cbJOFitplot,
                                            self.ui.btnJOResetFitplot),

                 options.OptionAlwaysShown ("number-up", int, 1,
                                            self.ui.cmbJONumberUp,
                                            self.ui.btnJOResetNumberUp,
                                            combobox_map=[1, 2, 4, 6, 9, 16]),

                 options.OptionAlwaysShown ("number-up-layout", str, "lrtb",
                                            self.ui.cmbJONumberUpLayout,
                                            self.ui.btnJOResetNumberUpLayout,
                                            combobox_map = [ "lrtb",
                                                             "lrbt",
                                                             "rltb",
                                                             "rlbt",
                                                             "tblr",
                                                             "tbrl",
                                                             "btlr",
                                                             "btrl" ]),

                 options.OptionAlwaysShown ("brightness", int, 100,
                                            self.ui.sbJOBrightness,
                                            self.ui.btnJOResetBrightness),

                 options.OptionAlwaysShown ("finishings", int, 3,
                                            self.ui.cmbJOFinishings,
                                            self.ui.btnJOResetFinishings,
                                            combobox_map = [ 3, 4, 5, 6,
                                                             7, 8, 9, 10,
                                                             11, 12, 13, 14,
                                                             20, 21, 22, 23,
                                                             24, 25, 26, 27,
                                                             28, 29, 30, 31,
                                                             50, 51, 52, 53 ],
                                            use_supported = True),

                 options.OptionAlwaysShown ("job-priority", int, 50,
                                            self.ui.sbJOJobPriority,
                                            self.ui.btnJOResetJobPriority),

                 options.OptionAlwaysShown ("media", str,
                                            "A4", # This is the default for
                                                  # when media-default is
                                                  # not supplied by the IPP
                                                  # server.  Fortunately it
                                                  # is a mandatory attribute.
                                            self.ui.cmbJOMedia,
                                            self.ui.btnJOResetMedia,
                                            use_supported = True),

                 options.OptionAlwaysShown ("sides", str, "one-sided",
                                            self.ui.cmbJOSides,
                                            self.ui.btnJOResetSides,
                                            combobox_map =
                                            [ "one-sided",
                                              "two-sided-long-edge",
                                              "two-sided-short-edge" ]),

                 options.OptionAlwaysShown ("job-hold-until", str,
                                            "no-hold",
                                            self.ui.cmbJOHoldUntil,
                                            self.ui.btnJOResetHoldUntil,
                                            use_supported = True),

                 options.OptionAlwaysShown ("mirror", bool, False,
                                            self.ui.cbJOMirror,
                                            self.ui.btnJOResetMirror),

                 options.OptionAlwaysShown ("scaling", int, 100,
                                            self.ui.sbJOScaling,
                                            self.ui.btnJOResetScaling),


                 options.OptionAlwaysShown ("saturation", int, 100,
                                            self.ui.sbJOSaturation,
                                            self.ui.btnJOResetSaturation),

                 options.OptionAlwaysShown ("hue", int, 0,
                                            self.ui.sbJOHue,
                                            self.ui.btnJOResetHue),

                 options.OptionAlwaysShown ("gamma", int, 1000,
                                            self.ui.sbJOGamma,
                                            self.ui.btnJOResetGamma),

                 options.OptionAlwaysShown ("cpi", float, 10.0,
                                            self.ui.sbJOCpi, self.ui.btnJOResetCpi),

                 options.OptionAlwaysShown ("lpi", float, 6.0,
                                            self.ui.sbJOLpi, self.ui.btnJOResetLpi),

                 options.OptionAlwaysShown ("page-left", int, 18,
                                            self.ui.sbJOPageLeft,
                                            self.ui.btnJOResetPageLeft),

                 options.OptionAlwaysShown ("page-right", int, 18,
                                            self.ui.sbJOPageRight,
                                            self.ui.btnJOResetPageRight),

                 options.OptionAlwaysShown ("page-top", int, 36,
                                            self.ui.sbJOPageTop,
                                            self.ui.btnJOResetPageTop),

                 options.OptionAlwaysShown ("page-bottom", int, 36,
                                            self.ui.sbJOPageBottom,
                                            self.ui.btnJOResetPageBottom),

                 options.OptionAlwaysShown ("prettyprint", bool, False,
                                            self.ui.cbJOPrettyPrint,
                                            self.ui.btnJOResetPrettyPrint),

                 options.OptionAlwaysShown ("wrap", bool, False, self.ui.cbJOWrap,
                                            self.ui.btnJOResetWrap),

                 options.OptionAlwaysShown ("columns", int, 1,
                                            self.ui.sbJOColumns,
                                            self.ui.btnJOResetColumns),
                 ]
        self.job_options_widgets = {}
        self.job_options_buttons = {}
        for option in opts:
            self.job_options_widgets[option.widget] = option
            self.job_options_buttons[option.button] = option
            self.connect(option.button, SIGNAL("clicked()"), self.on_job_option_reset)
            if type(option.widget) == QSpinBox:
                self.connect(option.widget, SIGNAL("valueChanged(int)"), self.on_job_option_changed)
            elif type(option.widget) == QComboBox:
                self.connect(option.widget, SIGNAL("currentIndexChanged(int)"), self.on_job_option_changed)
            elif type(option.widget) == QCheckBox:
                self.connect(option.widget, SIGNAL("stateChanged(int)"), self.on_job_option_changed)
            else:
                raise RuntimeError

        try:
            self.populateList(start_printer, change_ppd)
        except cups.HTTPError, (s,):
            self.cups = None
            self.setConnected()
            self.populateList()
            self.show_HTTP_Error(s)

        #hide some bits until implemented
        self.ui.btnNewPrinter.hide()
        self.ui.newPrinterLabel.hide()
        self.ui.btnNewPrinterSpecial.hide()
        self.ui.newPrinterSpecialLabel.hide()
        self.ui.entPName.hide()
        self.ui.lblPName.hide()

        #catch the currentItemChanged signal. This should stop the
        # 'Do you want to save settings' pop-up at startup
        self.ui.mainlist.blockSignals(True)
        self.ui.mainlist.setCurrentItem(self.settingsItem)
        self.ui.mainlist.blockSignals(False) #unblock the signal
        
        #FIXME hide labels until implemented
        self.ui.lblPOptions.hide()
        self.ui.lblPInstallOptions.hide()

        return self.ui

    # now called  dests_iconview_item_activated() in the Gnome version
    def on_tvMainList_cursor_changed(self):
        if self.changed:
            # The unapplied changes for this item have not been saved,
            # and the user just pressed "Cancel".
            #FIXME, should offer dialog prompting to save or cancel here
            return
        items = self.ui.mainlist.selectedItems()

        if len(items) < 1:
            return
        item = items[0]

        #FIXME only show settings until ready for the rest
        #item = self.settingsItem
        type = unicode(item.text(1))
        name = unicode(item.text(0))

        #name, type = self.getSelectedItem()
        #model, self.mainListSelected = self.tvMainList.get_selection().get_selected()
        #Save the values incase it gets deselected
        self.mainListSelectedType = type
        self.mainListSelectedName = name
        item_selected = True
        if type == "New":
            self.ui.ntbkMain.setCurrentIndex(0)
        elif type == "Settings":
            self.ui.ntbkMain.setCurrentIndex(1)
            if self.cups:
                self.fillServerTab()
            else:
                # No connection to CUPS.  Make sure the Apply/Revert buttons
                # are not sensitive.
                self.setDataButtonState()
            item_selected = False
        elif type in ['Printer', 'Class']:
            try:
                self.fillPrinterTab(name)
                # the below not needed, fillPrinterTab() already calls fillPrinterOptions()
                ##self.fillPrinterOptions("foo", True) #FIXME parameters need fixing
                self.setDataButtonState()
                self.ui.ntbkPrinter.setCurrentIndex(0)
            except RuntimeError:
                # Perhaps cupsGetPPD2 failed for a browsed printer.
                self.ui.ntbkMain.setCurrentIndex(3)
                return

            self.ui.printerNameLabel.setText(name)
            self.ui.ntbkMain.setCurrentIndex(2)
        elif type == "None":
            self.ui.ntbkMain.setCurrentIndex(3)
            self.setDataButtonState()
            item_selected = False

        """FIXME, copy button
        is_local = item_selected and not self.printers[name].discovered
        for widget in [self.copy, self.btnCopy]:
            widget.set_sensitive(item_selected)
        for widget in [self.delete, self.btnDelete]:
            widget.set_sensitive(is_local)
        """

    def printer_properties_response(self):
        name, type = self.getSelectedItem()
        if type in ("Printer", "Class"):
            return self.save_printer(self.printer)
        elif type == "Settings":
            return self.save_serversettings()

    @pyqtSignature("")
    def on_tvMainList_changed(self, new, old):
        """about to change, offer to save"""
        if self.changed:
            answer = KMessageBox.questionYesNo(self, "Do you want to save changes?", "Save Changes", KStandardGuiItem.save(), KStandardGuiItem.discard() )
    
            if answer == KMessageBox.Yes: #save is equivalent to yes
                self.printer_properties_response()
            elif answer == KMessageBox.No: #discard is equivalent to no
                self.changed = set() # avoid asking the user

    def busy (self, win = None):
        try:
            if not win:
                win = self
            win.setCursor(Qt.WaitCursor)
            KApplication.processEvents()
        except:
            nonfatalException ()

    def ready (self, win = None):
        try:
            if not win:
                win = self
            win.setCursor(Qt.ArrowCursor)
            KApplication.processEvents()
        except:
            nonfatalException ()

    def setConnected(self):
        connected = bool(self.cups)

        host = cups.getServer()

        if host[0] == '/':
            host = 'localhost'
        self.setWindowTitle(i18n("Printer configuration - %1", host))

        if connected:
            status_msg = i18n("Connected to %1", host)
        else:
            status_msg = i18n("Not connected")
        #FIXME do we want a statusbar? 
        #self.statusbarMain.push(self.status_context_id, status_msg)

        for widget in (#FIXMEself.btnNewPrinter, self.btnNewClass,
                       #self.new_printer, self.new_class,
                       self.ui.chkServerBrowse, self.ui.chkServerShare,
                       self.ui.chkServerRemoteAdmin,
                       self.ui.chkServerAllowCancelAll,
                       self.ui.chkServerLogDebug):
            widget.setEnabled(connected)

        sharing = self.ui.chkServerShare.isChecked ()
        self.ui.chkServerShareAny.setEnabled (sharing)

        try:
            del self.server_settings
        except:
            pass

    def populateList(self, start_printer = None, change_ppd = False):
        old_name, old_type = self.getSelectedItem()
        if self.ui.ntbkMain.currentIndex() == 1:
            serverPageShowing = True
        else:
            serverPageShowing = False

        select_path = None

        if self.cups:
            try:
                # get Printers
                self.printers = cupshelpers.getPrinters(self.cups)

                # Get default printer.
                try:
                    self.default_printer = self.cups.getDefault ()
                except AttributeError: # getDefault appeared in pycups-1.9.31
                    # This fetches the list of printers and classes *again*,
                    # just to find out the default printer.
                    dests = self.cups.getDests ()
                    if dests.has_key ((None,None)):
                        self.default_printer = dests[(None,None)].name
                    else:
                        self.default_printer = None
            except cups.IPPError, (e, m):
                self.show_IPP_Error(e, m)
                self.printers = {}
                self.default_printer = None
        else:
            self.printers = {}
            self.default_printer = None

        local_printers = []
        local_classes = []
        remote_printers = []
        remote_classes = []

        for name, printer in self.printers.iteritems():
            if printer.default:
                self.default_printer = name
            self.servers.add(printer.getServer())

            if printer.remote:
                if printer.is_class: remote_classes.append(name)
                else: remote_printers.append(name)
            else:
                if printer.is_class: local_classes.append(name)
                else: local_printers.append(name)

        local_printers.sort()
        local_classes.sort()
        remote_printers.sort()
        remote_classes.sort()

        if (old_name != "" and
            (not old_name in local_printers) and
            (not old_name in local_classes) and
            (not old_name in remote_printers) and
            (not old_name in remote_classes)):
            # The previously selected printer no longer exists.
            old_name = ""

        if (self.default_printer != None and
            start_printer == None and
            old_name == ""):
            start_printer = self.default_printer

        if not start_printer:
            start_printer = old_name

        expanded = {
            "_printers" : True,
            "_classes" : True,
            "_remote_printers" : True,
            "_remote_classes" : True,
            }

        """
        # remove old printers/classes
        iter = self.mainlist.get_iter_first()
        iter = self.mainlist.iter_next(iter) # step over server settings
        while iter:
            entry = self.mainlist.get_value(iter, 1)
            path = self.mainlist.get_path(iter)
            expanded[entry] = self.tvMainList.row_expanded(path)
            more_entries =  self.mainlist.remove(iter)
            if not more_entries: break
        """
        self.ui.mainlist.clear()
        QTreeWidgetItem(self.ui.mainlist, ["New Printer", 'New'])
        self.settingsItem = QTreeWidgetItem(self.ui.mainlist, ["Server Settings", 'Settings'])
        if serverPageShowing:
            self.settingsItem.setSelected(True)

        # add new
        for printers, text, name in (
            (local_printers, i18n("Local Printers"), "_printers"),
            (local_classes, i18n("Local Classes"), "_classes"),
            (remote_printers, i18n("Remote Printers"), "_remote_printers"),
            (remote_classes, i18n("Remote Classes"), "_remote_classes")):
            if not printers: continue

            #self.mainlist.addTopLevelItem(QTreeWidgetItem(self.mainlist, text))
            rootTreeItem = QTreeWidgetItem(self.ui.mainlist, [text, name])
            #iter = self.mainlist.append(None, (text, name))
            #path = self.mainlist.get_path(iter)

            for printer_name in printers:
                if start_printer == None:
                    start_printer = printer_name
                treeItem = QTreeWidgetItem(rootTreeItem, [printer_name, "Printer"])
                #p_iter = self.mainlist.append(iter, (printer_name, "Printer"))
                if printer_name==start_printer and not serverPageShowing:
                    treeItem.setSelected(True)
                    expanded[name] = True
            if expanded[name]:
                rootTreeItem.setExpanded(True)
                #self.tvMainList.expand_row(path, False)
        self.on_tvMainList_cursor_changed()
        self.setDataButtonState()

        """FIXME
        if change_ppd:
            self.on_btnChangePPD_clicked (self.btnChangePPD)
        """

    #TODO
    # Connect to Server

    def on_printer_changed(self, text):
        widget = self.sender()
        if not widget:  #method called as a method not a slot
            return
        if isinstance(widget, QCheckBox):
            value = widget.isChecked()
        elif isinstance(widget, QLineEdit): #this should be a KLineEdit but causes errors
            value = unicode(widget.text())
        elif isinstance(widget, QRadioButton):
            value = widget.isChecked()
        elif isinstance(widget, QComboBox): #KCombobox causes errors
            value = unicode(widget.currentText())
        else:
            raise ValueError, "Widget type not supported (yet)"

        p = self.printer
        old_values = {
            self.ui.entPDescription : p.info,
            self.ui.entPLocation : p.location,
            self.ui.entPDevice : p.device_uri,
            self.ui.chkPEnabled : p.enabled,
            self.ui.chkPAccepting : not p.rejecting,
            self.ui.chkPShared : p.is_shared,
            self.ui.cmbPStartBanner : p.job_sheet_start,
            self.ui.cmbPEndBanner : p.job_sheet_end,
            self.ui.rbtnPAllow: p.default_allow
            #FIXME Not implemented in the current UI
            #self.cmbPErrorPolicy : p.error_policy,
            #self.cmbPOperationPolicy : p.op_policy,
            }

        old_value = old_values[widget]

        if old_value == value:
            self.changed.discard(widget)
        else:
            self.changed.add(widget)
        self.setDataButtonState()

    #TODO
    # Access control

    #TODO
    # Server side options

    # set buttons sensitivity
    def setDataButtonState(self):
        try: # Might not be a printer selected
            possible = (self.ppd and
                        not bool (self.changed) and
                        self.printer.enabled and
                        not self.printer.rejecting)

            self.btnPrintTestPage.setEnabled(possible)

            commands = (self.printer.type & cups.CUPS_PRINTER_COMMANDS) != 0
            self.btnSelfTest.setEnabled(commands and possible)
            self.btnCleanHeads.setEnabled(commands and possible)
        except:
            debugprint ("exception in setDataButtonState")
            pass

        installablebold = False
        optionsbold = False
        """ FIXME, no conflicts button in system settings
        if self.conflicts:
            debugprint ("Conflicts detected")
            self.btnConflict.show()
            for option in self.conflicts:
                if option.tab_label == self.lblPInstallOptions:
                    installablebold = True
                else:
                    optionsbold = True
        else:
            self.ui.btnConflict.hide()
        """
        installabletext = i18n("Installable Options")
        optionstext = i18n("Printer Options")
        if installablebold:
            installabletext = i18nc("Conflicted entry", "<b>%1</b>", installabletext)
        if optionsbold:
            optionstext = i18nc("Conflicted entry", "<b>%1</b>", optionstext)
        self.ui.lblPInstallOptions.setText(installabletext)
        self.ui.lblPOptions.setText(optionstext)

        """  FIXME
        store = self.tvPrinterProperties.get_model ()
        if store:
            for n in range (self.ntbkPrinter.get_n_pages ()):
                page = self.ntbkPrinter.get_nth_page (n)
                label = self.ntbkPrinter.get_tab_label (page)
                if label == self.lblPInstallOptions:
                    iter = store.get_iter ((n,))
                    store.set_value (iter, 0, installabletext)
                elif label == self.lblPOptions:
                    iter = store.get_iter ((n,))
                    store.set_value (iter, 0, optionstext)
        """

        self.ui.changed(len (self.changed) > 0)
        #self.ui.btnPrinterPropertiesApply.setEnabled(len (self.changed) > 0)
        #self.ui.btnRevert.setEnabled(len (self.changed) > 0)

    def save_printer(self, printer, saveall=False):
        class_deleted = False
        name = printer.name

        try:
            if not printer.is_class and self.ppd: 
                self.getPrinterSettings()
                if self.ppd.nondefaultsMarked() or saveall:
                    self.passwd_retry = False # use cached Passwd 
                    self.cups.addPrinter(name, ppd=self.ppd)

            if printer.is_class:
                # update member list
                new_members = self.getCurrentClassMembers(self.ui.tvClassMembers)
                if not new_members:
                    result = KMessageBox.warningContinueCancel(self.ui, i18n("Class now has no members. Proceed anyway?"), i18n("Class Empty"))
                    if result == KMessageBox.Continue:
                        return True
                    class_deleted = True

                # update member list
                old_members = printer.class_members[:]

                for member in new_members:
                    member = unicode(member)
                    if member in old_members:
                        old_members.remove(member)
                    else:
                        self.cups.addPrinterToClass(member, name)
                for member in old_members:
                    self.cups.deletePrinterFromClass(member, name)    

            location = unicode(self.ui.entPLocation.text())
            info = unicode(self.ui.entPDescription.text())
            device_uri = unicode(self.ui.entPDevice.text())
            if device_uri.find (ellipsis) != -1:
                # The URI is sanitized and not editable.
                device_uri = printer.device_uri

            enabled = self.ui.chkPEnabled.isChecked()
            accepting = self.ui.chkPAccepting.isChecked()
            shared = self.ui.chkPShared.isChecked()

            if info!=printer.info or saveall:
                self.passwd_retry = False # use cached Passwd 
                self.cups.setPrinterInfo(name, info)
            if location!=printer.location or saveall:
                self.passwd_retry = False # use cached Passwd 
                self.cups.setPrinterLocation(name, location)
            if (not printer.is_class and
                (device_uri!=printer.device_uri or saveall)):
                self.passwd_retry = False # use cached Passwd 
                self.cups.setPrinterDevice(name, device_uri)

            if enabled != printer.enabled or saveall:
                self.passwd_retry = False # use cached Passwd 
                self.printer.setEnabled(enabled)
            if accepting == printer.rejecting or saveall:
                self.passwd_retry = False # use cached Passwd 
                self.printer.setAccepting(accepting)
            if shared != printer.is_shared or saveall:
                self.passwd_retry = False # use cached Passwd 
                self.printer.setShared(shared)

            job_sheet_start = unicode(self.ui.cmbPStartBanner.currentText())
            job_sheet_end = unicode(self.ui.cmbPEndBanner.currentText())
            #FIXME Not implemented in current UI
            #error_policy = unicode(self.cmbPErrorPolicy.currentText())
            #op_policy = unicode(self.cmbPOperationPolicy.currentText())

            if (job_sheet_start != printer.job_sheet_start or
                job_sheet_end != printer.job_sheet_end) or saveall:
                self.passwd_retry = False # use cached Passwd
                printer.setJobSheets(job_sheet_start, job_sheet_end)
            #FIXME Not implemented in current UI
            #if error_policy != printer.error_policy or saveall:
                #self.passwd_retry = False # use cached Passwd
                #printer.setErrorPolicy(error_policy)
            #if op_policy != printer.op_policy or saveall:
                #self.passwd_retry = False # use cached Passwd
                #printer.setOperationPolicy(op_policy)

            #Access Control
            default_allow = self.ui.rbtnPAllow.isChecked()
            except_users = self.getPUsers()

            if (default_allow != printer.default_allow or
                except_users != printer.except_users) or saveall:
                self.passwd_retry = False # use cached Passwd
                printer.setAccess(default_allow, except_users)

            for option in printer.attributes:
                if option not in self.server_side_options:
                    printer.unsetOption(option)
            for option in self.server_side_options.itervalues():
                if (option.is_changed() or
                    saveall and
                    option.get_current_value () != option.system_default):
                    printer.setOption(unicode(option.name), unicode(option.get_current_value()) )

        except cups.IPPError, (e, s):
            self.show_IPP_Error(e, s)
            return True
        self.changed = set() # of options
        
        if not self.__dict__.has_key ("server_settings"):
            # We can authenticate with the server correctly at this point,
            # but we have never fetched the server settings to see whether
            # the server is publishing shared printers.  Fetch the settings
            # now so that we can update the "not published" label if necessary.
            try:
                self.server_settings = self.cups.adminGetServerSettings()
            except:
                nonfatalException()

        if class_deleted:
            self.populateList ()
        else:
            # Update our copy of the printer's settings.
            try:
                printers = cupshelpers.getPrinters (self.cups)
                this_printer = { name: printers[name] }
                self.printers.update (this_printer)
            except cups.IPPError, (e, s):
                show_IPP_Error(e, s, self.PrinterPropertiesDialog)

        return False

    def getPrinterSettings(self):
        #self.ppd.markDefaults()
        for option in self.options.itervalues():
            option.writeback()

    @pyqtSignature("")
    def on_btnPrintTestPage_clicked(self):
        self.setTestButton(self.printer)
        if self.test_button_cancels:
            jobs = self.printer.testsQueued ()
            for job in jobs:
                debugprint ("Canceling job %s" % job)
                try:
                    self.cups.cancelJob (job)
                except cups.IPPError, (e, msg):
                    self.show_IPP_Error(e, msg)
            self.setTestButton (self.printer)
            return
        try:
            # if we have a page size specific custom test page, use it;
            # otherwise use cups' default one
            custom_testpage = None
            opt = self.ppd.findOption ("PageSize")
            if opt:
                print opt
                custom_testpage = os.path.join(SYSTEM_CONFIG_PRINTER_DIR, 'testpage-%s.ps' % opt.defchoice.lower())
                print custom_testpage

            if custom_testpage and os.path.exists(custom_testpage):
                debugprint ('Printing custom test page ' + custom_testpage)
                job_id = self.cups.printTestPage(self.printer.name,
                    file=custom_testpage)
                print job_id
            else:
                debugprint ('Printing default test page')
                job_id = self.cups.printTestPage(self.printer.name)

            self.setTestButton (self.printer)
            KMessageBox.information(self, i18n("Test page submitted as job %1", job_id), i18nc("Test page submitted", "Submitted"))
        except cups.IPPError, (e, msg):
            if (e == cups.IPP_NOT_AUTHORIZED and
                self.connect_server != 'localhost' and
                self.connect_server[0] != '/'):
                self.lblError.set_markup ('<span size="larger">'+
                                          i18n("<b>Not possible</b>") + '</span>\n\n' +
                                          i18n("The remote server did not accept "
                                            "the print job, most likely "
                                            "because the printer is not "
                                            "shared."))
                self.ErrorDialog.set_transient_for (self.MainWindow)
                self.ErrorDialog.run ()
                self.ErrorDialog.hide ()
            else:
                self.show_IPP_Error(e, msg)

    def maintenance_command (self, command):
        (tmpfd, tmpfname) = tempfile.mkstemp ()
        os.write (tmpfd, "#CUPS-COMMAND\n%s\n" % command)
        os.close (tmpfd)
        try:
            format = "application/vnd.cups-command"
            job_id = self.cups.printTestPage (self.printer.name,
                                              format=format,
                                              file=tmpfname,
                                              user=self.connect_user)
            self.lblInfo.set_markup ('<span size="larger">' +
                                     i18nc("Maintenance command submitted", "<b>Submitted</b>") + '</span>\n\n' +
                                     i18n("Maintenance command submitted as "
                                       "job %d", job_id))
            self.InfoDialog.set_transient_for (self.MainWindow)
            self.InfoDialog.run ()
            self.InfoDialog.hide ()
        except cups.IPPError, (e, msg):
            if (e == cups.IPP_NOT_AUTHORIZED and
                self.printer.name != 'localhost'):
                self.lblError.set_markup ('<span size="larger">'+
                                          i18n("<b>Not possible</b>") + '</span>\n\n' +
                                          i18n("The remote server did not accept "
                                            "the print job, most likely "
                                            "because the printer is not "
                                            "shared."))
                self.ErrorDialog.set_transient_for (self.MainWindow)
                self.ErrorDialog.run ()
                self.ErrorDialog.hide ()
            else:
                self.show_IPP_Error(e, msg)

    @pyqtSignature("")
    def on_btnSelfTest_clicked(self):
        self.maintenance_command ("PrintSelfTestPage")

    @pyqtSignature("")
    def on_btnCleanHeads_clicked(self):
        self.maintenance_command ("Clean all")

    def fillComboBox(self, combobox, values, value):
        combobox.clear()
        for nr, val in enumerate(values):
            combobox.addItem(val)
            if val == value: 
                combobox.setCurrentIndex(nr)

    def fillPrinterTab(self, name):
        self.changed = set() # of options
        self.options = {} # keyword -> Option object
        self.conflicts = set() # of options

        printer = self.printers[name] 
        self.printer = printer
        printer.getAttributes ()

        editable = not self.printer.discovered
        editablePPD = not self.printer.remote

        try:
            self.ppd = printer.getPPD()
        except cups.IPPError, (e, m):
            # Some IPP error other than IPP_NOT_FOUND.
            self.show_IPP_Error(e, m)
            # Treat it as a raw queue.
            self.ppd = False
        except RuntimeError:
            # The underlying cupsGetPPD2() function returned NULL without
            # setting an IPP error, so it'll be something like a failed
            # connection.
            #FIXME show a dialogue
            debugprint("Error!")
            """
            self.lblError.set_markup('<span size="larger">' +
                                     _("<b>Error</b>") + '</span>\n\n' +
                                     _("There was a problem connecting to "
                                       "the CUPS server."))
            self.ErrorDialog.set_transient_for(self.MainWindow)
            self.ErrorDialog.run()
            self.ErrorDialog.hide()
            """
            raise

        for widget in (self.ui.entPDescription, self.ui.entPLocation,
                        self.ui.entPName, self.ui.entPDevice):
                            widget.setReadOnly(not editable)

        for widget in (self.ui.btnChangePPD,
                       self.ui.chkPEnabled, self.ui.chkPAccepting, self.ui.chkPShared,
                       self.ui.cmbPStartBanner, self.ui.cmbPEndBanner,  self.ui.btnPAddUser,
                       self.ui.btnPDelUser, self.ui.rbtnPAllow, self.ui.rbtnPDeny):
                    #""",FIXME
                       #self.btnSelectDevice, self.cmbPErrorPolicy,
                       #self.cmbPOperationPolicy,):
                    #"""
                widget.setEnabled(editable)

        # Description page
        self.ui.entPName.setText(printer.name)
        self.ui.entPDescription.setText(printer.info)
        self.ui.entPLocation.setText(printer.location)

        uri = printer.device_uri
        if uri.startswith("smb://"):
            (group, host, share,
             user, password) = SMBURI (uri=uri[6:]).separate ()
            if password:
                uri = "smb://"
                if len (user) or len (password):
                    uri += ellipsis
                uri += SMBURI (group=group, host=host, share=share).get_uri ()
                self.ui.entPDevice.setEnabled(False)
            else:
                self.ui.entPDevice.setEnabled(True)

        self.ui.entPDevice.setText(uri)
        self.changed.discard(self.ui.entPDevice)

        # Hide make/model and Device URI for classes
        for widget in (self.ui.lblPMakeModel2, self.ui.lblPMakeModel,
                        self.ui.lblPDevice2, self.ui.entPDevice,
                        self.ui.btnChangePPD):
                       #, self.btnSelectDevice):
            if printer.is_class:
                widget.hide()
            else:
                widget.show()

        ## default printer
        default = self.cups.getDefault()
        
        if printer.name == default:
            #catch the stateChanged signal to prevent a signal loop
            self.ui.chkPMakeDefault.blockSignals(True) 
            self.ui.chkPMakeDefault.setChecked(True) #set checked
            #unblock the signal
            self.ui.chkPMakeDefault.blockSignals(False)
            self.ui.chkPMakeDefault.setText(i18n("This is the default printer"))
        else:
            #catch the stateChanged signal to prevent a signal loop
            self.ui.chkPMakeDefault.blockSignals(True)
            self.ui.chkPMakeDefault.setChecked(False) #set unchecked
            #unblock the signal
            self.ui.chkPMakeDefault.blockSignals(False)
            if default is not None:
                self.ui.chkPMakeDefault.setText(i18n('Make Default. (The current default is %1)', default))
            else:
                self.ui.chkPMakeDefault.setText(i18n('Make Default. (There is no current default)'))

        #self.setTestButton (printer)

        # Policy tab
        # ----------

        ## State
        self.ui.chkPEnabled.setChecked(printer.enabled)
        self.ui.chkPAccepting.setChecked(not printer.rejecting)
        self.ui.chkPShared.setChecked(printer.is_shared)
        try:
            if printer.is_shared:
                flag = cups.CUPS_SERVER_SHARE_PRINTERS
                publishing = int (self.server_settings[flag])
                if publishing:
                    self.ui.lblNotPublished.hide()
                else:
                    self.ui.lblNotPublished.show()
            else:
                self.ui.lblNotPublished.hide()
        except:
            self.ui.lblNotPublished.hide()

        # Job sheets
        self.ui.cmbPStartBanner.setEnabled(editable)
        self.ui.cmbPEndBanner.setEnabled(editable)

#FIXME Not implemented in the current UI
        # Policies
        #self.cmbPErrorPolicy.setEnabled(editable)
        #self.cmbPOperationPolicy.setEnabled(editable)

        
        # Access control
        self.ui.rbtnPAllow.setChecked(printer.default_allow)
        self.ui.rbtnPDeny.setChecked(not printer.default_allow)
        self.setPUsers(printer.except_users)

        #self.entPUser.set_text("")
        # Server side options (Job options)
        self.server_side_options = {}
        for option in self.job_options_widgets.values ():
            if option.name == "media" and self.ppd:
                # Slightly special case because the 'system default'
                # (i.e. what you get when you press Reset) depends
                # on the printer's PageSize.
                opt = self.ppd.findOption ("PageSize")
                if opt:
                    option.set_default (opt.defchoice)

            option_editable = editable
            try:
                value = self.printer.attributes[option.name]
            except KeyError:
                option.reinit (None)
            else:
                try:
                    if self.printer.possible_attributes.has_key (option.name):
                        supported = self.printer.\
                                    possible_attributes[option.name][1]
                        # Set the option widget.
                        # In CUPS 1.3.x the orientation-requested-default
                        # attribute may have the value None; this means there
                        # is no value set.  This suits our needs here, as None
                        # resets the option to the system default and makes the
                        # Reset button insensitive.
                        option.reinit (value, supported=supported)
                    else:
                        option.reinit (value)

                    self.server_side_options[option.name] = option
                except ValueError, inst:
                    option_editable = False
                    text = ('<span ' +
                                              'size="larger">' +
                                              i18n("<b>Error</b>") + '</span>\n\n' +
                                              i18n("Option '%1' has value '%2' "
                                                   "and cannot be edited.",
                                                   option.name, value))
                    QMessageBox.critical(self.ui, i18n("Error"), text)
            option.widget.setEnabled(option_editable)
            if not editable:
                option.button.setEnabled(False)
        self.other_job_options = []
        self.draw_other_job_options (editable=editable)
        for option in self.printer.attributes.keys ():
            if self.server_side_options.has_key (option):
                continue
            supported = ""
            if self.printer.possible_attributes.has_key (option):
                supported = self.printer.possible_attributes[option][1]
            self.add_job_option (option, value=self.printer.attributes[option],
                                 supported=supported, is_new=False,
                                 editable=editable)
        self.ui.entNewJobOption.setText ('')
        self.ui.entNewJobOption.setEnabled(editable)
        self.ui.btnNewJobOption.setEnabled(False)

        if printer.is_class:
            # remove InstallOptions tab
            ##tab_nr = self.ui.ntbkPrinter.page_num(self.swPInstallOptions)
            ##if tab_nr != -1:
            ##    self.ui.ntbkPrinter.remove_page(tab_nr)
            self.fillClassMembers(name, editable)
            pass
        else:
            # real Printer
            self.fillPrinterOptions(name, editablePPD)

        #self.changed = set() # of options
        self.updatePrinterProperties ()
        self.setDataButtonState()


    def fillPrinterOptions(self, name, editable):
        # remove Member tab
        tab_nr = self.ui.ntbkPrinter.indexOf(self.memberTabWidget)
        if tab_nr != -1:
            self.ui.ntbkPrinter.removeTab(tab_nr)
        policiesTabNo = self.ui.ntbkPrinter.indexOf(self.policiesTabWidget)
        self.ui.ntbkPrinter.insertTab(policiesTabNo+1, self.optionsTabWidget, i18n("Options"))

        """
        # clean Installable Options Tab
        for widget in self.vbPInstallOptions.get_children():
            self.vbPInstallOptions.remove(widget)

        """
        # clean Options Tab
        for widget in self.ui.optionsPageWidget.children():
            if isinstance(widget, QWidget):
                self.ui.vbPOptions.removeWidget(widget)
                widget.hide()
            del widget
        """

        # insert Options Tab
        if self.ntbkPrinter.page_num(self.swPOptions) == -1:
            self.ntbkPrinter.insert_page(
                self.swPOptions, self.lblPOptions, self.static_tabs)

        if not self.ppd:
            tab_nr = self.ntbkPrinter.page_num(self.swPInstallOptions)
            if tab_nr != -1:
                self.ntbkPrinter.remove_page(tab_nr)
            tab_nr = self.ntbkPrinter.page_num(self.swPOptions)
            if tab_nr != -1:
                self.ntbkPrinter.remove_page(tab_nr)           
            return
        """
        ppd = self.ppd
        ppd.markDefaults()

        hasInstallableOptions = False
        
        # build option tabs
        for group in ppd.optionGroups:
            if group.name == "InstallableOptions":
                """
                hasInstallableOptions = True
                container = self.vbPInstallOptions
                tab_nr = self.ntbkPrinter.page_num(self.swPInstallOptions)
                if tab_nr == -1:
                    self.ntbkPrinter.insert_page(
                        self.swPInstallOptions, gtk.Label(group.text),
                        self.static_tabs)
                """
                tab_label = self.ui.lblPInstallOptions
            else:
                """
                frame = gtk.Frame("<b>%s</b>" % group.text)
                frame.get_label_widget().set_use_markup(True)
                frame.set_shadow_type (gtk.SHADOW_NONE)
                self.vbPOptions.pack_start (frame, False, False, 0)
                container = gtk.Alignment (0.5, 0.5, 1.0, 1.0)
                # We want a left padding of 12, but there is a Table with
                # spacing 6, and the left-most column of it (the conflict
                # icon) is normally hidden, so just use 6 here.
                container.set_padding (6, 12, 6, 0)
                frame.add (container)
                """
                tab_label = self.ui.lblPOptions
            container = QGroupBox(self)
            self.ui.vbPOptions.addWidget(container)
            container.setTitle(group.text)
            """
            
            table = gtk.Table(1, 3, False)
            table.set_col_spacings(6)
            table.set_row_spacings(6)
            container.add(table)
            """
            table = QGridLayout(container)
            container.setLayout(table)

            rows = 0

            # InputSlot and ManualFeed need special handling.  With
            # libcups, if ManualFeed is True, InputSlot gets unset.
            # Likewise, if InputSlot is set, ManualFeed becomes False.
            # We handle it by toggling the sensitivity of InputSlot
            # based on ManualFeed.
            self.option_inputslot = self.option_manualfeed = None

            for nr, option in enumerate(group.options):
                if option.keyword == "PageRegion":
                    continue
                rows += 1
                """
                table.resize (rows, 3)
                """
                o = OptionWidget(option, ppd, self, tab_label=tab_label)
                """
                table.attach(o.conflictIcon, 0, 1, nr, nr+1, 0, 0, 0, 0)

                hbox = gtk.HBox()
                """
                if o.label:
                    table.addWidget(o.label, rows-1, 0)
                """
                    a = gtk.Alignment (0.5, 0.5, 1.0, 1.0)
                    a.set_padding (0, 0, 0, 6)
                    a.add (o.label)
                    table.attach(a, 1, 2, nr, nr+1, gtk.FILL, 0, 0, 0)
                    table.attach(hbox, 2, 3, nr, nr+1, gtk.FILL, 0, 0, 0)
                else:
                    table.attach(hbox, 1, 3, nr, nr+1, gtk.FILL, 0, 0, 0)
                hbox.pack_start(o.selector, False)
                """
                table.addWidget(o.selector, rows-1, 1)
                self.options[option.keyword] = o
                o.selector.setEnabled(editable)
                if option.keyword == "InputSlot":
                    self.option_inputslot = o
                elif option.keyword == "ManualFeed":
                    self.option_manualfeed = o

        """
        # remove Installable Options tab if not needed
        if not hasInstallableOptions:
            tab_nr = self.ntbkPrinter.page_num(self.swPInstallOptions)
            if tab_nr != -1:
                self.ntbkPrinter.remove_page(tab_nr)

        # check for conflicts
        for option in self.options.itervalues():
            conflicts = option.checkConflicts()
            if conflicts:
                self.conflicts.add(option)

        self.swPInstallOptions.show_all()
        self.swPOptions.show_all()
        """


    # Class members
    
    def fillClassMembers(self, name, editable):
        printer = self.printers[name]

        self.ui.btnClassAddMember.setEnabled(editable)
        self.ui.btnClassDelMember.setEnabled(editable)

        # remove Options tab
        tab_nr = self.ui.ntbkPrinter.indexOf(self.optionsTabWidget)
        if tab_nr != -1:
            self.ui.ntbkPrinter.removeTab(tab_nr)
        policiesTabNo = self.ui.ntbkPrinter.indexOf(self.policiesTabWidget)
        self.ui.ntbkPrinter.insertTab(policiesTabNo+1, self.memberTabWidget, i18n("Members"))

        self.ui.tvClassMembers.clear()
        self.ui.tvClassNotMembers.clear()
        names = self.printers.keys()
        names.sort()
        for name in names:
            p = self.printers[name]
            if p is not printer:
                if name in printer.class_members:
                    self.ui.tvClassMembers.addItem(name)
                else:
                    self.ui.tvClassNotMembers.addItem(name)

    def on_btnClassAddMember_clicked(self):
        self.moveClassMembers(self.ui.tvClassNotMembers,
                              self.ui.tvClassMembers)
        if self.getCurrentClassMembers(self.ui.tvClassMembers) != self.printer.class_members:
            self.changed.add(self.ui.tvClassMembers)
        else:
            self.changed.discard(self.ui.tvClassMembers)
        self.setDataButtonState()

    def on_btnClassDelMember_clicked(self):
        self.moveClassMembers(self.ui.tvClassMembers,
                              self.ui.tvClassNotMembers)
        if self.getCurrentClassMembers(self.ui.tvClassMembers) != self.printer.class_members:
            self.changed.add(self.ui.tvClassMembers)
        else:
            self.changed.discard(self.ui.tvClassMembers)
        self.setDataButtonState()

    #In Gnome is now on_delete_activate(self, UNUSED):
    def btnDelete_clicked(self):
        name, type = self.getSelectedItem()

        # Confirm
        if type == "Printer":
            message_format = i18n("Really delete printer %s?")
        else:
            message_format = i18n("Really delete class %s?")

        answer = KMessageBox.questionYesNo(self,
                    unicode(message_format) % name,
                    "")

        if answer == KMessageBox.Yes:
            try:
                self.cups.deletePrinter(name)
            except cups.IPPError, (e, msg):
                self.show_IPP_Error(e, msg)
        elif answer == KMessageBox.No:
            return

        self.changed = set()
        self.populateList()
        self.ui.mainlist.setCurrentItem(self.ui.mainlist.itemAt(0,0))

    #Taken from the gnome version
    def setDefaultPrinter(self, name):
        printer = self.printers[name]
        try:
            printer.setAsDefault()
        except cups.HTTPError, (s,):
            self.show_HTTP_Error(s)
            return
        except cups.IPPError(e, msg):
            show_IPP_Error(e, msg)
            return

    def chkPMakeDefault_stateChanged(self):
        default = self.cups.getDefault()
        try:
            if self.ui.chkPMakeDefault.isChecked():
                name = self.ui.entPName.text()
                self.setDefaultPrinter(unicode(name))
                self.ui.chkPMakeDefault.setText(i18n("This is the default printer"))
            else:
                if default is not None:
                    self.ui.chkPMakeDefault.setText(i18n('Make Default. (The current default is %1)', default))
                else:
                    self.ui.chkPMakeDefault.setText(i18n('Make Default. (There is no current default)'))

            self.ui.changed(len(self.changed) > 0)
            #self.ui.btnPrinterPropertiesApply.setEnabled(len (self.changed) > 0)
            #self.ui.btnRevert.setEnabled(len (self.changed) > 0)
            
        except cups.IPPError, (e, msg):
            self.show_IPP_Error(e, msg)
            return

##############################
        #Obsolete? Possibly no longer necessary. Not included in 
        #current gnome version
        # Also need to check system-wide lpoptions because that's how
        # previous Fedora versions set the default (bug #217395).
        (tmpfd, tmpfname) = tempfile.mkstemp ()
        success = False
        try:
            resource = "/admin/conf/lpoptions"
            self.cups.getFile(resource, tmpfname)
            #success = True
        except cups.HTTPError, (s,):
            try:
                os.remove (tmpfname)
            except OSError:
                pass

            if s != cups.HTTP_NOT_FOUND:
                self.show_HTTP_Error(s)
                return

        if success:
            lines = file (tmpfname).readlines ()
            changed = False
            i = 0
            for line in lines:
                if line.startswith ("Default "):
                    # This is the system-wide default.
                    name = line.split (' ')[1]
                    if name != self.printer.name:
                        # Stop it from over-riding the server default.
                        lines[i] = "Dest " + line[8:]
                        changed = True
                i += 1

            if changed:
                file (tmpfname, 'w').writelines (lines)
                try:
                    self.cups.putFile (resource, tmpfname)
                except cups.HTTPError, (s,):
                    os.remove (tmpfname)
                    self.show_HTTP_Error(s)
                    return
#########################
                # Now reconnect because the server needs to reload.
                self.reconnect ()

        try:
            os.remove (tmpfname)
        except OSError:
            pass

        try:
            self.populateList()
        except cups.HTTPError, (s,):
            self.cups = None
            self.setConnected()
            self.populateList()
            self.show_HTTP_Error(s)

    def option_changed(self, option):
        if option.is_changed():
            self.changed.add(option)
        else:
            self.changed.discard(option)

        if option.conflicts:
            self.conflicts.add(option)
        else:
            self.conflicts.discard(option)
        self.setDataButtonState()

        if (self.option_manualfeed and self.option_inputslot and
            option == self.option_manualfeed):
            if option.get_current_value() == "True":
                self.option_inputslot.disable ()
            else:
                self.option_inputslot.enable ()

    # Access control
    def getPUsers(self):
        """return list of usernames from the GUI"""
        result = [] #empty list to hold result
        #if there is only one user in the list
        if self.ui.tvPUsers.topLevelItemCount() < 2:
            item = self.ui.tvPUsers.itemAt(0, 0)
            #use a string instead of a QString.
            #system-config-printer fails in 
            #self.connection.setPrinterUsersDenied(self.name, except_users)
            #if we use a QString
            if item is not None:
                name = str(item.text(0))
                result.append(name)
        else:
            #create an iterator for the QTreeWidgetObject
            #had to create an iterator class manually because QTreeWidgetItemIterator
            #is broken
            it = Iter(self.ui.tvPUsers)    
            while it: #iterate over the tree
                try:
                    item = it.next()
                    name = item.text(0)
                    result.append(name)
                except StopIteration:
                    break
                    
        result.sort()
        return result

    def setPUsers(self, users):
        """write list of usernames into the GUI"""
        self.ui.tvPUsers.clear()
        for user in users:
            #create a QTreeWidgetItem
            u = QTreeWidgetItem(self.ui.tvPUsers)
            u.setText(0, user) #set the label for the item
            #add the item to the top level of the tree
            self.ui.tvPUsers.addTopLevelItem(u)

    def checkPUsersChanged(self):
        """check if users in GUI and printer are different
        and set self.changed"""
        if self.getPUsers() != self.printer.except_users:
            self.changed.add(self.ui.tvPUsers)
        else:
            self.changed.discard(self.ui.tvPUsers)

        self.setDataButtonState()

    def btnPAddUser_clicked(self):
        current_user = os.getenv('USER')
        user, ok = KInputDialog.getText(i18n('Add User'), i18n('Enter Username'), current_user)
        type(user)
        
        if ok and not user.isEmpty():
            u = QTreeWidgetItem(self.ui.tvPUsers)
            u.setText(0, user)
            self.ui.tvPUsers.addTopLevelItem(u)
        
        self.checkPUsersChanged()

    def btnPDelUser_clicked(self):
        users = self.ui.tvPUsers.selectedItems()
        for u in users:
            index = self.ui.tvPUsers.indexOfTopLevelItem(u)
            self.ui.tvPUsers.takeTopLevelItem(index)
    
        self.checkPUsersChanged()

    # Server side options
    def on_job_option_reset(self):
        button = self.sender()
        option = self.job_options_buttons[button]
        option.reset ()
        # Remember to set this option for removal in the IPP request.
        if self.server_side_options.has_key (option.name):
            del self.server_side_options[option.name]
        if option.is_changed ():
            self.changed.add(option)
        else:
            self.changed.discard(option)
        self.setDataButtonState()

    def on_job_option_changed(self):
        if not self.printer:
            return
        widget = self.sender()
        option = self.job_options_widgets[widget]
        option.changed ()
        if option.is_changed ():
            self.server_side_options[option.name] = option
            self.changed.add(option)
        else:
            if self.server_side_options.has_key (option.name):
                del self.server_side_options[option.name]
            self.changed.discard(option)
        self.setDataButtonState()
        # Don't set the reset button insensitive if the option hasn't
        # changed from the original value: it's still meaningful to
        # reset the option to the system default.

    def draw_other_job_options (self, editable=True):
        n = len (self.other_job_options)
        #if n == 0:
        #    self.tblJOOther.hide_all ()
        #    return

        #self.tblJOOther.resize (n, 3)
        children = self.ui.tblJOOtherWidget.children()
        for child in children:
            if type(child) != QGridLayout:
                self.ui.tblJOOther.removeWidget(child)
                child.hide()
        i = 0
        self.removeJobOptionButtons = {}
        for opt in self.other_job_options:
            self.ui.tblJOOther.addWidget(opt.label, i, 0)
            self.ui.tblJOOther.addWidget(opt.selector, i, 1)
            opt.label.show()
            opt.selector.show()
            opt.selector.setEnabled(editable)

            btn = QPushButton("Remove", self.ui.tblJOOtherWidget)
            self.connect(btn, SIGNAL("clicked()"), self.on_btnJOOtherRemove_clicked)
            #btn.set_data("pyobject", opt)
            self.removeJobOptionButtons[btn] = opt
            btn.setEnabled(editable)
            self.ui.tblJOOther.addWidget(btn, i, 2)
            i += 1

        #self.tblJOOther.show_all ()

    def add_job_option(self, name, value = "", supported = "", is_new=True,
                       editable=True):
        option = options.OptionWidget(name, value, supported,
                                      self.option_changed)
        option.is_new = is_new
        self.other_job_options.append (option)
        self.draw_other_job_options (editable=editable)
        self.server_side_options[name] = option
        if name in self.changed: # was deleted before
            option.is_new = False
        self.changed.add(option)
        self.setDataButtonState()
        if is_new:
            ##option.selector.grab_focus ()
            pass #FIXME

    def on_btnJOOtherRemove_clicked(self):
        button = self.sender()
        option = self.removeJobOptionButtons[button]
        #option = button.get_data("pyobject")
        self.other_job_options.remove (option)
        self.draw_other_job_options ()
        if option.is_new:
            self.changed.discard(option)
        else:
            # keep name as reminder that option got deleted
            self.changed.add(option.name)
        del self.server_side_options[option.name]
        self.setDataButtonState()

    def on_btnNewJobOption_clicked(self):
        name = self.ui.entNewJobOption.text()
        self.add_job_option(name)
        #self.tblJOOther.show_all()
        self.ui.entNewJobOption.setText('')
        self.ui.btnNewJobOption.setEnabled(False)
        self.setDataButtonState()

    def on_entNewJobOption_changed(self, widget):
        text = unicode(self.ui.entNewJobOption.text())
        active = (len(text) > 0) and text not in self.server_side_options
        self.ui.btnNewJobOption.setEnabled(active)

    def on_entNewJobOption_activate(self, widget):
        self.on_btnNewJobOption_clicked (widget) # wrong widget but ok

    ##########################################################################
    ### Server settings
    ##########################################################################

    def fillServerTab(self):
        self.changed = set()
        try:
            self.server_settings = self.cups.adminGetServerSettings()
        except cups.IPPError, (e, m):
            #FIXME
            self.show_IPP_Error(e, m)
            self.ui.tvMainList.get_selection().unselect_all()
            self.on_tvMainList_cursor_changed(self.ui.tvMainList)
            return

        for widget, setting in [
            (self.ui.chkServerBrowse, cups.CUPS_SERVER_REMOTE_PRINTERS),
            (self.ui.chkServerShare, cups.CUPS_SERVER_SHARE_PRINTERS),
            (self.ui.chkServerShareAny, try_CUPS_SERVER_REMOTE_ANY),
            (self.ui.chkServerRemoteAdmin, cups.CUPS_SERVER_REMOTE_ADMIN),
            (self.ui.chkServerAllowCancelAll, cups.CUPS_SERVER_USER_CANCEL_ANY),
            (self.ui.chkServerLogDebug, cups.CUPS_SERVER_DEBUG_LOGGING),]:
            # widget.set_data("setting", setting)
            self.widget_data_setting[widget] = setting
            self.disconnect(widget, SIGNAL("stateChanged(int)"), self.on_server_widget_changed)
            if self.server_settings.has_key(setting):
                widget.setChecked(int(self.server_settings[setting]))
                widget.setEnabled(True)
            else:
                widget.setChecked(False)
                widget.setEnabled(False)
            self.connect(widget, SIGNAL("stateChanged(int)"), self.on_server_widget_changed)
        self.setDataButtonState()
        # Set sensitivity of 'Allow printing from the Internet'.
        self.on_server_changed (self.ui.chkServerShare) # (any will do here)

    def on_server_widget_changed(self, value):
        self.on_server_changed(self.sender())

    def on_server_changed(self, widget):
        setting = self.widget_data_setting[widget]
        if self.server_settings.has_key (setting):
            if str(int(widget.isChecked())) == self.server_settings[setting]:
                self.changed.discard(widget)
            else:
                self.changed.add(widget)

        sharing = self.ui.chkServerShare.isChecked ()
        self.ui.chkServerShareAny.setEnabled (
            sharing and self.server_settings.has_key(try_CUPS_SERVER_REMOTE_ANY))

        self.setDataButtonState()

    def save_serversettings(self):
        setting_dict = self.server_settings.copy()
        for widget, setting in [
            (self.ui.chkServerBrowse, cups.CUPS_SERVER_REMOTE_PRINTERS),
            (self.ui.chkServerShare, cups.CUPS_SERVER_SHARE_PRINTERS),
            (self.ui.chkServerShareAny, try_CUPS_SERVER_REMOTE_ANY),
            (self.ui.chkServerRemoteAdmin, cups.CUPS_SERVER_REMOTE_ADMIN),
            (self.ui.chkServerAllowCancelAll, cups.CUPS_SERVER_USER_CANCEL_ANY),
            (self.ui.chkServerLogDebug, cups.CUPS_SERVER_DEBUG_LOGGING),]:
            if not self.server_settings.has_key(setting): continue
            setting_dict[setting] = str(int(widget.isChecked()))
        try:
            self.cups.adminSetServerSettings(setting_dict)
        except cups.IPPError, (e, m):
            self.show_IPP_Error(e, m)
            return True
        except RuntimeError, s:
            self.show_IPP_Error(None, s)
            return True
        self.changed = set()
        self.setDataButtonState()
        time.sleep(1) # give the server a chance to process our request

        # Now reconnect, in case the server needed to reload.
        self.reconnect ()

        # Refresh the server settings in case they have changed in the
        # mean time.
        try:
            self.fillServerTab()
        except:
            nonfatalException()

    # ====================================================================
    # == New Printer Dialog ==============================================
    # ====================================================================

    # new printer
    def on_new_printer_activate(self):
        self.ui.setCursor(Qt.WaitCursor)
        self.newPrinterGUI.init("printer")
        self.ui.setCursor(Qt.ArrowCursor)

    # new class
    def on_new_class_activate(self):
        self.ui.setCursor(Qt.WaitCursor)
        self.newPrinterGUI.init("class")
        self.ui.setCursor(Qt.ArrowCursor)

    @pyqtSignature("")
    def on_btnSelectDevice_clicked(self):
        self.newPrinterGUI.init("device")

    @pyqtSignature("")
    def on_btnChangePPD_clicked(self):
        self.ui.setCursor(Qt.WaitCursor)
        self.newPrinterGUI.init("ppd")
        self.ui.setCursor(Qt.ArrowCursor)

    def checkNPName(self, name):
        if not name: return False
        name = name.lower()
        for printer in self.printers.values():
            if not printer.discovered and printer.name.lower()==name:
                return False
        return True

    def makeNameUnique(self, name):
        """Make a suggested queue name valid and unique."""
        name = name.replace (" ", "_")
        name = name.replace ("/", "_")
        name = name.replace ("#", "_")
        if not self.checkNPName (name):
            suffix=2
            while not self.checkNPName (name + str (suffix)):
                suffix += 1
                if suffix == 100:
                    break
            name += str (suffix)
        return name

    #TODO
    ## Watcher interface helpers

    @pyqtSignature("")
    def on_btnRevert_clicked(self):
        self.changed = set() # avoid asking the user
        self.on_tvMainList_cursor_changed()

    @pyqtSignature("")
    def on_btnPrinterPropertiesApply_clicked(self):
        err = self.printer_properties_response()
        if not err:
            self.populateList()
        else:
            nonfatalException()

    def show_IPP_Error(self, exception, message):
        if exception == cups.IPP_NOT_AUTHORIZED:
            KMessageBox.error(self, i18n('The password may be incorrect.'), i18n('Not authorized'))
        else:
            KMessageBox.error(self, i18n("There was an error during the CUPS " "operation: '%1'.", message),
                                    i18n('CUPS server error'))

    def show_HTTP_Error(self, status):
        if (status == cups.HTTP_UNAUTHORIZED or
            status == cups.HTTP_FORBIDDEN):
            
            KMessageBox.error(self,i18n('The password may be incorrect, or the '
                                   'server may be configured to deny '
                                   'remote administration.'),
                                   i18n('Not authorized') )
        else:
            if status == cups.HTTP_BAD_REQUEST:
                msg = i18nc("HTTP error", "Bad request")
            elif status == cups.HTTP_NOT_FOUND:
                msg = i18nc("HTTP error", "Not found")
            elif status == cups.HTTP_REQUEST_TIMEOUT:
                msg = i18nc("HTTP error", "Request timeout")
            elif status == cups.HTTP_UPGRADE_REQUIRED:
                msg = i18nc("HTTP error", "Upgrade required")
            elif status == cups.HTTP_SERVER_ERROR:
                msg = i18nc("HTTP error", "Server error")
            elif status == -1:
                msg = i18nc("HTTP error", "Not connected")
            else:
                msg = i18nc("HTTP error", "status %1", status)

        KMessageBox.error(self, i18n("There was an HTTP error: %1.", msg), i18n('CUPS server error'))

    def getSelectedItem(self):
        return unicode(self.mainListSelectedName), unicode(self.mainListSelectedType)
        """
        items = self.mainlist.selectedItems()
        if len(items) < 1:
            return ("", 'None')
            item = items[0]
            name = item.text(0)
            type = item.text(1)
            name = str(name).decode ('utf-8')
            return name.strip(), type
        """

    def reconnect (self):
        """Reconnect to CUPS after the server has reloaded."""
        # libcups would handle the reconnection if we just told it to
        # do something, for example fetching a list of classes.
        # However, our local authentication certificate would be
        # invalidated by a server restart, so it is better for us to
        # handle the reconnection ourselves.

        # Disconnect.
        self.cups = None
        self.setConnected()

        cups.setServer(self.connect_server)
        cups.setUser(self.connect_user)
        attempt = 1
        while attempt <= 5:
            try:
                self.cups = cups.Connection ()
                break
            except RuntimeError:
                # Connection failed.
                time.sleep(1)
                attempt += 1

        self.setConnected()
        self.passwd_retry = False

    def updatePrinterProperties(self):
        debugprint ("update printer properties")
        printer = self.printer
        self.ui.lblPMakeModel.setText(printer.make_and_model)
        state = self.printer_states.get (printer.state, i18nc("Printer state", "Unknown"))[:]
        reason = printer.other_attributes.get ('printer-state-message', '')
        if len (reason) > 0:
            state += ' - ' + reason
        self.ui.lblPState.setText(state)
        if len (self.changed) == 0:
            debugprint ("no changes yet: full printer properties update")

            # State
            self.ui.chkPEnabled.setEnabled(printer.enabled)
            self.ui.chkPAccepting.setEnabled(not printer.rejecting)
            self.ui.chkPShared.setEnabled(printer.is_shared)

            # Job sheets
            self.fillComboBox(self.ui.cmbPStartBanner,
                              printer.job_sheets_supported,
                              printer.job_sheet_start),
            self.fillComboBox(self.ui.cmbPEndBanner, printer.job_sheets_supported,
                              printer.job_sheet_end)

#FIXME Not implemented in current UI
            # Policies
            #self.fillComboBox(self.cmbPErrorPolicy,
                              #printer.error_policy_supported,
                              #printer.error_policy)
            #self.fillComboBox(self.cmbPOperationPolicy,
                              #printer.op_policy_supported,
                              #printer.op_policy)

            
            # Access control
            self.ui.rbtnPAllow.setChecked(printer.default_allow)
            self.ui.rbtnPDeny.setChecked(not printer.default_allow)
            self.setPUsers(printer.except_users)
            

    def setTestButton (self, printer):
        if printer.testsQueued ():
            self.test_button_cancels = True
            self.ui.btnPrintTestPage.setText(i18n('Cancel Tests'))
            self.ui.btnPrintTestPage.setEnabled(True)
        else:
            self.test_button_cancels = False
            self.ui.btnPrintTestPage.setText(i18n('Print Test Page'))
            self.setDataButtonState()

    def getCurrentClassMembers(self, listwidget):
        count = listwidget.count()
        result = []
        for i in range(count):            
            result.append(listwidget.item(i).text())
        result.sort()
        return result

    def moveClassMembers(self, treeview_from, treeview_to):
        rows = treeview_from.selectedItems()
        for row in rows:
            treeview_from.takeItem(treeview_from.row(row))
            treeview_to.addItem(row)

    #FIXME obsolete?
    def cupsPasswdCallback(self, querystring):
        return "" #FIXME
        if self.passwd_retry or len(self.password) == 0:
            waiting = self.WaitWindow.get_property('visible')
            if waiting:
                self.WaitWindow.hide ()
            self.lblPasswordPrompt.set_label (self.prompt_primary +
                                            querystring)
            self.PasswordDialog.set_transient_for (self.MainWindow)
            self.entPasswd.grab_focus ()

            result = self.PasswordDialog.run()
            self.PasswordDialog.hide()
            if waiting:
                self.WaitWindow.show ()
            while gtk.events_pending ():
                gtk.main_iteration ()
            if result == gtk.RESPONSE_OK:
                self.password = self.entPasswd.get_text()
            else:
                self.password = ''
            self.passwd_retry = False
        else:
            self.passwd_retry = True
        return self.password


class NewPrinterGUI(QDialog):

    new_printer_device_tabs = {
        "parallel" : 0, # empty tab
        "usb" : 0,
        "hal" : 0,
        "beh" : 0,
        "hp" : 0,
        "hpfax" : 0,
        "socket": 2,
        "ipp" : 3,
        "http" : 3,
        "lpd" : 4,
        "scsi" : 5,
        "serial" : 6,
        "smb" : 7,
        }

    ntbkNewPrinterPages = {
        "name" : 0,
        "device" : 1,
        "make" : 2,
        "model" : 3,
        "class-members" : 4,
        "downloadable" : -1,
    }

    def __init__(self, mainapp):
        QDialog.__init__(self, mainapp)
        self.mainapp = mainapp
        self.language = mainapp.language
        self.dialog_mode = ""

        self.WaitWindow = QMessageBox(self.mainapp)
        self.WaitWindow.setStandardButtons(QMessageBox.NoButton)

        uic.loadUi(APPDIR + "/" + "new-printer.ui", self)

        self.btnNPBack.setIcon(KIcon("go-previous"))
        self.btnNPForward.setIcon(KIcon("go-next"))
        self.btnNPCancel.setIcon(KIcon("dialog-cancel"))
        self.btnNPApply.setIcon(KIcon("dialog-ok"))
        self.btnNCAddMember.setIcon(KIcon("arrow-left"))
        self.btnNCDelMember.setIcon(KIcon("arrow-right"))

        self.connect(self.tvNPDevices, SIGNAL("itemSelectionChanged()"), self.on_tvNPDevices_cursor_changed)
        self.connect(self.tvNPMakes, SIGNAL("itemSelectionChanged()"), self.on_tvNPMakes_cursor_changed)
        self.connect(self.tvNPModels, SIGNAL("itemSelectionChanged()"), self.on_tvNPModels_cursor_changed)
        self.connect(self.entNPTDevice, SIGNAL("textEdited(const QString&)"), self.on_entNPTDevice_changed)
#        self.connect(self.entNPTIPPHostname, SIGNAL("textEdited(const QString&)"), self.on_entNPTIPPHostname_changed)
#        self.connect(self.entNPTIPPQueuename, SIGNAL("textEdited(const QString&)"), self.on_entNPTIPPQueuename_changed)
        self.connect(self.entSMBURI, SIGNAL("textEdited(const QString&)"), self.on_entSMBURI_changed)
        self.rbtnSMBAuthPrompt.setChecked(True)
        self.on_rbtnSMBAuthSet_toggled(False)
        self.connect(self.rbtnSMBAuthSet, SIGNAL("toggled(bool)"), self.on_rbtnSMBAuthSet_toggled)
        self.rbtnNPFoomatic.setChecked(True)
        self.connect(self.rbtnNPFoomatic, SIGNAL("toggled(bool)"), self.on_rbtnNPFoomatic_toggled)
        self.connect(self.filechooserPPDButton, SIGNAL("clicked()"),self.on_filechooserPPDButton)
        self.options = {} # keyword -> Option object
        self.changed = set()
        self.conflicts = set()
        self.ppd = None
        
        # Synchronisation objects.
        self.ppds_lock = thread.allocate_lock()
        self.devices_lock = thread.allocate_lock()
        self.smb_lock = thread.allocate_lock()
        self.ipp_lock = thread.allocate_lock()
        self.drivers_lock = thread.allocate_lock()

        #self.connect(self.btnNCAddMember, SIGNAL("clicked()"), self.slot_btnNCAddMember_clicked)
        #self.connect(self.btnNCDelMember, SIGNAL("clicked()"), self.slot_btnNCDelMember_clicked)

        # Optionally disable downloadable driver support.
        #if not config.DOWNLOADABLE_DRIVER_SUPPORT:
        if True:
            self.rbtnNPDownloadableDriverSearch.setEnabled(False)
            self.downloadableDriverSearchFrame.hide()

        """
        # Set up OpenPrinting widgets.
        self.openprinting = openprinting.OpenPrinting ()
        self.openprinting_query_handle = None
        combobox = self.cmbNPDownloadableDriverFoundPrinters
        cell = gtk.CellRendererText()
        combobox.pack_start (cell, True)
        combobox.add_attribute(cell, 'text', 0)

        # SMB browser
        self.smb_store = gtk.TreeStore (str, # host or share
                                        str, # comment
                                        gobject.TYPE_PYOBJECT, # domain dict
                                        gobject.TYPE_PYOBJECT) # host dict
        self.tvSMBBrowser.set_model (self.smb_store)
        self.smb_store.set_sort_column_id (0, gtk.SORT_ASCENDING)

        # SMB list columns
        col = gtk.TreeViewColumn (_("Share"), gtk.CellRendererText (),
                                  text=0)
        col.set_resizable (True)
        col.set_sort_column_id (0)
        self.tvSMBBrowser.append_column (col)

        col = gtk.TreeViewColumn (_("Comment"), gtk.CellRendererText (),
                                  text=1)
        self.tvSMBBrowser.append_column (col)
        slct = self.tvSMBBrowser.get_selection ()
        slct.set_select_function (self.smb_select_function)

        self.SMBBrowseDialog.set_transient_for(self.NewPrinterWindow)

        # IPP browser
        self.ipp_store = gtk.TreeStore (str, # queue
                                        str, # location
                                        gobject.TYPE_PYOBJECT) # dict
        self.tvIPPBrowser.set_model (self.ipp_store)
        self.ipp_store.set_sort_column_id (0, gtk.SORT_ASCENDING)

        # IPP list columns
        col = gtk.TreeViewColumn (_("Queue"), gtk.CellRendererText (),
                                  text=0)
        col.set_resizable (True)
        col.set_sort_column_id (0)
        self.tvIPPBrowser.append_column (col)

        col = gtk.TreeViewColumn (_("Location"), gtk.CellRendererText (),
                                  text=1)
        self.tvIPPBrowser.append_column (col)
        self.IPPBrowseDialog.set_transient_for(self.NewPrinterWindow)

        self.tvNPDriversTooltips = TreeViewTooltips(self.tvNPDrivers, self.NPDriversTooltips)

        ppd_filter = gtk.FileFilter()
        ppd_filter.set_name(_("PostScript Printer Description files (*.ppd, *.PPD, *.ppd.gz, *.PPD.gz, *.PPD.GZ)"))
        ppd_filter.add_pattern("*.ppd")
        ppd_filter.add_pattern("*.PPD")
        ppd_filter.add_pattern("*.ppd.gz")
        ppd_filter.add_pattern("*.PPD.gz")
        ppd_filter.add_pattern("*.PPD.GZ")
        self.filechooserPPD.add_filter(ppd_filter)

        ppd_filter = gtk.FileFilter()
        ppd_filter.set_name(_("All files (*)"))
        ppd_filter.add_pattern("*")
        self.filechooserPPD.add_filter(ppd_filter)

        self.xml.signal_autoconnect(self)
        """

        #FIXME hide bits which are not yet implemented
        self.btnSMBBrowse.hide()
        self.btnSMBVerify.hide()
        self.btnNPTLpdProbe.hide()

    def option_changed(self, option):
        if option.is_changed():
            self.changed.add(option)
        else:
            self.changed.discard(option)

        if option.conflicts:
            self.conflicts.add(option)
        else:
            self.conflicts.discard(option)
        self.setDataButtonState()

        return

    def setDataButtonState(self):
        self.btnNPForward.setEnabled(not bool(self.conflicts))

    def init(self, dialog_mode):
        self.dialog_mode = dialog_mode
        self.options = {} # keyword -> Option object
        self.changed = set()
        self.conflicts = set()

        """
        combobox = self.cmbNPDownloadableDriverFoundPrinters
        combobox.set_model (gtk.ListStore (str, str))
        self.entNPDownloadableDriverSearch.set_text ('')
        button = self.btnNPDownloadableDriverSearch
        label = button.get_children ()[0].get_children ()[0].get_children ()[1]
        self.btnNPDownloadableDriverSearch_label = label
        label.set_text (_("Search"))
        """

        if self.dialog_mode == "printer":
            self.setWindowTitle(i18n("New Printer"))
            # Start on devices page (1, not 0)
            self.ntbkNewPrinter.setCurrentIndex(self.ntbkNewPrinterPages["device"])
            self.fillDeviceTab()
            self.on_rbtnNPFoomatic_toggled()
            # Start fetching information from CUPS in the background
            self.new_printer_PPDs_loaded = False
            self.queryPPDs ()
        elif self.dialog_mode == "class":
            self.setWindowTitle(i18n("New Class"))
            self.fillNewClassMembers()
            # Start on name page
            self.ntbkNewPrinter.setCurrentIndex(self.ntbkNewPrinterPages["name"])
        elif self.dialog_mode == "device":
            self.setWindowTitle(i18n("Change Device URI"))
            self.ntbkNewPrinter.setCurrentIndex(self.ntbkNewPrinterPages["device"])
            self.queryDevices ()
            self.loadPPDs()
            self.fillDeviceTab(self.mainapp.printer.device_uri)
            # Start fetching information from CUPS in the background
            self.new_printer_PPDs_loaded = False
            self.queryPPDs ()
        elif self.dialog_mode == "ppd":
            self.setWindowTitle(i18n("Change Driver"))
            self.ntbkNewPrinter.setCurrentIndex(2)
            self.on_rbtnNPFoomatic_toggled()

            self.auto_model = ""
            ppd = self.mainapp.ppd
            if ppd:
                attr = ppd.findAttr("Manufacturer")
                if attr:
                    self.auto_make = attr.value
                else:
                    self.auto_make = ""
                attr = ppd.findAttr("ModelName")
                if not attr: attr = ppd.findAttr("ShortNickName")
                if not attr: attr = ppd.findAttr("NickName")
                if attr:
                    if attr.value.startswith(self.auto_make):
                        self.auto_model = attr.value[len(self.auto_make):].strip ()
                    else:
                        try:
                            self.auto_model = attr.value.split(" ", 1)[1]
                        except IndexError:
                            self.auto_model = ""
                else:
                    self.auto_model = ""
            else:
                # Special CUPS names for a raw queue.
                self.auto_make = 'Raw'
                self.auto_model = 'Queue'

            self.loadPPDs ()
            self.fillMakeList()

        if self.dialog_mode in ("printer", "class"):
            self.entNPName.setText (self.mainapp.makeNameUnique(self.dialog_mode))
            #FIXMEself.entNPName.grab_focus()
            for widget in [self.entNPLocation,
                           self.entNPDescription]: #,
                           #self.entSMBURI, self.entSMBUsername,
                           #self.entSMBPassword, self.entNPTDirectJetHostname]:
                widget.setText('')

            try:
                p = os.popen ('/bin/hostname', 'r')
                hostname = p.read ().strip ()
                p.close ()
                self.entNPLocation.setText(hostname)
            except:
                nonfatalException ()

        self.entNPTDirectJetPort.setText('9100')
        self.setNPButtons()
        self.exec_()

    # get PPDs

    def queryPPDs(self):
        debugprint ("queryPPDs")
        if not self.ppds_lock.acquire(0):
            debugprint ("queryPPDs: in progress")
            return
        debugprint ("Lock acquired for PPDs thread")

        self.getPPDs_thread(self.language[0])
        debugprint ("PPDs thread started")

    def getPPDs_thread(self, language):
        try:
            debugprint ("Connecting (PPDs)")
            cups.setServer (self.mainapp.connect_server)
            cups.setUser (self.mainapp.connect_user)
            cups.setPasswordCB (self.mainapp.cupsPasswdCallback)
            # cups.setEncryption (...)
            c = cups.Connection ()
            debugprint ("Fetching PPDs")
            ppds_dict = c.getPPDs()
            self.ppds_result = cupshelpers.ppds.PPDs(ppds_dict,
                                                     language=language)
            debugprint ("Closing connection (PPDs)")
            del c
        except cups.IPPError, (e, msg):
            self.ppds_result = cups.IPPError (e, msg)
        except:
            nonfatalException()
            self.ppds_result = { }

        debugprint ("Releasing PPDs lock")
        self.ppds_lock.release ()

    def fetchPPDs(self, parent=None):
        debugprint ("fetchPPDs")
        self.queryPPDs()
        time.sleep (0.1)

        # Keep the UI refreshed while we wait for the devices to load.
        waiting = False
        while (self.ppds_lock.locked()):
            if not waiting:
                waiting = True
                self.WaitWindow.setText(i18n('<b>Searching</b>') + '<br /><br />' +
                                         i18n('Searching for drivers'))
                self.WaitWindow.show ()

            KApplication.processEvents()

            time.sleep (0.1)

        if waiting:
            self.WaitWindow.hide ()

        debugprint ("Got PPDs")
        result = self.ppds_result # atomic operation
        if isinstance (result, cups.IPPError):
            # Propagate exception.
            raise result
        return result

    def loadPPDs(self, parent=None):
        try:
            return self.ppds
        except:
            self.ppds = self.fetchPPDs (parent=parent)
            return self.ppds

    def dropPPDs(self):
        try:
            del self.ppds
        except:
            pass

    # Class members

    def fillNewClassMembers(self):
        self.tvNCMembers.clear()
        self.tvNCNotMembers.clear()
        for printer in self.mainapp.printers.itervalues():
            self.tvNCNotMembers.addItem(printer.name)

    @pyqtSignature("")
    def on_btnNCAddMember_clicked(self):
        self.mainapp.moveClassMembers(self.tvNCNotMembers, self.tvNCMembers)
        self.btnNPApply.setEnabled(
            bool(self.mainapp.getCurrentClassMembers(self.tvNCMembers)))

    @pyqtSignature("")
    def on_btnNCDelMember_clicked(self):
        self.mainapp.moveClassMembers(self.tvNCMembers, self.tvNCNotMembers)
        self.btnNPApply.setEnabled(
            bool(self.mainapp.getCurrentClassMembers(self.tvNCMembers)))

    @pyqtSignature("")
    def on_btnNPBack_clicked(self):
        self.nextNPTab(-1)

    @pyqtSignature("")
    def on_btnNPForward_clicked(self):
        self.nextNPTab()

    def nextNPTab(self, step=1):
        self.setCursor(Qt.WaitCursor)
        page_nr = self.ntbkNewPrinter.currentIndex()

        if self.dialog_mode == "class":
            #order = [0, 4, 5]
            order = [self.ntbkNewPrinterPages["name"], self.ntbkNewPrinterPages["class-members"]]
        elif self.dialog_mode == "printer":
            if page_nr == 1: # Device (first page)
                # Choose an appropriate name.
                name = 'printer'
                try:
                    if self.device.id:
                        name = self.device.id_dict["MDL"]
                    name = self.mainapp.makeNameUnique (name)
                    self.entNPName.setText(name)
                except:
                    nonfatalException ()
                self.auto_make, self.auto_model = None, None
                self.device.uri = self.getDeviceURI()
                if self.device.type in ("socket", "lpd", "ipp", "bluetooth"):
                    host = self.getNetworkPrinterMakeModel(self.device)
                    faxuri = None
                    if host:
                        faxuri = self.get_hplip_uri_for_network_printer(host,
                                                                        "fax")
                    if faxuri:
                        #create message string
                        q = i18n("This printer supports both "
                                            "printing and sending faxes.  "
                                            "Which functionality should be "
                                            "used for this queue?")
                        
                        #buttons need to be KGuiItem objects
                        printer_button = KGuiItem(i18n("Printer"))
                        fax_button = KGuiItem(i18n("Fax"))

                        #This is a bit hackish. Essentially Printer is mapped to the Yes button, Fax to
                        #the No button. KMessageBox doesn't seem to provide a version for 'This OR That'
                        queue_type = KMessageBox.questionYesNo(self.NewPrinterWindow, q , 'Choose function', printer_button, fax_button)

                        #test return value of messagebox Printer == 3, Fax == 4
                        if (queue_type == 3): 
                            self.device.id_dict = \
                               self.get_hpfax_device_id(faxuri)
                            self.device.uri = faxuri
                            self.auto_make = self.device.id_dict["MFG"]
                            self.auto_model = self.device.id_dict["MDL"]
                            self.device.id = "MFG:" + self.auto_make + \
                                             ";MDL:" + self.auto_model + \
                                             ";DES:" + \
                                             self.device.id_dict["DES"] + ";"
                uri = self.device.uri
                if uri and uri.startswith ("smb://"):
                    uri = SMBURI (uri=uri[6:]).sanitize_uri ()

                # Try to access the PPD, in this case our detected IPP
                # printer is a queue on a remote CUPS server which is
                # not automatically set up on our local CUPS server
                # (for example DNS-SD broadcasted queue from Mac OS X)
                self.remotecupsqueue = None
                res = re.search ("ipp://(\S+(:\d+|))/printers/(\S+)", uri)
                if res:
                    resg = res.groups()
                    try:
                        conn = httplib.HTTPConnection(resg[0])
                        conn.request("GET", "/printers/%s.ppd" % resg[2])
                        resp = conn.getresponse()
                        if resp.status == 200: self.remotecupsqueue = resg[2]
                    except:
                        debugprint("exception in getting remotecupsqueue")
                        pass

                    # We also want to fetch the printer-info and
                    # printer-location attributes, to pre-fill those
                    # fields for this new queue.
                    oldserver = cups.getServer()
                    oldport = cups.getPort()
                    try:
                        cups.setServer (resg[0])
                        if len (resg[1]) > 0:
                            cups.setPort (int (resg[1]))
                        else:
                            cups.setPort (631)

                        c = cups.Connection ()
                        r = ['printer-info', 'printer-location']
                        attrs = c.getPrinterAttributes (uri=uri,
                                                        requested_attributes=r)
                        info = attrs.get ('printer-info', '')
                        location = attrs.get ('printer-location', '')
                        if len (info) > 0:
                            self.entNPDescription.setText(info)
                        if len (location) > 0:
                            self.entNPLocation.setText(location)
                    except:
                        nonfatalException ()

                    cups.setServer (oldserver)
                    cups.setPort (oldport)

                if (not self.remotecupsqueue and
                    not self.new_printer_PPDs_loaded):
                    try:
                        self.loadPPDs(self)
                    except cups.IPPError, (e, msg):
                        #self.ready (self)
                        self.show_IPP_Error(e, msg)
                        return
                    except:
                        self.ready (self)
                        return
                    self.new_printer_PPDs_loaded = True

                ppdname = None
                try:
                    if self.remotecupsqueue:
                        # We have a remote CUPS queue, let the client queue
                        # stay raw so that the driver on the server gets used
                        ppdname = 'raw'
                        self.ppd = ppdname
                        name = self.remotecupsqueue
                        name = self.mainapp.makeNameUnique (name)
                        self.entNPName.setText(name)
                    elif self.device.id:
                        id_dict = self.device.id_dict
                        (status, ppdname) = self.ppds.\
                            getPPDNameFromDeviceID (id_dict["MFG"],
                                                    id_dict["MDL"],
                                                    id_dict["DES"],
                                                    id_dict["CMD"],
                                                    self.device.uri)
                    else:
                        (status, ppdname) = self.ppds.\
                            getPPDNameFromDeviceID ("Generic",
                                                    "Printer",
                                                    "Generic Printer",
                                                    [],
                                                    self.device.uri)

                    if ppdname and not self.remotecupsqueue:
                        ppddict = self.ppds.getInfoFromPPDName (ppdname)
                        make_model = ppddict['ppd-make-and-model']
                        (make, model) = \
                            cupshelpers.ppds.ppdMakeModelSplit (make_model)
                        self.auto_make = make
                        self.auto_model = model
                except:
                    nonfatalException ()
                if not self.remotecupsqueue:
                    self.fillMakeList()
            elif page_nr == 3: # Model has been selected
                if not self.device.id:
                    # Choose an appropriate name when no Device ID
                    # is available, based on the model the user has
                    # selected.
                    try:
                        items = self.tvNPModels.selectedItems()
                        name = unicode(items[0].text())
                        name = self.mainapp.makeNameUnique (name)
                        self.entNPName.setText(name)
                    except:
                        nonfatalException ()

            ##self.ready (self.NewPrinterWindow)
            if self.remotecupsqueue:
                order = [1, 0]
            elif self.rbtnNPFoomatic.isChecked():
                order = [1, 2, 3, 6, 0]
            elif self.rbtnNPPPD.isChecked():
                order = [1, 2, 6, 0]
            else:
                # Downloadable driver
                order = [1, 2, 7, 6, 0]
        elif self.dialog_mode == "device":
            order = [1]
        elif self.dialog_mode == "ppd":
            self.rbtnChangePPDasIs.setChecked(True)
            if self.rbtnNPFoomatic.isChecked():
                order = [2, 3, 5, 6]
            elif self.rbtnNPPPD.isChecked():
                order = [2, 5, 6]
            else:
                # Downloadable driver
                order = [2, 7, 5, 6]

        next_page_nr = order[order.index(page_nr)+step]

        # fill Installable Options tab
        if next_page_nr == 6 and step > 0:
            #TODO Prepare Installable Options screen.
            self.ppd = self.getNPPPD()
            """FIXME todo
            if next_page_nr == 6:
                # Prepare Installable Options screen.
                if isinstance(self.ppd, cups.PPD):
                    self.fillNPInstallableOptions()
                else:
                    self.installable_options = None
                    # Put a label there explaining why the page is empty.
                    ppd = self.ppd
                    self.ppd = None
                    self.fillNPInstallableOptions()
                    self.ppd = ppd

                # step over if empty and not in PPD mode
                if self.dialog_mode != "ppd" and not self.installable_options:
                    next_page_nr = order[order.index(next_page_nr)+1]
            """
            next_page_nr = order[order.index(next_page_nr)+1]
        self.installable_options = False
        # Step over empty Installable Options tab
        if next_page_nr == 6 and not self.installable_options and step<0:
            next_page_nr = order[order.index(next_page_nr)-1]

        if next_page_nr == 7: # About to show downloadable drivers
            if self.drivers_lock.locked ():
                # Still searching for drivers.
                self.lblWait.set_markup ('<span size="larger">' +
                                         i18n('<b>Searching</b>') + '</span>\n\n' +
                                         i18n('Searching for drivers'))
                self.WaitWindow.set_transient_for (self.NewPrinterWindow)
                self.WaitWindow.show ()

                # Keep the UI refreshed while we wait for the drivers
                # query to complete.
                while self.drivers_lock.locked ():
                    self.mainapp.busy(self)
                    time.sleep (0.1)

                self.ready (self.NewPrinterWindow)
                self.WaitWindow.hide ()

            self.fillDownloadableDrivers()

        self.ntbkNewPrinter.setCurrentIndex(next_page_nr)

        self.setNPButtons()
        self.setCursor(Qt.ArrowCursor)

    def setNPButtons(self):
        nr = self.ntbkNewPrinter.currentIndex()

        if self.dialog_mode == "device":
            self.btnNPBack.hide()
            self.btnNPForward.hide()
            self.btnNPApply.show()
            uri = self.getDeviceURI ()
            self.btnNPApply.setEnabled(validDeviceURI (uri))
            return

        if self.dialog_mode == "ppd":
            if nr == 5: # Apply
                if not self.installable_options:
                    # There are no installable options, so this is the
                    # last page.
                    debugprint ("No installable options")
                    self.btnNPForward.hide ()
                    self.btnNPApply.show ()
                else:
                    self.btnNPForward.show ()
                    self.btnNPApply.hide ()
                return
            elif nr == 6:
                self.btnNPForward.hide()
                self.btnNPApply.show()
                return
            else:
                self.btnNPForward.show()
                self.btnNPApply.hide()
            if nr == 2:
                self.btnNPBack.hide()
                self.btnNPForward.show()
                downloadable_selected = False
                if self.rbtnNPDownloadableDriverSearch.isChecked():
                    combobox = self.cmbNPDownloadableDriverFoundPrinters
                    iter = combobox.get_active_iter () #FIXME
                    if iter and combobox.get_model ().get_value (iter, 1):
                        downloadable_selected = True

                self.btnNPForward.setEnabled(bool(
                        self.rbtnNPFoomatic.isChecked() or
                        not self.filechooserPPD.text().isEmpty() or
                        downloadable_selected))
                return
            else:
                self.btnNPBack.show()

        # class/printer

        if nr == 1: # Device
            valid = False
            try:
                uri = self.getDeviceURI ()
                valid = validDeviceURI (uri)
            except:
                pass
            self.btnNPForward.setEnabled(valid)
            self.btnNPBack.hide ()
        else:
            self.btnNPBack.show()

        self.btnNPForward.show()
        self.btnNPApply.hide()

        if nr == 0: # Name
            self.btnNPBack.show()
            if self.dialog_mode == "class":
                self.btnNPForward.setEnabled(True)
            if self.dialog_mode == "printer":
                self.btnNPForward.hide()
                self.btnNPApply.show()
                self.btnNPApply.setEnabled(
                    self.mainapp.checkNPName(unicode(self.entNPName.text())))
        if nr == 2: # Make/PPD file
            downloadable_selected = False
            if self.rbtnNPDownloadableDriverSearch.isChecked():
                combobox = self.cmbNPDownloadableDriverFoundPrinters
                iter = combobox.get_active_iter () #FIXME
                if iter and combobox.get_model ().get_value (iter, 1):
                    downloadable_selected = True

            self.btnNPForward.setEnabled(bool(
                self.rbtnNPFoomatic.isChecked() or
                not self.filechooserPPD.text().isEmpty() or
                downloadable_selected))
        
        if nr == 3: # Model/Driver
            iter = self.tvNPDrivers.currentItem()
            self.btnNPForward.setEnabled(bool(iter))
        if nr == 4: # Class Members
            self.btnNPForward.hide()
            self.btnNPApply.show()
            self.btnNPApply.setEnabled(self.tvNCMembers.count() > 0)
        if nr == 7: # Downloadable drivers
            if self.ntbkNPDownloadableDriverProperties.get_current_page() == 1: #FIXME
                accepted = self.rbtnNPDownloadLicenseYes.get_active ()
            else:
                treeview = self.tvNPDownloadableDrivers
                model, iter = treeview.get_selection ().get_selected ()
                accepted = (iter != None)

            self.btnNPForward.set_sensitive(accepted)
    
    def on_filechooserPPDButton(self):
        home = QDir.homePath()
        url = KUrl.fromPath(home)
        filename = KFileDialog.getOpenFileName(url, '*.ppd', self)
        self.filechooserPPD.setText(filename)
        self.btnNPForward.setEnabled(True)

    def getDevices_thread(self):
        try:
            debugprint ("Connecting (devices)")
            cups.setServer (self.mainapp.connect_server)
            #cups.setUser (self.mainapp.connect_user)
            cups.setUser ("jr")
            cups.setPasswordCB (self.mainapp.cupsPasswdCallback)
            # cups.setEncryption (...)
            c = cups.Connection ()
            debugprint ("Fetching devices")
            self.devices_result = cupshelpers.getDevices(c)
        except cups.IPPError, (e, msg):
            self.devices_result = cups.IPPError (e, msg)
        except:
            debugprint ("Exception in getDevices_thread")
            self.devices_result = {}

        try:
            debugprint ("Closing connection (devices)")
            del c
        except:
            pass

        debugprint ("Releasing devices lock")
        self.devices_lock.release ()


    # Device URI
    def queryDevices(self):
        if not self.devices_lock.acquire(0):
            debugprint ("queryDevices: in progress")
            return
        debugprint ("Lock acquired for devices thread")
        self.getDevices_thread()
        debugprint ("Devices thread started")

    def fetchDevices(self, parent=None):
        debugprint ("fetchDevices")
        self.queryDevices ()
        time.sleep (0.1)

        # Keep the UI refreshed while we wait for the devices to load.
        waiting = False
        #crashes kcm here as soon as this loop exits
        while (self.devices_lock.locked()):
            if not waiting:
                waiting = True
                self.WaitWindow.setText (i18n('<b>Searching</b>') + '<br/><br/>' +
                                         i18n('Searching for printers'))
                if not parent:
                   parent = self.mainapp
                self.WaitWindow.show ()
            
            KApplication.processEvents()

            time.sleep (0.1)
        
        #if waiting:
            #self.WaitWindow.hide()

        debugprint ("Got devices")
        
        result = self.devices_result # atomic operation

        if isinstance (result, cups.IPPError):
            # Propagate exception.
            raise result
        return result

    def get_hpfax_device_id(self, faxuri):
        os.environ["URI"] = faxuri
        cmd = 'LC_ALL=C DISPLAY= hp-info -d "${URI}"'
        debugprint (faxuri + ": " + cmd)
        try:
            p = subprocess.Popen (cmd, shell=True,
                                  stdin=file("/dev/null"),
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
            (stdout, stderr) = p.communicate ()
        except:
            # Problem executing command.
            return None

        for line in stdout.split ("\n"):
            if line.find ("fax-type") == -1:
                continue
            faxtype = -1
            res = re.search ("(\d+)", line)
            if res:
                resg = res.groups()
                faxtype = resg[0]
            if faxtype >= 0: break
        if faxtype < 0:
            return None
        elif faxtype == 4:
            return cupshelpers.parseDeviceID ('MFG:HP;MDL:Fax 2;DES:HP Fax 2;')
        else:
            return cupshelpers.parseDeviceID ('MFG:HP;MDL:Fax;DES:HP Fax;')

    def get_hplip_uri_for_network_printer(self, host, mode):
        os.environ["HOST"] = host
        if mode == "print": mod = "-c"
        elif mode == "fax": mod = "-f"
        else: mod = "-c"
        cmd = 'hp-makeuri ' + mod + ' "${HOST}"'
        debugprint (host + ": " + cmd)
        uri = None
        try:
            p = subprocess.Popen (cmd, shell=True,
                                  stdin=file("/dev/null"),
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
            (stdout, stderr) = p.communicate ()
        except:
            # Problem executing command.
            return None

        uri = stdout.strip ()
        return uri

    def getNetworkPrinterMakeModel(self, device):
        # Determine host name/IP
        host = None
        s = device.uri.find ("://")
        if s != -1:
            s += 3
            e = device.uri[s:].find (":")
            if e == -1: e = device.uri[s:].find ("/")
            if e == -1: e = device.uri[s:].find ("?")
            if e == -1: e = len (device.uri)
            host = device.uri[s:s+e]
        # Try to get make and model via SNMP
        if host:
            os.environ["HOST"] = host
            cmd = '/usr/lib/cups/backend/snmp "${HOST}"'
            debugprint (host + ": " + cmd)
            stdout = None
            try:
                p = subprocess.Popen (cmd, shell=True,
                                      stdin=file("/dev/null"),
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
                (stdout, stderr) = p.communicate ()
            except:
                # Problem executing command.
                pass

            if stdout != None:
                mm = re.sub("^\s*\S+\s+\S+\s+\"", "", stdout)
                mm = re.sub("\"\s+.*$", "", mm)
                if mm and mm != "": device.make_and_model = mm
        # Extract make and model and create a pseudo device ID, so
        # that a PPD/driver can be assigned to the device
        make_and_model = None
        if len (device.make_and_model) > 7:
            make_and_model = device.make_and_model
        elif len (device.info) > 7:
            make_and_model = device.info
            make_and_model = re.sub("\s*(\(|\d+\.\d+\.\d+\.\d+).*$", "", make_and_model)
        if make_and_model and not device.id:
            mk = None
            md = None
            (mk, md) = cupshelpers.ppds.ppdMakeModelSplit (make_and_model)
            device.id = "MFG:" + mk + ";MDL:" + md + ";DES:" + mk + " " + md + ";"
            device.id_dict = cupshelpers.parseDeviceID (device.id)
        # Check whether the device is supported by HPLIP and replace
        # its URI by an HPLIP URI
        if host:
            hplipuri = self.get_hplip_uri_for_network_printer(host, "print")
            if hplipuri:
                device.uri = hplipuri
                s = hplipuri.find ("/usb/")
                if s == -1: s = hplipuri.find ("/par/")
                if s == -1: s = hplipuri.find ("/net/")
                if s != -1:
                    s += 5
                    e = hplipuri[s:].find ("?")
                    if e == -1: e = len (hplipuri)
                    mdl = hplipuri[s:s+e].replace ("_", " ")
                    if mdl.startswith ("hp ") or mdl.startswith ("HP "):
                        mdl = mdl[3:]
                        device.make_and_model = "HP " + mdl
                        device.id = "MFG:HP;MDL:" + mdl + ";DES:HP " + mdl + ";"
                        device.id_dict = cupshelpers.parseDeviceID (device.id)
        # Return the host name/IP for further actions
        return host

    def fillDeviceTab(self, current_uri=None, query=True):
        if query:
            try:
                devices = self.fetchDevices()
            except cups.IPPError, (e, msg):
                self.show_IPP_Error(e, msg)
                devices = {}
            except:
                nonfatalException()
                devices = {}

            if current_uri:
                if devices.has_key (current_uri):
                    current = devices.pop(current_uri)
                else:
                    current = cupshelpers.Device (current_uri)
                    current.info = "Current device"

            self.devices = devices.values()

        for device in self.devices:
            if device.type == "usb":
                # Find USB URIs with corresponding HPLIP URIs and mark them
                # for deleting, so that the user will only get the HPLIP
                # URIs for full device support in the list
                ser = None
                s = device.uri.find ("?serial=")
                if s != -1:
                    s += 8
                    e = device.uri[s:].find ("?")
                    if e == -1: e = len (device.uri)
                    ser = device.uri[s:s+e]
                mod = None
                s = device.uri[6:].find ("/")
                if s != -1:
                    s += 7
                    e = device.uri[s:].find ("?")
                    if e == -1: e = len (device.uri)
                    mod = device.uri[s:s+e].lower ().replace ("%20", "_")
                    if mod.startswith ("hp_"):
                        mod = mod[3:]
                matchfound = 0
                for hpdevice in self.devices:
                    hpser = None
                    hpmod = None
                    uri = hpdevice.uri
                    if not uri.startswith ("hp:"): continue
                    if ser:
                        s = uri.find ("?serial=")
                        if s != -1:
                            s += 8
                            e = uri[s:].find ("?")
                            if e == -1: e = len (uri)
                            hpser = uri[s:s+e]
                            if hpser != ser: continue
                            matchfound = 1
                    if mod and not (ser and hpser):
                        s = uri.find ("/usb/")
                        if s != -1:
                            s += 5
                            e = uri[s:].find ("?")
                            if e == -1: e = len (uri)
                            hpmod = uri[s:s+e].lower ()
                            if hpmod.startswith ("hp_"):
                                hpmod = hpmod[3:]
                            if hpmod != mod: continue
                            matchfound = 1
                    if matchfound == 1: break
                if matchfound == 1:
                    device.uri = "delete"
            if device.type == "hal":
                # Remove HAL USB URIs, for these printers there are already
                # USB URIs
                if device.uri.startswith("hal:///org/freedesktop/Hal/devices/usb_device"):
                    device.uri = "delete"
            if device.type == "socket":
                # Remove default port to more easily find duplicate URIs
                device.uri = device.uri.replace (":9100", "")
            try:
                ## XXX This needs to be moved to *after* the device is
                # selected.  Looping through all the network printers like
                # this is far too slow.
                if False and device.type in ("socket", "lpd", "ipp", "bluetooth"):
                    host = self.getNetworkPrinterMakeModel(device)
                    faxuri = None
                    if host:
                        faxuri = self.get_hplip_uri_for_network_printer(host,
                                                                        "fax")
                    if faxuri:
                        self.devices.append(cupshelpers.Device(faxuri,
                              **{'device-class' : "direct",
                                 'device-info' : device.info + " HP Fax HPLIP",
                                 'device-device-make-and-model' : "HP Fax",
                                 'device-id' : "MFG:HP;MDL:Fax;DES:HP Fax;"}))
                    if device.uri.startswith ("hp:"):
                        device.type = "hp" 
                        device.info += (" HPLIP")
            except:
                nonfatalException ()
        # Mark duplicate URIs for deletion
        for i in range (len (self.devices)):
            for j in range (len (self.devices)):
                if i == j: continue
                device1 = self.devices[i]
                device2 = self.devices[j]
                if device1.uri == "delete" or device2.uri == "delete":
                    continue
                if device1.uri == device2.uri:
                    # Keep the one with the longer (better) device ID
                    if (not device1.id):
                        device1.uri = "delete"
                    elif (not device2.id):
                        device2.uri = "delete"
                    elif (len (device1.id) < len (device2.id)):
                        device1.uri = "delete"
                    else:
                        device2.uri = "delete"
        self.devices = filter(lambda x: x.uri not in ("hp:/no_device_found",
                                                      "hpfax:/no_device_found",
                                                      "hp", "hpfax",
                                                      "hal", "beh",
                                                      "scsi", "http", "delete"),
                              self.devices)
        self.devices.sort()

        self.devices.append(cupshelpers.Device('',
             **{'device-info' :i18nc("Other device", "Other")}))
        if current_uri:
            current.info = i18nc("Current device", "%1 (Current)", current.info)
            self.devices.insert(0, current)
            self.device = current
        self.tvNPDevices.clear()

        for device in self.devices:
            self.tvNPDevices.addItem(device.info)

        #self.tvNPDevices.get_selection().select_path(0)
        self.tvNPDevices.setCurrentRow(0)
        self.on_tvNPDevices_cursor_changed()

    def on_entNPTDevice_changed(self, entry):
        self.setNPButtons()

    #TODO
    ## SMB browsing
    def on_entSMBURI_changed (self, text):
        uri = unicode(text)
        (group, host, share, user, password) = SMBURI (uri=uri).separate ()
        if user:
            self.entSMBUsername.setText(user)
        if password:
            self.entSMBPassword.setText(password)
        if user or password:
            uri = SMBURI (group=group, host=host, share=share).get_uri ()
            self.entSMBURI.setText(uri)
            self.rbtnSMBAuthSet.setChecked(True)
        elif unicode(self.entSMBUsername.text()) == '':
            self.rbtnSMBAuthPrompt.setChecked(True)

        self.btnSMBVerify.setEnabled(bool(uri))

    def on_rbtnSMBAuthSet_toggled(self, ticked):
        self.tblSMBAuth.setEnabled(ticked)

    def on_entNPTIPPHostname_textChanged(self):
        valid = len (self.entNPTIPPHostname.text ()) > 0
        self.btnIPPFindQueue.setEnabled(valid)
        self.update_IPP_URI_label ()

    ### IPP Browsing
    def update_IPP_URI_label(self):
        hostname = unicode(self.entNPTIPPHostname.text())
        queue = unicode(self.entNPTIPPQueuename.text())
        valid = len (hostname) > 0 and queue != '/printers/'

        if valid:
            uri = "ipp://%s%s" % (hostname, queue)
            self.lblIPPURI.setText(uri)
            self.lblIPPURI.show ()
            self.entNPTIPPQueuename.show ()
        else:
            self.lblIPPURI.hide ()

        self.btnIPPVerify.setEnabled(valid)
        self.setNPButtons ()

    @pyqtSignature("")
    def on_btnIPPFindQueue_clicked(self):
        self.mainapp.busy()
        self.entNPTIPPQueuename.clear()
        IPPBrowseDialog(self)
        self.mainapp.ready()
    
    @pyqtSignature("")
    def on_btnIPPVerify_clicked(self):
        uri = unicode(self.lblIPPURI.text())
        match = re.match ("(ipp|https?)://([^/]+)(.*)/([^/]*)", uri)
        verified = False
        if match:
            oldserver = cups.getServer ()
            host = match.group(2)
            try:
                cups.setServer (match.group (2))
                c = cups.Connection ()
                attributes = c.getPrinterAttributes (uri = uri)
                verified = True
            except cups.IPPError, (e, msg):
                debugprint ("Failed to get attributes: %s (%d)" % (msg, e))
            except:
                nonfatalException ()
            cups.setServer (oldserver)
        else:
            debugprint (uri)

        if verified:
            KMessageBox.information (self, _("This print share is accessible."),
                                    _("Print Share Verified"))
        else:
            KMessageBox.error (self, _("This print share is not accessible."),
                               _("Inaccessible"))

    def on_tvNPDevices_cursor_changed(self):
        device = self.devices[self.tvNPDevices.currentRow()]
        self.device = device
        self.lblNPDeviceDescription.setText('')
        page = self.new_printer_device_tabs.get(device.type, 1)
        self.ntbkNPType.setCurrentIndex(page)

        type = device.type
        url = device.uri.split(":", 1)[-1]
        if page == 0:
            # This is the "no options" page, with just a label to describe
            # the selected device.
            if device.type == "parallel":
                text = i18n("A printer connected to the parallel port.")
            elif device.type == "usb":
                text = i18n("A printer connected to a USB port.")
            elif device.type == "hp":
                text = i18n("HPLIP software driving a printer, "
                         "or the printer function of a multi-function device.")
            elif device.type == "hpfax":
                text = i18n("HPLIP software driving a fax machine, "
                         "or the fax function of a multi-function device.")
            elif device.type == "hal":
                text = i18n("Local printer detected by the "
                         "Hardware Abstraction Layer (HAL).")
            else:
                text = device.uri

            self.lblNPDeviceDescription.setText(text)
        elif device.type=="socket":
            if device.uri.startswith ("socket"):
                host = device.uri[9:]
                i = host.find (":")
                if i != -1:
                    port = int (host[i + 1:])
                    host = host[:i]
                else:
                    port = 9100

                self.entNPTDirectJetHostname.setText(host)
                self.entNPTDirectJetPort.setText(str (port))
        elif device.type=="serial":
            pass
            """
            if not device.is_class:
                options = device.uri.split("?")[1]
                options = options.split("+")
                option_dict = {}
                for option in options:
                    name, value = option.split("=")
                    option_dict[name] = value
                    
                for widget, name, optionvalues in (
                    (self.cmbNPTSerialBaud, "baud", None),
                    (self.cmbNPTSerialBits, "bits", None),
                    (self.cmbNPTSerialParity, "parity",
                     ["none", "odd", "even"]),
                    (self.cmbNPTSerialFlow, "flow",
                     ["none", "soft", "hard", "hard"])):
                    if option_dict.has_key(name): # option given in URI?
                        if optionvalues is None: # use text in widget
                            model = widget.get_model()
                            iter = model.get_iter_first()
                            nr = 0
                            while iter:
                                value = model.get(iter,0)[0]
                                if value == option_dict[name]:
                                    widget.set_active(nr)
                                    break
                                iter = model.iter_next(iter)
                                nr += 1
                        else: # use optionvalues
                            nr = optionvalues.index(
                                option_dict[name])
                            widget.set_active(nr+1) # compensate "Default"
                    else:
                        widget.set_active(0)
            """

        # XXX FILL TABS FOR VALID DEVICE URIs
        elif device.type in ("ipp", "http"):
            if (device.uri.startswith ("ipp:") or
                device.uri.startswith ("http:")):
                match = re.match ("(ipp|https?)://([^/]+)(.*)", device.uri)
                if match:
                    server = match.group (2)
                    printer = match.group (3)
                else:
                    server = ""
                    printer = ""

                self.entNPTIPPHostname.setText(server)
                self.entNPTIPPQueuename.setText(printer)
                self.lblIPPURI.setText(device.uri)
                self.lblIPPURI.show()
                self.entNPTIPPQueuename.show()
            else:
                self.entNPTIPPHostname.setText('')
                self.entNPTIPPQueuename.setText('/printers/')
                self.entNPTIPPQueuename.show()
                self.lblIPPURI.hide()
        elif device.type=="lpd":
            if device.uri.startswith ("lpd"):
                host = device.uri[6:]
                i = host.find ("/")
                if i != -1:
                    printer = host[i + 1:]
                    host = host[:i]
                else:
                    printer = ""
                self.cmbentNPTLpdHost.addItem(host)
                self.cmbentNPTLpdQueue.addItem(printer)
        elif device.uri == "lpd":
            pass
        elif device.uri == "smb":
            self.entSMBURI.setText('')
            self.btnSMBVerify.setEnabled(False)
        elif device.type == "smb":
            self.entSMBUsername.setText('')
            self.entSMBPassword.setText('')
            self.entSMBURI.setText(device.uri[6:])
            self.btnSMBVerify.setEnabled(True)
        else:
            self.entNPTDevice.setText(device.uri)

        self.setNPButtons()

    def getDeviceURI(self):
        type = self.device.type
        if type == "socket": # DirectJet
            host = unicode(self.entNPTDirectJetHostname.text())
            port = unicode(self.entNPTDirectJetPort.text())
            device = "socket://" + host
            if port:
                device = device + ':' + port
        elif type in ("http", "ipp"): # IPP
            if self.lblIPPURI.isVisible:
                device = unicode(self.lblIPPURI.text())
            else:
                device = "ipp"
        elif type == "lpd": # LPD
            host = unicode(self.cmbentNPTLpdHost.currentText())
            printer = unicode(self.cmbentNPTLpdQueue.currentText())
            device = "lpd://" + host
            if printer:
                device = device + "/" + printer
        elif type == "parallel": # Parallel
            device = self.device.uri
        elif type == "scsi": # SCSII
            device = ""
        elif type == "serial": # Serial
            pass
            """
            options = []
            for widget, name, optionvalues in (
                (self.cmbNPTSerialBaud, "baud", None),
                (self.cmbNPTSerialBits, "bits", None),
                (self.cmbNPTSerialParity, "parity",
                 ("none", "odd", "even")),
                (self.cmbNPTSerialFlow, "flow",
                 ("none", "soft", "hard", "hard"))):
                nr = widget.get_active()
                if nr:
                    if optionvalues is not None:
                        option = optionvalues[nr-1]
                    else:
                        option = widget.get_active_text()
                    options.append(name + "=" + option)
            options = "+".join(options)
            device =  self.device.uri.split("?")[0] #"serial:/dev/ttyS%s" 
            if options:
                device = device + "?" + options
            """
        elif type == "smb":
            uri = unicode(self.entSMBURI.text())
            (group, host, share, u, p) = SMBURI (uri=uri).separate ()
            user = ''
            password = ''
            if self.rbtnSMBAuthSet.isChecked():
                user = unicode(self.entSMBUsername.text())
                password = unicode(self.entSMBPassword.text())
            uri = SMBURI (group=group, host=host, share=share,
                          user=user, password=password).get_uri ()
            device = "smb://" + uri
        elif not self.device.is_class:
            device = self.device.uri
        else:
            device = str(self.entNPTDevice.text())
        return device
        # class/printer

        if nr == self.ntbkNewPrinterPages["device"]: # Device
            valid = False
            try:
                uri = self.getDeviceURI ()
                valid = validDeviceURI (uri)
            except:
                debugprint("exception in getDeviceURI()")
                pass
            self.btnNPForward.setEnabled(valid)
            self.btnNPBack.hide ()
        else:
            self.btnNPBack.show()

        self.btnNPForward.show()
        self.btnNPApply.hide()

        if nr == self.ntbkNewPrinterPages["name"]: # Name
            self.btnNPBack.show()
            if self.dialog_mode == "printer":
                self.btnNPForward.hide()
                self.btnNPApply.show()
                self.btnNPApply.setEnabled(
                    self.mainapp.checkNPName(self.entNPName.getText()))
        if nr == self.ntbkNewPrinterPages["make"]: # Make/PPD file
            downloadable_selected = False
            if self.rbtnNPDownloadableDriverSearch.get_active ():
                combobox = self.cmbNPDownloadableDriverFoundPrinters
                iter = combobox.get_active_iter ()
                if iter and combobox.get_model ().get_value (iter, 1):
                    downloadable_selected = True

            self.btnNPForward.setEnabled(bool(
                self.rbtnNPFoomatic.get_active() or
                not self.filechooserPPD.text().isEmpty() or
                downloadable_selected))
        if nr == self.ntbkNewPrinterPages["model"]: # Model/Driver
            model, iter = self.tvNPDrivers.get_selection().get_selected()
            self.btnNPForward.set_sensitive(bool(iter))
        if nr == self.ntbkNewPrinterPages["class-members"]: # Class Members
            self.btnNPForward.hide()
            self.btnNPApply.show()
            self.btnNPApply.setEnabled(
                bool(self.mainapp.getCurrentClassMembers(self.tvNCMembers)))
        if nr == self.ntbkNewPrinterPages["downloadable"]: # Downloadable drivers
            if self.ntbkNPDownloadableDriverProperties.get_current_page() == 1:
                accepted = self.rbtnNPDownloadLicenseYes.get_active ()
            else:
                accepted = True

            self.btnNPForward.set_sensitive(accepted)

    # PPD

    def on_rbtnNPFoomatic_toggled(self):
        rbtn1 = self.rbtnNPFoomatic.isChecked()
        rbtn2 = self.rbtnNPPPD.isChecked()
        rbtn3 = self.rbtnNPDownloadableDriverSearch.isChecked()
        self.tvNPMakes.setEnabled(rbtn1)
        self.filechooserPPD.setEnabled(rbtn2)
        
        """FIXME
        if not rbtn3 and self.openprinting_query_handle:
            # Need to cancel a search in progress.
            self.openprinting.cancelOperation (self.openprinting_query_handle)
            self.openprinting_query_handle = None
            self.btnNPDownloadableDriverSearch_label.setText(_("Search"))
            # Clear printer list.
            self.cmbNPDownloadableDriverFoundPrinters.clear()
        """

        for widget in [self.entNPDownloadableDriverSearch,
                       self.cmbNPDownloadableDriverFoundPrinters]:
            widget.setEnabled(rbtn3)
        self.btnNPDownloadableDriverSearch.\
            setEnabled(rbtn3 and (self.openprinting_query_handle == None))

        self.setNPButtons()

    def fillMakeList(self):
        makes = self.ppds.getMakes()
        self.tvNPMakes.clear()
        found = False
        index = 0
        for make in makes:
            self.tvNPMakes.addItem(make)
            index = index + 1
            if make==self.auto_make:
                self.tvNPMakes.setCurrentRow(index-1)
                found = True

        self.on_tvNPMakes_cursor_changed()

    def on_tvNPMakes_cursor_changed(self):
        items = self.tvNPMakes.selectedItems()
        if len(items) > 0:
            self.NPMake = unicode(items[0].text())
            self.fillModelList()

    def fillModelList(self):
        models = self.ppds.getModels(self.NPMake)
        self.tvNPModels.clear()
        selected = False
        index = 0
        selected = False
        for pmodel in models:
            self.tvNPModels.addItem(pmodel)
            if self.NPMake==self.auto_make and pmodel==self.auto_model:
                self.tvNPModels.setCurrentRow(index)
                selected = True
            index = index + 1
        if not selected:
            self.tvNPModels.setCurrentRow(0)
        ##self.tvNPModels.columns_autosize()
        self.on_tvNPModels_cursor_changed()

    def fillDriverList(self, pmake, pmodel):
        self.NPModel = pmodel
        self.tvNPDrivers.clear()

        ppds = self.ppds.getInfoFromModel(pmake, pmodel)

        self.NPDrivers = self.ppds.orderPPDNamesByPreference(ppds.keys()) 
        for i in range (len(self.NPDrivers)):
            ppd = ppds[self.NPDrivers[i]]
            driver = ppd["ppd-make-and-model"]
            driver = driver.replace(" (recommended)", "")

            try:
                lpostfix = " [%s]" % ppd["ppd-natural-language"]
                driver += lpostfix
            except KeyError:
                pass

            if i == 0:
                self.tvNPDrivers.addItem(i18nc("Recommended driver", "%1 (recommended)", driver))
                self.tvNPDrivers.setCurrentRow(0)
            else:
                self.tvNPDrivers.addItem(driver)
        ##self.tvNPDrivers.columns_autosize()

    def on_tvNPModels_cursor_changed(self):
        items = self.tvNPModels.selectedItems()
        if len(items) > 0:
            pmodel = unicode(items[0].text())
            self.fillDriverList(self.NPMake, pmodel)

    def getNPPPD(self):
        try:
            if self.rbtnNPFoomatic.isChecked():
                #items = self.tvNPDrivers.selectedItems()
                #nr = unicode(items[0])
                nr = self.tvNPDrivers.currentRow()
                ppd = self.NPDrivers[nr]
            elif self.rbtnNPPPD.isChecked():
                ppd = cups.PPD(unicode(self.filechooserPPD.text()))
            else:
                """FIXME
                # PPD of the driver downloaded from OpenPrinting XXX
                treeview = self.tvNPDownloadableDrivers
                model, iter = treeview.get_selection ().get_selected ()
                driver = model.get_value (iter, 1)
                if driver.has_key ('ppds'):
                    # Only need to download a PPD.
                    file_to_download = driver
                """

                ppd = "XXX"

        except RuntimeError, e:
            if self.rbtnNPFoomatic.isChecked():
                # Foomatic database problem of some sort.
                err_title = i18n('Database error')
                model, iter = (self.tvNPDrivers.get_selection().
                               get_selected())
                nr = model.get_path(iter)[0]
                driver = self.NPDrivers[nr]
                if driver.startswith ("gutenprint"):
                    # This printer references some XML that is not
                    # installed by default.  Point the user at the
                    # package they need to install.
                    err = i18n("You will need to install the '%1' package "
                            "in order to use this driver.",
                            "gutenprint-foomatic")
                else:
                    err = i18n("The '%1' driver cannot be "
                             "used with printer '%2 %3'.", driver, self.NPMake, self.NPModel)
            elif self.rbtnNPPPD.isChecked():
                # This error came from trying to open the PPD file.
                err_title = i18n('PPD error')
                filename = self.filechooserPPD.text()
                err = i18n('Failed to read PPD file.  Possible reason '
                        'follows:') + '\n'
                os.environ["PPD"] = filename
                # We want this to be in the current natural language,
                # so we intentionally don't set LC_ALL=C here.
                p = os.popen ('/usr/bin/cupstestppd -rvv "$PPD"', 'r')
                output = p.readlines ()
                p.close ()
                err += reduce (lambda x, y: x + y, output)
            else:
                # Failed to get PPD downloaded from OpenPrinting XXX
                err_title = i18n('Downloadable drivers')
                err_text = i18n("Support for downloadable "
                             "drivers is not yet completed.")

            error_text = ('<span size="larger">' +
                          i18nc("Error title", "<b>%1</b>", err_title) + '</span>\n\n' + err)
            KMessageBox.error(self, error_text, err_title)
            return None

        if isinstance(ppd, str) or isinstance(ppd, unicode):
            try:
                if (ppd != "raw"):
                    f = self.mainapp.cups.getServerPPD(ppd)
                    ppd = cups.PPD(f)
                    os.unlink(f)
            except AttributeError:
                nonfatalException()
                debugprint ("pycups function getServerPPD not available: never mind")
            except RuntimeError:
                nonfatalException()
                debugprint ("libcups from CUPS 1.3 not available: never mind")
            except cups.IPPError:
                nonfatalException()
                debugprint ("CUPS 1.3 server not available: never mind")

        return ppd

    # Create new Printer
    @pyqtSignature("")
    def on_btnNPApply_clicked(self):
        if self.dialog_mode in ("class", "printer"):
            name = unicode(self.entNPName.text())
            location = unicode(self.entNPLocation.text())
            info = unicode(self.entNPDescription.text())
        else:
            name = self.mainapp.printer.name

        #replace any whitespace in printer name with underscore otherwise
        #CUPS throws an error
        name = name.replace(" ", "_")
        
        # Whether to check for missing drivers.
        check = False
        checkppd = None
        ppd = self.ppd

        if self.dialog_mode=="class":
            members = self.mainapp.getCurrentClassMembers(self.tvNCMembers)
            try:
                for member in members:
                    self.passwd_retry = False # use cached Passwd 
                    self.mainapp.cups.addPrinterToClass(str(member), name)
            except cups.IPPError, (e, msg):
                self.show_IPP_Error(e, msg)
                return
        elif self.dialog_mode=="printer":
            self.device.uri = unicode(self.device.uri)
            uri = None
            if self.device.uri:
                uri = self.device.uri
            else:
                uri = self.getDeviceURI()
            if not self.ppd: # XXX needed?
                # Go back to previous page to re-select driver.
                self.nextNPTab(-1)
                return

            # write Installable Options to ppd
            for option in self.options.itervalues():
                option.writeback()

            self.mainapp.busy(self)
            self.WaitWindow.setText(i18n('<b>Adding</b>') + '<br /><br />' +
                                    i18n('Adding printer'))
            #self.WaitWindow.set_transient_for (self.NewPrinterWindow)
            self.WaitWindow.show ()
            KApplication.processEvents()
            try:
                self.passwd_retry = False # use cached Passwd
                if isinstance(ppd, str) or isinstance(ppd, unicode):
                    self.mainapp.cups.addPrinter(name, ppdname=ppd,
                         device=uri, info=info, location=location)
                    check = True
                elif ppd is None: # raw queue
                    self.mainapp.cups.addPrinter(name, device=uri,
                                         info=info, location=location)
                else:
                    cupshelpers.setPPDPageSize(ppd, self.language[0])
                    self.mainapp.cups.addPrinter(name, ppd=ppd,
                         device=uri, info=info, location=location)
                    check = True
                    checkppd = ppd
                cupshelpers.activateNewPrinter (self.mainapp.cups, name)
            except cups.IPPError, (e, msg):
                #self.ready(self)
                self.WaitWindow.hide ()
                self.show_IPP_Error(e, msg)
                return
            except:
                ##self.ready (self.NewPrinterWindow)
                self.WaitWindow.hide ()
                fatalException (1)
            self.WaitWindow.hide ()
            ##self.ready (self.NewPrinterWindow)
    #comment  
        if self.dialog_mode in ("class", "printer"):
            try:
                self.passwd_retry = False # use cached Passwd 
                self.mainapp.cups.setPrinterLocation(name, location)
                self.passwd_retry = False # use cached Passwd 
                self.mainapp.cups.setPrinterInfo(name, info)
            except cups.IPPError, (e, msg):
                self.show_IPP_Error(e, msg)
                return
        elif self.dialog_mode == "device":
            try:
                uri = self.getDeviceURI()
                self.mainapp.cups.addPrinter(name, device=uri)
            except cups.IPPError, (e, msg):
                self.show_IPP_Error(e, msg)
                return 
        elif self.dialog_mode == "ppd":
            if not ppd:
                ppd = self.ppd = self.getNPPPD()
                if not ppd:
                    # Go back to previous page to re-select driver.
                    self.nextNPTab(-1)
                    return

            # set ppd on server and retrieve it
            # cups doesn't offer a way to just download a ppd ;(=
            raw = False
            if isinstance(ppd, str) or isinstance(ppd, unicode):
                if self.rbtnChangePPDasIs.isChecked():
                    # To use the PPD as-is we need to prevent CUPS copying
                    # the old options over.  Do this by setting it to a
                    # raw queue (no PPD) first.
                    try:
                        self.mainapp.cups.addPrinter(name, ppdname='raw')
                    except cups.IPPError, (e, msg):
                        self.show_IPP_Error(e, msg)
                try:
                    self.mainapp.cups.addPrinter(name, ppdname=ppd)
                except cups.IPPError, (e, msg):
                    self.show_IPP_Error(e, msg)
                    return

                try:
                    filename = self.mainapp.cups.getPPD(name)
                    ppd = cups.PPD(filename)
                    os.unlink(filename)
                except cups.IPPError, (e, msg):
                    if e == cups.IPP_NOT_FOUND:
                        raw = True
                    else:
                        self.show_IPP_Error(e, msg)
                        return
            else:
                # We have an actual PPD to upload, not just a name.
                if not self.rbtnChangePPDasIs.isChecked():
                    cupshelpers.copyPPDOptions(self.mainapp.ppd, ppd) # XXX
                else:
                    # write Installable Options to ppd
                    for option in self.options.itervalues():
                        option.writeback()
                    cupshelpers.setPPDPageSize(ppd, self.language[0])

                try:
                    self.mainapp.cups.addPrinter(name, ppd=ppd)
                except cups.IPPError, (e, msg):
                    self.show_IPP_Error(e, msg)

            if not raw:
                check = True
                checkppd = ppd

        self.accept()
        self.mainapp.populateList(start_printer=name)
        if check:
            try:
                self.checkDriverExists (name, ppd=checkppd)
            except:
                nonfatalException()

            # Also check to see whether the media option has become
            # invalid.  This can happen if it had previously been
            # explicitly set to a page size that is not offered with
            # the new PPD (see bug #441836).
            """
            try:
                option = self.mainapp.server_side_options['media']
                if option.get_current_value () == None:
                    debugprint ("Invalid media option: resetting")
                    option.reset ()
                    self.mainapp.changed.add (option)
                    self.mainapp.save_printer (self.mainapp.printer)
            except KeyError:
                pass
            except:
                print "exception in check to see whether the media option has become invalid"
                nonfatalException()
            """

    def show_IPP_Error(self, exception, message):
            if exception == cups.IPP_NOT_AUTHORIZED:
                KMessageBox.error(self, i18n('The password may be incorrect.'), i18n('Not authorized'))
            else:
                KMessageBox.error(self, i18n("There was an error during the CUPS " "operation: '%1'.", message),
                                        i18n('CUPS server error'))

    def checkDriverExists(self, name, ppd=None):
        """Check that the driver for an existing queue actually
        exists, and prompt to install the appropriate package
        if not.

        ppd: cups.PPD object, if already created"""

        # Is this queue on the local machine?  If not, we can't check
        # anything at all.
        server = cups.getServer ()
        if not (server == 'localhost' or server == '127.0.0.1' or
                server == '::1' or server[0] == '/'):
            return

        # Fetch the PPD if we haven't already.
        if not ppd:
            try:
                filename = self.mainapp.cups.getPPD(name)
            except cups.IPPError, (e, msg):
                if e == cups.IPP_NOT_FOUND:
                    # This is a raw queue.  Nothing to check.
                    return
                else:
                    self.show_IPP_Error(e, msg)
                    return

            ppd = cups.PPD(filename)
            os.unlink(filename)

        (pkgs, exes) = cupshelpers.missingPackagesAndExecutables (ppd)
        if len (pkgs) > 0 or len (exes) > 0:
            # We didn't find a necessary executable.  Complain.
            install = "/usr/bin/system-install-packages"
            if len (pkgs) > 0 and os.access (install, os.X_OK):
                pkg = pkgs[0]
                install_text = ('<span weight="bold" size="larger">' +
                                i18n('Install driver') + '</span>\n\n' +
                                i18n("Printer '%1' requires the %2 package but "
                                  "it is not currently installed.", name, pkg))
                dialog = self.InstallDialog
                self.lblInstall.set_markup(install_text)
            else:
                error_text = ('<span weight="bold" size="larger">' +
                              i18n('Missing driver') + '</span>\n\n' +
                              i18n("Printer '%1' requires the '%2' program but "
                                "it is not currently installed.  Please "
                                "install it before using this printer.", name, (exes + pkgs)[0]))
                KMessageBox.error(self, error_text, "Missing Driver")

            """
            if pkg and response == gtk.RESPONSE_OK:
                # Install the package.
                def wait_child (sig, stack):
                    (pid, status) = os.wait ()

                signal.signal (signal.SIGCHLD, wait_child)
                pid = os.fork ()
                if pid == 0:
                    # Child.
                    try:
                        os.execv (install, [install, pkg])
                    except:
                        pass
                    sys.exit (1)
                elif pid == -1:
                    pass # should handle error
            """

    #FIXME obsolete?
    def on_entNPTIPPQueuename_textChanged(self, ent):
        self.update_IPP_URI_label ()

    #FIXME not in gnome?
    @pyqtSignature("")
    def on_btnNPCancel_clicked(self):
        self.hide()
        
    #copy busy and ready functions from the mainapp
    #since they can't be inherited from the mainapp
    def busy (self, win = None):
        try:
            if not win:
                win = self
            win.setCursor(Qt.WaitCursor)
            KApplication.processEvents()
        except:
            nonfatalException ()

    def ready (self, win = None):
        try:
            if not win:
                win = self
            win.setCursor(Qt.ArrowCursor)
            KApplication.processEvents()
        except:
            nonfatalException ()
#end of class NewPrinterGUI

class IPPBrowseDialog(QDialog):

    def __init__(self, newprinter):
        QDialog.__init__(self, newprinter)
        self.newprinter = newprinter
        
        self.language = newprinter.language
        self.dialog_mode = ""

        self.WaitWindow = QMessageBox(self.newprinter)
        self.WaitWindow.setStandardButtons(QMessageBox.NoButton)
        
        uic.loadUi(APPDIR + "/" + "ipp-browse-dialog.ui", self)
        
        self.btnIPPBrowseOk.setEnabled(False)
        self.show()
        
        #create an empty data model
        self.ippStore = self.twIPPBrowser
        self.twIPPBrowser.sortByColumn(0, Qt.AscendingOrder)
        
        #Set tree header labels. Takes list of strings
        self.ippStore.setHeaderLabels([_("Share"), _("Comment")])

        self.browse_ipp_queues()

    def browse_ipp_queues(self):
        if not self.newprinter.ipp_lock.acquire(0):
            return
        self.browse_ipp_queues_thread()

    def browse_ipp_queues_thread(self):
        host = None
        try:
            store = self.ippStore
            store.clear ()
            items = QTreeWidgetItem(QStringList([_('Scanning...'), '']))
            store.addTopLevelItem(items)

            host = self.newprinter.entNPTIPPHostname.text()
        except:
            nonfatalException()

        oldserver = cups.getServer ()
        printers = classes = {}
        failed = False
        port = 631
        
        if host != None:
            #need to pass a unicode string. urllib falls over QStrings
            (host, port) = urllib.splitnport (unicode(host), defport=port)

        try:
            c = cups.Connection (host=host, port=port)
            printers = c.getPrinters ()
            del c
        except cups.IPPError, (e, m):
            debugprint ("IPP browser: %s" % m)
            failed = True
        except:
            nonfatalException()
            failed = True
        cups.setServer (oldserver)

        try:
            store.clear ()
            for printer, dict in printers.iteritems ():
                #only these properties seem to be used from dict so 
                #just store these instead of whole dict
                location = dict.get('printer-location', '')
                uri = dict.get('printer-uri-supported', 'ipp')
                items = QTreeWidgetItem(QStringList([printer, location, uri]))
                store.addTopLevelItem(items)

            if len (printers) + len (classes) == 0:
                # Display 'No queues' dialog
                if failed:
                    title = _("Not possible")
                    text = (_("It is not possible to obtain a list of queues "
                              "from `%s'.") % host + '\n\n' +
                            _("Obtaining a list of queues is a CUPS extension "
                              "to IPP.  Network printers do not support it."))
                else:
                    title = _("No queues")
                    text = _("There are no queues available.")

                self.hide ()
                KMessageBox.error (self.newprinter, text, title)

            try:
                self.ready(self)
            except:
                nonfatalException()

            self.newprinter.ipp_lock.release()
        except:
            nonfatalException()

        
    def on_twIPPBrowser_itemClicked(self):
        self.btnIPPBrowseOk.setEnabled(True)

    def on_btnIPPBrowseOk_clicked(self):
        index = self.twIPPBrowser.currentIndex()
        item = self.twIPPBrowser.itemFromIndex(index)
        queue = item.text(0)
        uri = unicode(item.text(2))
        self.hide()
        self.newprinter.entNPTIPPQueuename.setText (queue)
        self.newprinter.entNPTIPPQueuename.show()
        #uri = dict.get('printer-uri-supported', 'ipp')
        match = re.match ("(ipp|https?)://([^/]+)(.*)", uri)
        if match:
            self.newprinter.entNPTIPPHostname.setText (match.group (2))
            self.newprinter.entNPTIPPQueuename.setText (match.group (3))

        self.newprinter.lblIPPURI.setText (uri)
        self.newprinter.lblIPPURI.show()
        self.newprinter.setNPButtons()

    def on_btnIPPBrowseCancel_clicked(self):
        self.hide()

    def on_btnIPPBrowseRefresh_clicked(self):
        self.browse_ipp_queues()
    
    #copy busy and ready functions from the mainapp
    #since they can't be inherited from the mainapp
    def busy (self, win = None):
        try:
            if not win:
                win = self
            win.setCursor(Qt.WaitCursor)
            KApplication.processEvents()
        except:
            nonfatalException ()

    def ready (self, win = None):
        try:
            if not win:
                win = self
            win.setCursor(Qt.ArrowCursor)
            KApplication.processEvents()
        except:
            nonfatalException ()
#End of class IPPBrowseDialog

#Needed to iterate over a QTreeWidgetItem for the user access
#list
class Iter(QTreeWidgetItemIterator):
    def __init__(self, *args):
        QTreeWidgetItemIterator.__init__(self, *args)
    def next(self):
        self.__iadd__(1)
        value = self.value()
        if not value:
            raise StopIteration
        else:
            return self.value() 

def CreatePlugin(widget_parent, parent, component_data):
    u = GUI()
    KGlobal.locale().insertCatalog("system-config-printer-kde")
    kcm = u.makeui(component_data, widget_parent)
    return kcm

#if __name__ == "__main__":
    #"""start the application"""

    #appName     = "system-config-printer-kde"
    #catalogue   = "system-config-printer-kde"
    #programName = ki18n("System Config Printer KDE")
    #version     = "1.0"
    #description = ki18n("Printer configuration tool")
    #license     = KAboutData.License_GPL
    #copyright   = ki18n("2007 Tim Waugh, Red Hat Inc, 2007-2008 Canonical Ltd")
    #text        = KLocalizedString()
    #homePage    = "https://launchpad.net/system-config-printer"
    #bugEmail    = ""

    #aboutData   = KAboutData (appName, catalogue, programName, version, description,
                                #license, copyright, text, homePage, bugEmail)

    #aboutData.addAuthor(ki18n("Jonathan Riddell"), ki18n("Author"))
    #aboutData.addAuthor(ki18n("Tim Waugh/Red Hat"), ki18n("System Config Printer Author"))

    #options = KCmdLineOptions()

    #KCmdLineArgs.init(sys.argv, aboutData)
    #KCmdLineArgs.addCmdLineOptions(options)

    #app = KApplication()
    #args = KCmdLineArgs.parsedArgs()

    #applet = GUI()
    #sys.exit(app.exec_())
