
/***************************************************************************
                          kadddevicecontainer.cpp  -  description
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
#include "kadddevicecontainer.h"
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>


KAddDeviceDlgExtension::KAddDeviceDlgExtension( QWidget *parent )
  : QWidget( parent ) {
   setupUi( this );
   connect(kleBroadcast,SIGNAL(textChanged(QString)), this,SLOT(valueChanged(QString)));
   connect(kleDescription,SIGNAL(textChanged(QString)), this,SLOT(valueChanged(QString)));
}

void KAddDeviceDlgExtension::valueChanged( const QString & )
{
  emit valueChangedSignal(0);
}

KAddDeviceContainer::KAddDeviceContainer(QWidget *parent, const char *name)
 : KDialog(parent)
{
    _modified = false;   
    _advanced = false;
 
     //Setup extension dialog
    setExtension( new KAddDeviceDlgExtension( this ) );
    setOrientation( Qt::Vertical );
   
    //Create dialog that contains the KAddDeviceDlg and KAddDeviceWifiExt (when 
    //configuring a wireless interface) widgets 
    QPixmap activeEthernetDeviceImg(BarIcon("network_connected_lan_knc"));
    setIcon(activeEthernetDeviceImg);
    //First create a main QHBoxLayout
    mainLayout = new QVBoxLayout( this, 10, 2, "mainLayout"); 
    
    //Create the Addvanced settings, Ok and Cancel buttons and add them to a QHBoxLayout
    buttonsLayout = new QHBoxLayout( 0, 0, 4, "buttonsLayout"); 
    kpbAdvanced = new KPushButton( this );
    kpbAdvanced->setObjectName( "kpbAdvanced" );
    buttonsLayout->addWidget( kpbAdvanced );
    buttonsSpacer = new QSpacerItem( 70, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    buttonsLayout->addItem( buttonsSpacer );

    kpbApply = new KPushButton( this );
    kpbApply->setObjectName( "kpbApply" );
    kpbApply->setEnabled( false );
    kpbApply->setDefault( true );
    buttonsLayout->addWidget( kpbApply );

    kpbCancel = new KPushButton( this );
    kpbCancel->setObjectName( "kpbCancel" );
    buttonsLayout->addWidget( kpbCancel );
            
    setCaption( QString::null );	//krazy:exclude=nullstrassign for old broken gcc
    kpbAdvanced->setText( i18n( "&Advanced Settings" ) );
    kpbAdvanced->setToolTip( i18n( "Toggle between advanced and basic settings" ) );
    kpbAdvanced->setWhatsThis( i18n( "Toggle between advanced and basic settings" ) );
    kpbApply->setText( i18n( "&OK" ) );
    kpbApply->setToolTip( i18n( "Apply changes" ) );
    kpbCancel->setText( i18n( "&Cancel" ) );
    kpbCancel->setToolTip( i18n( "Forget changes" ) );

    //Creat and add the KAddDeviceDlg widget to the main layout
    addDlg = new KAddDeviceDlg(this);
    mainLayout->addWidget(addDlg);
    mainLayout->setResizeMode(QLayout::Auto);
    
    //Setup connections
    connect((KAddDeviceDlgExtension*)extension(),SIGNAL(valueChangedSignal(int)),this,SLOT(toggleApplyButtonSlot(int)));
    connect(addDlg->kleIPAddress,SIGNAL(textChanged(const QString&)),this,SLOT(toggleApplyButtonSlot(const QString&)));
    connect(addDlg->kcbNetmask,SIGNAL(activated(int)),this,SLOT(toggleApplyButtonSlot(int)));
    connect(addDlg->kcbAutoBootProto,SIGNAL(activated(const QString&)),this,SLOT(toggleApplyButtonSlot(const QString&)));
    connect(addDlg->kcbstartAtBoot,SIGNAL(stateChanged(int)),this,SLOT(toggleApplyButtonSlot(int)));
    connect(addDlg->rbBootProtoAuto,SIGNAL(toggled(bool)),this,SLOT(toggleAdvancedOptionsSlot(bool)));
    connect(addDlg->rbBootProtoAuto,SIGNAL(toggled(bool)),kpbAdvanced,SLOT(setDisabled(bool)));
    connect(addDlg->rbBootProtoAuto,SIGNAL(stateChanged(int)),this,SLOT(toggleApplyButtonSlot(int)));
   // connect(addDlg->rbBootProtoAuto,SIGNAL(toggled(bool)),addDlg->kleIPAddress,SLOT(setDisabled(bool)));
   // connect(addDlg->rbBootProtoAuto,SIGNAL(toggled(bool)),addDlg->kcbNetmask,SLOT(setDisabled(bool)));        
   // connect(addDlg->rbBootProtoManual,SIGNAL(toggled(bool)),addDlg->kcbAutoBootProto,SLOT(setDisabled(bool)));
    connect(kpbCancel,SIGNAL(clicked()),this,SLOT(cancelSlot()));
    connect(kpbApply,SIGNAL(clicked()),this,SLOT(verifyDeviceInfoSlot()));
    connect(kpbAdvanced,SIGNAL(clicked()),this,SLOT(advancedOptionsSlot()));
    
}

void KAddDeviceContainer::addWirelessWidget(){
  extDlg = new KAddDeviceWifiExt(this);
  mainLayout->addWidget( extDlg );
  connect(extDlg->kleEssid,SIGNAL(textChanged(const QString&)),this,SLOT(toggleApplyButtonSlot(const QString&)));
  connect(extDlg->kleWepKey,SIGNAL(textChanged(const QString&)),this,SLOT(toggleApplyButtonSlot(const QString&)));
}
void KAddDeviceContainer::toggleApplyButtonSlot( const QString & )
{
  toggleApplyButtonSlot(0);
}

void KAddDeviceContainer::toggleApplyButtonSlot( int )
{
  _modified = true;
   kpbApply->setEnabled(true);
}

void KAddDeviceContainer::toggleAdvancedOptionsSlot(bool enabled )
{
   KAddDeviceDlgExtension *advancedOptions = (KAddDeviceDlgExtension *)this->extension();

  if (enabled)
    advancedOptions->gbAdvancedDeviceInfo->setEnabled(false);
  else
    advancedOptions->gbAdvancedDeviceInfo->setEnabled(true);
  
  _modified = true;
  kpbApply->setEnabled(true);
}

void KAddDeviceContainer::verifyDeviceInfoSlot()
{
  KAddDeviceDlgExtension *advancedOptions = (KAddDeviceDlgExtension *)this->extension();
  
  if (addDlg->rbBootProtoManual->isChecked())
  {	
    if (!KAddressValidator::isValidIPAddress(addDlg->kleIPAddress->text()))
    {
      KMessageBox::error(this,i18n("The format of the specified IP address is not valid."),i18n("Invalid IP Address "));
    }    
    else if (_advanced && !KAddressValidator::isNetmaskValid(addDlg->kcbNetmask->currentText()))
        KMessageBox::error(this,i18n("The format of the specified netmask is not valid."),i18n("Invalid IP Address "));
        
    else if (_advanced && !KAddressValidator::isBroadcastValid(advancedOptions->kleBroadcast->text()))
        KMessageBox::error(this,i18n("The format of the specified broadcast is not valid."),i18n("Invalid IP Address"));
        
    else
    {
      _modified = true;
      close();
    }
  }	
  else if (addDlg->rbBootProtoAuto->isChecked())
  {
    _modified = true;
    close();
  }    
}

void KAddDeviceContainer::makeButtonsResizeable()
{
#ifdef __GNUC__
#warning "KDE4 porting FIXME"	
#endif	
  //kpbApply->setAutoResize(true);
  //kpbAdvanced->setAutoResize(true);
  //kpbCancel->setAutoResize(true);
}

bool KAddDeviceContainer::modified() const
{
  return _modified;
}
bool KAddDeviceContainer::advanced() const
{
  return _advanced;
}


void KAddDeviceContainer::advancedOptionsSlot()
{
    if (!_advanced)
    {
      kpbAdvanced->setText(i18n("Basic Settings"));
      addDlg->kcbNetmask->setEditable(true);	
    }
    else
    {
      kpbAdvanced->setText(i18n("Advanced Settings"));
      addDlg->kcbNetmask->setEditable(false);      
    }
    _advanced = !_advanced;
    showExtension(_advanced );  
}


void KAddDeviceContainer::cancelSlot()
{
 _modified = false;
 close();
}


KAddDeviceContainer::~KAddDeviceContainer()
{
}

void KAddDeviceContainer::addButtons()
{
  widgetHSpacer = new QSpacerItem( 20, 16, QSizePolicy::Minimum, QSizePolicy::Expanding );
  mainLayout->addItem( widgetHSpacer );  
  mainLayout->addLayout( buttonsLayout );
}

#include "kadddevicecontainer.moc"
