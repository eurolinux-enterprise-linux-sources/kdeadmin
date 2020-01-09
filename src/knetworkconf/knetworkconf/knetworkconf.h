/***************************************************************************
                          knetworkconf.h  -  description
                             -------------------
    begin                : Sun Jan 12 00:54:19 UTC 2003
    copyright            : (C) 2003 by Juan Luis Baptiste
    email                : jbaptiste@merlinux.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNETWORKCONF_H
#define KNETWORKCONF_H

#define DEVICE_UP 0
#define DEVICE_DOWN 1

#include <unistd.h>
#include <sys/types.h>

#include <QWidget>
#include <qpixmap.h>
#include <q3process.h>
#include <q3ptrlist.h>
#include <qstringlist.h>

#include <QCheckBox>
#include <q3groupbox.h>
#include <qradiobutton.h>
#include <kdialog.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <k3listview.h>
#include <klineedit.h>
#include <k3listbox.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstandarddirs.h>


#include "ui_knetworkconfdlg.h"
#include "ui_kadddnsserverdlg.h"
#include "kadddevicecontainer.h"
#include "knetworkinfo.h"
#include "kroutinginfo.h"
#include "knetworkconfigparser.h"
#include "kdnsinfo.h"
#include "ui_kaddknownhostdlg.h"
#include "kknownhostinfo.h"
#include "knetworkinterface.h"


class KAddDNSServerDlg : public QWidget, public Ui::KAddDNSServerDlg
{
    Q_OBJECT
public:
  KAddDNSServerDlg( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
    init();
  }
  bool modified() const;
  void setAddingAlias(bool add);
private slots:
    void validateAddressSlot();

protected:
    void init();
    void makeButtonsResizeable();
private:
    bool _modified2;
    bool addingAlias;

};

class KNetworkConfDlg : public QWidget, public Ui::KNetworkConfDlg
{
public:
  KNetworkConfDlg( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};

class KAddKnownHostDlg : public QWidget, public Ui::KAddKnownHostDlg
{
  Q_OBJECT
public:
  KAddKnownHostDlg( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
    init();
  }
    bool modified() const;
protected:
    void init();
private slots:
    void validateAddressSlot();
    void makeButtonsResizeable();
    void editHostSlot();
    void removeHostSlot();
    void addHostSlot();
private:
    bool _modifiedhost;
};

/** KNetworkConf is the base class of the project */
class KNetworkConf : public KNetworkConfDlg
{
  Q_OBJECT
  public:
    /** construtor */
    explicit KNetworkConf(QWidget* parent=0, const char *name=0);
    /** destructor */
    ~KNetworkConf();
    /** Puts the application in read-only mode. This happens when the user runing the application isn't root. */
    void setReadOnly(bool state);
    void setVersion(QString ver);
    QString getVersion();
    /** Disables all buttons a line edit widgets when the user has read only access. */
    void disableAll();

    /**
      Fill the Listview with the info of the network interfaces.
    */
    void loadNetworkDevicesInfo();
    void loadRoutingInfo();
    void loadDNSInfo();
    void loadNetworkProfiles();

  private: // Private attributes
    /**  */
    KNetworkConfigParser *config;
    KNetworkInterface * getDeviceInfo(QString device);
    QString getDeviceName(QString ipAddr);
  /** Creates a QStringList with the IP addresses contained in the QListBox of name servers. */
    QStringList getNamserversList(K3ListBox * serverList);
/** Creates a QPtrList<KKownHostInfo> with the info contained in the K3ListView of name servers. */
    Q3PtrList<KKnownHostInfo> getKnownHostsList(K3ListView * hostsList);
    QString currentDevice;
    KRoutingInfo *routingInfo;
    KDNSInfo *dnsInfo;
    bool reloaded;
    QString commandOutput;
    Q3Process *procUpdateDevice;
    Q3Process *procDeviceState;
    QStringList deviceNamesList;
    bool devicesModified;
    bool readOnly;
    Q3PtrList<KKnownHostInfo> knownHostsList;
    Q3PtrList<KNetworkInfo> profilesList;
    bool nameServersModified;
    /** The program's version. */
    QString version;
    bool modified;
    bool devStateChanged;
    /**  */
    KNetworkInfo * netInfo;
    /** Has the errors throwed by GST when executed. */
    QString commandErrOutput;
    /** Changes the state of device 'dev' to DEVICE_UP or DEVICE_DOWN.
    Return true on success, false on failure.  */
    void changeDeviceState(const QString &dev, int state);
    KNetworkInfo *getProfile(Q3PtrList<KNetworkInfo> profilesList, QString selectedProfile);
    void showSelectedProfile(QString profile);

  public slots:
      virtual void saveInfoSlot();
/** Puts the application in read-only mode. This happens when the user runing the application isn't root. */
    void setReadOnlySlot(bool state);

  private slots:
    /** Enables the configure and remove buttons. */
    virtual void enableButtonsSlot();
    /** opens the add server dialog. */
    virtual void addServerSlot();
    /** opens the edit server dialog. */
    virtual void editServerSlot();
    /** Terminates the application*/
    virtual void quitSlot();
    virtual void readFromStdout();
    virtual void readFromStdoutUpDown();
    virtual void enableInterfaceSlot();
    virtual void disableInterfaceSlot();
   /** Pops up the window for adding a new interface. */
    virtual void configureDeviceSlot();
    void enableApplyButtonSlot();
    /** Shows the help browser. Hopefully some day it will be one :-). */
    virtual void helpSlot();
    virtual void enableApplyButtonSlot(bool);
    virtual void enableApplyButtonSlot(const QString &text);
    /** Saves all the modified info of devices, routes,etc. */
    virtual void moveDownServerSlot();
    virtual void moveUpServerSlot();
    virtual void removeServerSlot();
    bool valuesChanged(KNetworkInterface *dev,
                        QString bootProto,
                        QString netmask,
                        QString ipAddr,
                        bool onBoot,
                        QString desc,
			QString broadcast);
    /** Returns a list of strings of all the configured devices. */
    QStringList getDeviceList();
    /** Sets the QPushButton::autoResize() in true for all buttons. */
    void makeButtonsResizeable();

    /** Adds a new host to the K3ListView that has the known hosts. */
    void addKnownHostSlot();
    void aboutSlot();
    /** Edits the info about a known host. */
    void editKnownHostSlot();
    /** Removes a known host from the list view */
    void removeKnownHostSlot();
    /** No descriptions */
    void readFromStdErrUpDown();
    void getNetworkInfoSlot();
    /** Shows the main window after the network info has been loaded. */
    void showMainWindow();
    void verifyDeviceStateChanged();
    /** Sees if a device is active or not in the ifconfig output. Not very nice, but it works. Inthe future, this has to be managed by gst. */
    bool isDeviceActive(const QString &device, const QString &ifconfigOutput);

    /*Shows a context menu when right-clicking in the interface list*/
    void showInterfaceContextMenuSlot(K3ListView*, Q3ListViewItem*, const QPoint&);

    /** Enable some signals in the GUI that need to be enabled *after* the loading of the network info is done.*/
    void enableSignals ();
    virtual void enableProfileSlot();

  signals:
    //Signal used to tell kcontrol that the network configuration has been changed.
    void networkStateChanged(bool);

};

#endif
