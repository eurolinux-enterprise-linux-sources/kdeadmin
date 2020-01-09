/***************************************************************************
                          kadddevicecontainer.h  -  description
                             -------------------
    begin                : Wed Jun 15 00:40:33 UTC 2005
    copyright            : (C) 2005 by Juan Luis Baptiste
    email                : juan.baptiste@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KADDDEVICECONTAINER_H
#define KADDDEVICECONTAINER_H

#include <QCheckBox>
#include <q3groupbox.h>
#include <qradiobutton.h>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <kcombobox.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>

#include "ui_kadddevicedlg.h"
#include "ui_kadddevicedlgextension.h"
#include "ui_kadddevicewifiext.h"
#include "kaddressvalidator.h"


class KAddDeviceDlgExtension : public QWidget, public Ui::KAddDeviceDlgExtension
{
  Q_OBJECT
public:
  KAddDeviceDlgExtension( QWidget *parent );
protected slots:
  void valueChanged( const QString &s );
signals:
  void valueChangedSignal(int);
};

class KAddDeviceWifiExt : public QWidget, public Ui::KAddDeviceWifiExt
{
public:
  KAddDeviceWifiExt( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};

class KAddDeviceDlg : public QWidget, public Ui::KAddDeviceDlg
{
public:
  KAddDeviceDlg( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};

/**
Network interface configuration dialog. This dialog contains the KAddDeviceDlg and KAddDeviceWifiExt widgets.

@author Juan Luis Baptiste
*/
class KAddDeviceContainer : public KDialog
{
  Q_OBJECT
  public:
    explicit KAddDeviceContainer(QWidget *parent = 0, const char *name = 0);

    ~KAddDeviceContainer();
    KPushButton* kpbAdvanced;
    KPushButton* kpbApply;
    KPushButton* kpbCancel;
    void addButtons();
    KAddDeviceDlg *addDlg;
    KAddDeviceWifiExt *extDlg;
    void addWirelessWidget();
    bool modified() const;
    bool advanced() const;

  private:
    void makeButtonsResizeable();
  
  protected:
    QVBoxLayout* mainLayout;    
    QHBoxLayout* buttonsLayout;
    QSpacerItem* buttonsSpacer;
    QSpacerItem* widgetHSpacer;    
    bool _modified;
    bool _advanced;    
  
  protected slots:
    void toggleApplyButtonSlot( const QString & );
    void toggleApplyButtonSlot( int );
    void toggleAdvancedOptionsSlot(bool enabled );
    void verifyDeviceInfoSlot();
    void advancedOptionsSlot();
    void cancelSlot();    
};

#endif
