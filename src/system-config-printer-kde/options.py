# -*- coding: utf-8 -*-
## system-config-printer

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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

"""
These classes are used in the Job Options tab of the printer config dialogue
"""

def OptionWidget(name, v, s, on_change):
    if isinstance(v, list):
        # XXX
        if isinstance(s, list):
            for vv in v + s:
                if not isinstance(vv, str): raise ValueError
            return OptionSelectMany(name, v, s, on_change)
        print v, s
        raise NotImplemented
    else:
        if (isinstance(s, int) or
            isinstance(s, float) or
            (isinstance(s, tuple) and
             len(s) == 2 and
             ((isinstance(s[0], int) and isinstance(s[1], int)) or
              (isinstance(s[0], float) and isinstance(s[1], float))))):
            try:
                if (isinstance(s, int) or
                    isinstance(s, tuple) and isinstance(s[0], int)):
                    v = int(v)
                else:
                    v = float(v)
            except ValueError:
                return OptionText(name, v, "", on_change)
            return OptionNumeric(name, v, s, on_change)
        elif isinstance(s, list):
            for sv in s:
                if not isinstance(sv, int):
                    return OptionSelectOne(name, v, s, on_change)
            try:
                v = int(v)
            except ValueError:
                return OptionSelectOne(name, v, s, on_change)
            return OptionSelectOneNumber(name, v, s, on_change)
        elif isinstance(s, str):
            return OptionText(name, v, s, on_change)
        else:
            raise ValueError

# ---------------------------------------------------------------------------

class OptionInterface:
    def get_current_value(self):
        raise NotImplemented

    def is_changed(self):
        raise NotImplemented

class OptionAlwaysShown(OptionInterface):
    # States
    STATE_UNCHANGED=0
    STATE_RESET=1
    STATE_ADJUSTED=2

    def __init__(self, name, ipp_type, system_default,
                 widget, button, combobox_map = None, use_supported = False):
        self.name = name
        self.widget = widget
        self.button = button
        if ipp_type == bool:
            def bool_type (x):
                if type (x) == str:
                    if x.lower () in ("false", "no", "off"):
                        return False
                    # Even the empty string is true.
                    return True
                return bool (x)
            ipp_type = bool_type
        self.ipp_type = ipp_type
        self.set_default (system_default)
        self.combobox_map = combobox_map
        if combobox_map != None and ipp_type == int:
            i = 0
            dict = {}
            while i < self.widget.count():
                dict[combobox_map[i]] = self.widget.itemText(i)
                i += 1
            self.combobox_dict = dict
        self.use_supported = use_supported
        self.reinit (None)

    def set_default(self, system_default):
        # For the media option, the system default depends on the printer's
        # PageSize setting.  This method allows the main module to tell us
        # what that is.
        self.system_default = self.ipp_type (system_default)

    def reinit(self, original_value, supported=None):
        """Set the original value of the option and the supported choices.
        The special value None for original_value resets the option to the
        system default."""
        if (supported != None and
            self.use_supported):
            if (type(self.widget) == QComboBox and
                self.ipp_type == str):
                self.widget.clear()
                for each in supported:
                    self.widget.addItem(each)
            elif (type(self.widget) == QComboBox and
                  self.ipp_type == int and
                  self.combobox_map != None):
                self.widget.clear()
                for each in supported:
                    self.widget.addItem(self.combobox_dict[each])
        if original_value != None:
            self.original_value = self.ipp_type (original_value)
            self.set_widget_value (self.original_value)
            if original_value != self.get_widget_value():
                self.button.setEnabled(True)
        else:
            self.original_value = None
            self.set_widget_value (self.system_default)
            self.button.setEnabled(False)
        self.state = self.STATE_UNCHANGED

    def set_widget_value(self, ipp_value):
        t = type(self.widget)
        if t == QSpinBox:
            return self.widget.setValue(ipp_value)
        elif t == QComboBox:
            if self.ipp_type == str and self.combobox_map == None:
                index = self.widget.findText(ipp_value)
                if index != -1:
                    self.widget.setCurrentIndex(index)
            else:
                # It's an int.
                if self.combobox_map:
                    index = self.combobox_map.index (ipp_value)
                else:
                    index = ipp_value
                return self.widget.setCurrentIndex(index)
        elif t == QCheckBox:
            return self.widget.setChecked(ipp_value)
        else:
            raise NotImplemented

    def get_widget_value(self):
        t = type(self.widget)
        if t == QSpinBox:
            # Ideally we would use self.widget.get_value() here, but
            # it doesn't work if the value has been typed in and then
            # the Apply button immediately clicked.  To handle this,
            # we use self.widget.get_text() and fall back to
            # get_value() if the result cannot be interpreted as the
            # type we expect.
            try:
                return self.ipp_type (self.widget.value ())
            except ValueError:
                # Can't convert result of get_text() to ipp_type.
                return self.ipp_type (self.widget.value ())
        elif t == QComboBox:
            if self.combobox_map:
                return self.combobox_map[self.widget.currentIndex()]
            if self.ipp_type == str:
                return self.widget.currentText()
            return self.ipp_type (unicode(self.widget.currentText()))
        elif t == QCheckBox:
            return self.ipp_type (self.widget.isChecked())

        raise NotImplemented

    def get_current_value(self):
        return self.get_widget_value ()

    def is_changed(self):
        if self.original_value != None:
            # There was a value set previously.
            if self.state == self.STATE_RESET:
                # It's been removed.
                return True
            if self.state == self.STATE_ADJUSTED:
                if self.get_current_value () != self.original_value:
                    return True
                return False

            # The value is the same as before, and not reset.
            return False

        # There was no original value set.
        if self.state == self.STATE_ADJUSTED:
            # It's been adjusted.
            return True

        # It's been left alone, or possible adjusted and then reset.
        return False

    def reset(self):
        self.set_widget_value (self.system_default)
        self.state = self.STATE_RESET
        self.button.setEnabled(False)

    def changed(self):
        self.state = self.STATE_ADJUSTED
        self.button.setEnabled(True)

class OptionAlwaysShownSpecial(OptionAlwaysShown):
    def __init__(self, name, ipp_type, system_default,
                 widget, button, combobox_map = None, use_supported = False,
                 special_choice = "System default"):
        self.special_choice = special_choice
        self.special_choice_shown = False
        OptionAlwaysShown.__init__ (self, name, ipp_type, system_default,
                                    widget, button,
                                    combobox_map=combobox_map,
                                    use_supported=use_supported)

    def show_special_choice (self):
        if self.special_choice_shown:
            return

        self.special_choice_shown = True
        # Only works for ComboBox widgets
        self.widget.insertItem(0, self.special_choice)
        self.widget.setCurrentIndex(0)
        ##model = self.widget.get_model ()
        ##iter = model.insert (0)
        ##model.set_value (iter, 0, self.special_choice)
        ##self.widget.set_active_iter (model.get_iter_first ())

    def hide_special_choice (self):
        if not self.special_choice_shown:
            return

        self.special_choice_shown = False
        # Only works for ComboBox widgets
        self.widget.removeItem(0)

    def reinit(self, original_value, supported=None):
        if original_value != None:
            self.hide_special_choice ()
        else:
            self.show_special_choice ()

        OptionAlwaysShown.reinit (self, original_value, supported=supported)

    def reset(self):
        self.show_special_choice ()
        OptionAlwaysShown.reset (self)

    def changed(self):
        OptionAlwaysShown.changed (self)
        if self.widget.currentIndex() > 0:
            self.hide_special_choice ()

class Option(OptionInterface):

    conflicts = None

    def __init__(self, name, value, supported, on_change):
        self.name = name
        self.value = value
        self.supported = supported
        self.on_change = on_change
        self.is_new = False

        label = unicode(name)
        if not label.endswith (':'):
            label += ':'
        self.label = QLabel(label)

    def get_current_value(self):
        raise NotImplemented

    def is_changed(self):
        return (self.is_new or
                str (self.get_current_value()) != str (self.value))

    def changed(self, widget, *args):
        self.on_change(self)
    
# ---------------------------------------------------------------------------

class OptionSelectOne(Option):

    def __init__(self, name, value, supported, on_change):
        Option.__init__(self, name, value, supported, on_change)

        self.selector = gtk.combo_box_new_text()
        
        
        selected = None
        for nr, choice in enumerate(supported):
            self.selector.append_text(str(choice))
            if str (value) == str (choice):
                selected = nr
        if selected is not None:
            self.selector.set_active(selected)
        else:
            print "Unknown value for %s: %s" % (name, value)
            print "Choices:", supported
        self.selector.connect("changed", self.changed)

    def get_current_value(self):
        return self.selector.get_active_text()

# ---------------------------------------------------------------------------

class OptionSelectOneNumber(OptionSelectOne):

    def get_current_value(self):
        return int(self.selector.get_active_text())

# ---------------------------------------------------------------------------

class OptionSelectMany(Option):

    def __init__(self, name, value, supported, on_change):
        Option.__init__(self, name, value, supported, on_change)
        self.checkboxes = []
        vbox = gtk.VBox()

        for s in supported:
            checkbox = gtk.CheckButton(label=s)
            checkbox.set_active(s in value)
            vbox.add(checkbox)
            checkbox.connect("toggled", self.changed)
            self.checkboxes.append(checkbox)
        self.selector = vbox
            
    def get_current_value(self):
        return[s for s, chk in zip(self.supported, self.checkboxes)
               if chk.get_active()]

# ---------------------------------------------------------------------------

class OptionNumeric(Option):
    def __init__(self, name, value, supported, on_change):
        self.is_float = (isinstance(supported, float) or
                         (isinstance(supported, tuple) and
                          isinstance(supported[0], float)))
        if self.is_float:
            digits = 2
        else:
            digits = 0

        if not isinstance(supported, tuple):
            supported = (0, supported)
        Option.__init__(self, name, value, supported, on_change)
        adj = gtk.Adjustment(value, supported[0], supported[1], 1.0, 5.0, 0.0)
        self.selector = gtk.SpinButton(adj, climb_rate=1.0, digits=digits)
        if not self.is_float:
            self.selector.set_numeric(True)
        self.selector.connect("changed", self.changed)

    def get_current_value(self):
        if self.is_float:
            return self.selector.get_value()
        return self.selector.get_value_as_int()

# ---------------------------------------------------------------------------

class OptionText(Option):
    def __init__(self, name, value, supported, on_change):
        Option.__init__(self, name, value, supported, on_change)

        self.selector = QLineEdit()
        self.selector.setText(value)
        self.selector.connect(self.selector, SIGNAL("textChanged(QString)"), self.changed)

    def get_current_value(self):
        return self.selector.text()
