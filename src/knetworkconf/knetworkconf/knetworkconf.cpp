/***************************************************************************
                          knetworkconf.cpp  -  description
                             -------------------
    begin                : Sun Jan 12 8:54:19 UTC 2003
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

#include "knetworkconf.h"

#include <kiconloader.h>

//Added by qt3to4:
#include <QPixmap>
#include <Q3PtrList>
#include <ktoolinvocation.h>
#include <kaboutapplicationdialog.h>
#include <KComponentData>


void KAddDNSServerDlg::setAddingAlias(bool add)
{
  addingAlias = add;
}

bool KAddDNSServerDlg::modified() const
{
   return _modified2;
}
void KAddDNSServerDlg::init()
{
   _modified2 = false;
   addingAlias = false;
   makeButtonsResizeable();
   connect( kpbAddServer, SIGNAL( clicked() ), this, SLOT( validateAddressSlot() ) );
}

void KAddDNSServerDlg::validateAddressSlot()
{
  if (addingAlias && kleNewServer->text() !="")
  {
     _modified2 = true;
     close();
  }
  else
  {
    KMessageBox::error(this,i18n("You have to type an alias first."),i18n("Invalid Text"));
  }
  if (!addingAlias)
  {
    if (KAddressValidator::isValidIPAddress(kleNewServer->text()))
    {
       _modified2 = true;
      close();
    }
    else
    {
      KMessageBox::error(this,i18n("The format of the specified IP address is not valid."),i18n("Invalid IP Address"));
    }
  }
}


void KAddDNSServerDlg::makeButtonsResizeable()
{
#ifdef __GNUC__
#warning "KDE4 fix me "
#endif
  //kpbAddServer->setAutoResize(true);
  //kpbCancel->setAutoResize(true);
}


void KAddKnownHostDlg::init()
{
    connect( kpbCancel, SIGNAL( clicked() ), this,SLOT( close() ) );
    connect( kpbAdd, SIGNAL( clicked() ), this, SLOT( validateAddressSlot() ) );
    connect( klbAliases, SIGNAL(doubleClicked(Q3ListBoxItem*) ),this,SLOT(editHostSlot() ) );
    connect( kpbAddHost, SIGNAL( clicked() ), this, SLOT(addHostSlot() ) );
    connect( kpbEditHost, SIGNAL( clicked() ) , this, SLOT(editHostSlot() ) );
    connect( kpbRemoveHost, SIGNAL( clicked() ) , this, SLOT(removeHostSlot() ) );
    _modifiedhost = false;
    makeButtonsResizeable();
}

void KAddKnownHostDlg::validateAddressSlot()
{
  if (!KAddressValidator::isValidIPAddress(kleIpAddress->text()))
  {
       KMessageBox::error(this,i18n("The format of the specified IP address is not valid."),i18n("Invalid IP Address"));
  }
  else if ((klbAliases->firstItem() == 0))
  {
      KMessageBox::error(this,i18n("You must add at least one alias for the specified IP address."),i18n("Insufficient Aliases"));
  }
  else
  {
    _modifiedhost = true;
    close();
  }

}


void KAddKnownHostDlg::makeButtonsResizeable()
{
/*    kpbAdd->setAutoResize(true);
    kpbCancel->setAutoResize(true);
    kpbAddHost->setAutoResize(true);
    kpbEditHost->setAutoResize(true);
    kpbRemoveHost->setAutoResize(true);
*/
}


bool KAddKnownHostDlg::modified() const
{
    return _modifiedhost;
}


void KAddKnownHostDlg::editHostSlot()
{
  KDialog *dialog = new KDialog(this);

  KAddDNSServerDlg *dlg = new KAddDNSServerDlg(this);
  dialog->setMainWidget(dlg);
  if (klbAliases->currentItem() >= 0)
  {
    int currentPos = klbAliases->currentItem();
    dlg->setCaption(i18n("Edit Alias"));
    dlg->lIPAddress->setText(i18n("Alias:"));
    dlg->setAddingAlias(true);
    Q3ListBoxItem *item = klbAliases->item(currentPos);
    QString currentText = item->text();
    dlg->kleNewServer->setText(currentText);
    dlg->kpbAddServer->setText(i18n("&OK"));
    dialog->exec();

    if(dlg->modified())
    {
      klbAliases->changeItem(dlg->kleNewServer->text(),currentPos);
//      enableApplyButtonSlot();
    }
  }
}


void KAddKnownHostDlg::removeHostSlot()
{
  if (klbAliases->currentItem() >= 0)
    klbAliases->removeItem(klbAliases->currentItem());
}


void KAddKnownHostDlg::addHostSlot()
{
  KDialog * dialog = new KDialog(this);
  KAddDNSServerDlg *addDlg = new KAddDNSServerDlg(this);
  dialog->setMainWidget(addDlg);
  addDlg->setCaption(i18n("Add New Alias"));
  addDlg->lIPAddress->setText(i18n("Alias:"));
  addDlg->setAddingAlias(true);

  dialog->exec();
  if(addDlg->modified())
  {
    klbAliases->insertItem(addDlg->kleNewServer->text());
//    enableApplyButtonSlot();
  }
}

KNetworkConf::KNetworkConf(QWidget *parent, const char *name) : KNetworkConfDlg(parent)
{
  netInfo = 0L;
  makeButtonsResizeable();
  config = new KNetworkConfigParser();
  klvCardList->setAllColumnsShowFocus(true);
  klvKnownHosts->setAllColumnsShowFocus(true);
  klvProfilesList->setAllColumnsShowFocus(true);
  klvProfilesList->setRenameable(0,true);
  klvProfilesList->setRenameable(1,true);

  //Connect signals emmitted by the backend to know when data is ready to be painted.
  connect(config,SIGNAL(readyLoadingNetworkInfo()),this,SLOT(getNetworkInfoSlot()));
  connect(config,SIGNAL(readyLoadingNetworkInfo()),this,SLOT(showMainWindow()));
  connect(config,SIGNAL(readyLoadingNetworkInfo()),this,SLOT(enableSignals()));
  connect(config, SIGNAL(setReadOnly(bool)),this,SLOT(setReadOnlySlot(bool)));
  connect(klvCardList,
          SIGNAL(contextMenu(K3ListView*,Q3ListViewItem*,const QPoint&)),
          this,
          SLOT(showInterfaceContextMenuSlot(K3ListView*,Q3ListViewItem*, const QPoint&)));
  connect( kcbGwDevice, SIGNAL(activated(QString) ),this,SLOT(enableApplyButtonSlot(QString) ) );
  connect( klvKnownHosts, SIGNAL(doubleClicked(Q3ListViewItem*) ),this,SLOT( editKnownHostSlot() ) );
  connect( kpbAddDomainServer, SIGNAL( clicked() ), this, SLOT(addServerSlot() ) );
  connect( kpbAddKnownHost, SIGNAL( clicked() ), this, SLOT(addKnownHostSlot() ) );
  connect( kpbDownDomainServerList, SIGNAL( clicked() ), this, SLOT(moveDownServerSlot() ) );
  connect( kpbEditKnownHost, SIGNAL( clicked() ), this, SLOT(editKnownHostSlot() ) );
  connect(kpbRemoveDomainServer,SIGNAL( clicked() ),this,SLOT( removeServerSlot() ) );
  connect( kpbRemoveKnownHost,SIGNAL( clicked() ),this,SLOT( removeKnownHostSlot() ) );
  connect( kpbUpDomainServerList,SIGNAL( clicked() ),this,SLOT(moveUpServerSlot() ) );
  connect( kpbEditDomainServer,SIGNAL( clicked() ),this,SLOT(editServerSlot() ) );
  connect( klbDomainServerList, SIGNAL(doubleClicked(Q3ListBoxItem* ) ),this,SLOT(  editServerSlot() ) );
  connect(klvCardList,SIGNAL( selectionChanged() ),this,SLOT(enableButtonsSlot() ) );
  connect( klvCardList, SIGNAL( doubleClicked(Q3ListViewItem*) ), this, SLOT(configureDeviceSlot() ) );
/*
  connect( klvCardList, SIGNAL( rightButtonClicked(Q3ListViewItem*,QPoint,int) ), this, SLOT( showInterfaceContextMenuSlot() ) );
*/
  connect( kpbConfigureNetworkInterface, SIGNAL( clicked() ), this, SLOT( configureDeviceSlot() ) );
  connect( kpbUpButton,SIGNAL( clicked() ), this, SLOT( enableInterfaceSlot() ) );
  connect( kpbDownButton,SIGNAL( clicked() ), this, SLOT(disableInterfaceSlot() ) );
  connect( kpbEnableProfile,SIGNAL( clicked() ), this, SLOT(enableProfileSlot() ) );
  //Temporarly added while gst supports routing option.
//  cbEnableRouting->hide();
}

KNetworkConf::~KNetworkConf()
{
	delete config;
}

void KNetworkConf::getNetworkInfoSlot()
{
  netInfo = config->getNetworkInfo();
  if (netInfo == NULL)
  {
    KMessageBox::error(this,
                        i18n("Could not load network info."),
                        i18n("Error Reading Configuration File"));
    //qApp->quit();
    //exit(1);
  }
  else
  {
    //QPtrList<KNetworkInterface> deviceList;
    routingInfo = netInfo->getRoutingInfo();
    dnsInfo = netInfo->getDNSInfo();
    profilesList = netInfo->getProfilesList();

    loadNetworkDevicesInfo();
    loadRoutingInfo();
    loadDNSInfo();
    loadNetworkProfiles();
    nameServersModified = false;
    devicesModified = false;
    modified = false;
//    kpbApply->setEnabled(false);
  }

}

/**
  Fill the Listview with the info of the network interfaces.
*/
void KNetworkConf::loadNetworkDevicesInfo()
{
  KNetworkInterface *device;
  QPixmap activeEthernetDeviceImg(KStandardDirs::locate("icon","hicolor/22x22/actions/network_connected_lan_knc.png"));
  QPixmap inactiveEthernetDeviceImg(KStandardDirs::locate("icon","hicolor/22x22/actions/network_disconnected_lan.png"));
  QPixmap activeWirelessDeviceImg(KStandardDirs::locate("icon","hicolor/22x22/actions/network_traffic_wlan.png"));
  QPixmap inactiveWirelessDeviceImg(KStandardDirs::locate("icon","hicolor/22x22/actions/network_disconnected_wlan.png"));

  klvCardList->clear();
  Q3PtrList<KNetworkInterface> deviceList = netInfo->getDeviceList();
  for (device = deviceList.first(); device; device = deviceList.next())
  {
    if ( device->getType() != "loopback" )
    {
#ifdef __GNUC__
#warning "Qt4: porting fixme"
#endif
      if (klvCardList->findItem(device->getDeviceName(),0,true)/*,CaseSensitive|ExactMatch) == 0*/)
      {
        Q3ListViewItem * item = new Q3ListViewItem( klvCardList, 0 );

        if (device->isActive())
        {
          if (device->getType() == "ethernet")
            item->setPixmap(0,activeEthernetDeviceImg);
          else if (device->getType() == "wireless")
            item->setPixmap(0,activeWirelessDeviceImg);

          item->setText(3,i18n("Enabled"));
          item->setPixmap(3,SmallIcon("dialog-ok"));
        }
        else
        {
          if (device->getType() == "ethernet")
            item->setPixmap(0,inactiveEthernetDeviceImg);
          else if (device->getType() == "wireless")
            item->setPixmap(0,inactiveWirelessDeviceImg);

          item->setText(3,i18n("Disabled"));
          item->setPixmap(3,SmallIcon("process-stop"));
          if (device->getBootProto().toLower() == "dhcp")
            item->setText(1,"");
        }

        item->setText(0,device->getDeviceName());
        item->setText(1,device->getIpAddress());
        qDebug("bootproto= %s",qPrintable(device->getBootProto()));
        if (device->getBootProto() == "none")
          item->setText(2,i18n("Manual"));
        else
          item->setText(2,device->getBootProto());
        item->setText(4,device->getDescription());
        QStringList l = deviceNamesList.grep(device->getDeviceName());
        if (l.size() == 0)
          deviceNamesList.append(device->getDeviceName());
      }
    }
  }
}


/** Terminates the application
 */
void KNetworkConf::quitSlot(){
  int code = 0;
  connect( config, SIGNAL(readyLoadingNetworkInfo()), this, SLOT(quitSlot()) );
  if (modified)
  {
    code = KMessageBox::warningYesNoCancel(this,
                          i18n("The new configuration has not been saved.\nDo you want to apply changes before quitting?"),
                          i18n("New Configuration Not Saved"),KStandardGuiItem::apply(),KStandardGuiItem::quit());
    if (code == KMessageBox::Yes)
      saveInfoSlot();
    else if (code == KMessageBox::No)
      qApp->quit();
  }
  else
    qApp->quit();
}
/** Enables the configure and remove buttons. */
void KNetworkConf::enableButtonsSlot(){
  if (!readOnly)
  {
     kpbConfigureNetworkInterface->setEnabled(true);
    Q3ListViewItem *item = klvCardList->currentItem();
    QString currentDevice = item->text(0);
    KNetworkInterface *dev = getDeviceInfo(currentDevice);

     if (dev->isActive())
     {
       kpbUpButton->setEnabled(false);
       kpbDownButton->setEnabled(true);
     }
     else
     {
       kpbUpButton->setEnabled(true);
       kpbDownButton->setEnabled(false);
     }
  }
}
/** opens the add server dialog. */
void KNetworkConf::addServerSlot(){
  KDialog addDlg(this);
  KAddDNSServerDlg *widget=new KAddDNSServerDlg(this);
  addDlg.setMainWidget(widget);
  addDlg.exec();
  if(widget->modified())
  {
    klbDomainServerList->insertItem(widget->kleNewServer->text());
    nameServersModified = true;
    enableApplyButtonSlot();
  }
}
/** opens the edit server dialog. */
void KNetworkConf::editServerSlot(){
  KDialog dlg(this);
  KAddDNSServerDlg *widget = new KAddDNSServerDlg(this);
  dlg.setMainWidget(widget);
  if (klbDomainServerList->currentItem() >= 0)
  {
    int currentPos = klbDomainServerList->currentItem();
    dlg.setCaption(i18n("Edit Server"));
    Q3ListBoxItem *item = klbDomainServerList->item(currentPos);
    QString currentText = item->text();
    widget->kleNewServer->setText(currentText);
    widget->kpbAddServer->setText(i18n("&OK"));
    dlg.exec();

    if(widget->modified())
    {
      klbDomainServerList->changeItem(widget->kleNewServer->text(),currentPos);
      nameServersModified = true;
      enableApplyButtonSlot();
    }
  }
}

/** Pops up the window for adding a new interface. */
void KNetworkConf::configureDeviceSlot(){
  Q3ListViewItem *item = klvCardList->currentItem();
  KWirelessInterface *wifiDev = NULL;

  //KAddDeviceContainer *configDlg = new KAddDeviceContainer(this,0);
  KAddDeviceContainer configDlg(this,0);

  if (item != NULL)
  {
    QString currentDevice = item->text(0);
    configDlg.setCaption(i18n("Configure Device %1", currentDevice));
    KNetworkInterface *dev = getDeviceInfo(currentDevice);
    KAddDeviceDlgExtension *advancedOptions = (KAddDeviceDlgExtension *)configDlg.extension();

    if (dev->getBootProto() == "none")
    {
      configDlg.addDlg->rbBootProtoManual->setChecked(true);
      configDlg.addDlg->rbBootProtoAuto->setChecked(false);
    }
    else if (dev->getBootProto().toLower() == "dhcp")
    {
      configDlg.addDlg->kcbAutoBootProto->setCurrentItem(0);
      configDlg.addDlg->rbBootProtoManual->setChecked(false);
      configDlg.addDlg->rbBootProtoAuto->setChecked(true);
      configDlg.addDlg->kleIPAddress->setEnabled(false);
      configDlg.addDlg->kcbNetmask->setEnabled(false);
      configDlg.kpbAdvanced->setEnabled(false);
    }
   else if (dev->getBootProto().toLower() == "bootp")
    {
      configDlg.addDlg->kcbAutoBootProto->setCurrentIndex(1);
      configDlg.addDlg->rbBootProtoManual->setChecked(false);
      configDlg.addDlg->rbBootProtoAuto->setChecked(true);
      configDlg.kpbAdvanced->setEnabled(false);
    }
    if (dev->getOnBoot() == "yes")
      configDlg.addDlg->kcbstartAtBoot->setChecked(true);
    else
    {
      configDlg.addDlg->kcbstartAtBoot->setChecked(false);
    }
    configDlg.addDlg->kleIPAddress->setText(dev->getIpAddress());
    advancedOptions->kleDescription->setText(dev->getDescription());
    advancedOptions->kleBroadcast->setText(dev->getBroadcast());

    if (!dev->getNetmask().isEmpty())
      configDlg.addDlg->kcbNetmask->setCurrentText(dev->getNetmask());

    if (readOnly)
    {
      configDlg.addDlg->kcbAutoBootProto->setEnabled(false);
      configDlg.addDlg->kcbstartAtBoot->setEnabled(false);
      advancedOptions->gbAdvancedDeviceInfo->setEnabled(false);
    }

    //If the interface is wireless, then add the wireless configuration widget
    if (dev->getType() == WIRELESS_IFACE_TYPE){
      wifiDev = static_cast<KWirelessInterface*>(dev);
      configDlg.addWirelessWidget();
      configDlg.extDlg->kleEssid->setText(wifiDev->getEssid());
      configDlg.extDlg->kleWepKey->setText(wifiDev->getWepKey());
    }

    configDlg.addButtons();
    //Disable Apply button so it only is enabled when a change is made
    configDlg.kpbApply->setEnabled(false);
    configDlg.exec();

    if (configDlg.modified())
    {
      if(configDlg.addDlg->rbBootProtoManual->isChecked())
      {
        item->setText(2,i18n("Manual"));
	dev->setBootProto("none");
      }
      //If the selected boot protocol is dhcp or bootp (Auto), then we don't need the
      //past IP address, netmask, network and broadcast, as a new one will be assigned by
      //the dhcp server.
      else if (configDlg.addDlg->rbBootProtoAuto->isChecked())
      {
        if (configDlg.addDlg->kcbAutoBootProto->currentText() != dev->getBootProto())
        {
          dev->setIpAddress("");
          configDlg.addDlg->kleIPAddress->setText("");
          dev->setNetmask("");
          dev->setNetwork("");
          dev->setBroadcast("");
        }
        item->setText(2,configDlg.addDlg->kcbAutoBootProto->currentText());
        dev->setBootProto(configDlg.addDlg->kcbAutoBootProto->currentText());
      }
      item->setText(1,configDlg.addDlg->kleIPAddress->text());
      item->setText(4,advancedOptions->kleDescription->text());

        if (valuesChanged(dev,
                        configDlg.addDlg->kcbAutoBootProto->currentText(),
                        configDlg.addDlg->kcbNetmask->currentText(),
                        configDlg.addDlg->kleIPAddress->text(),
                        configDlg.addDlg->kcbstartAtBoot->isChecked(),
	 		advancedOptions->kleDescription->text(),
		  	advancedOptions->kleBroadcast->text()))
        {
          dev->setIpAddress(configDlg.addDlg->kleIPAddress->text().trimmed());
          dev->setNetmask(configDlg.addDlg->kcbNetmask->currentText().trimmed());
          QString network = KAddressValidator::calculateNetwork(dev->getIpAddress().trimmed(),dev->getNetmask().trimmed());
          dev->setNetwork(network);
          QString broadcast;
          if (configDlg.advanced())
	          broadcast = advancedOptions->kleBroadcast->text().trimmed();
        else
            broadcast = KAddressValidator::calculateBroadcast(dev->getIpAddress().trimmed(),dev->getNetmask().trimmed());
          dev->setBroadcast(broadcast);
          dev->setDescription(advancedOptions->kleDescription->text());

          if (configDlg.addDlg->kcbstartAtBoot->isChecked())
            dev->setOnBoot("yes");
          else
            dev->setOnBoot("no");
        }
        //If the interface is wireless, then save the wireless configuration options
        if (dev->getType() == WIRELESS_IFACE_TYPE){
          wifiDev->setEssid(configDlg.extDlg->kleEssid->text());
          wifiDev->setWepKey(configDlg.extDlg->kleWepKey->text());
          dev = wifiDev;
        }
        devicesModified = true;
        enableApplyButtonSlot();
    }
  }
}

/**Returns the info of the network device 'device or NULL if not found.'*/
KNetworkInterface * KNetworkConf::getDeviceInfo(QString device){
  Q3PtrList<KNetworkInterface> deviceList = netInfo->getDeviceList();
  Q3PtrListIterator<KNetworkInterface>  i(deviceList);
  KNetworkInterface *temp;
  while ((temp = i.current()) != 0)
  {
    if (temp->getDeviceName() == device)
    {
      return temp;
    }
    ++i;
  }
  return NULL;
}

/**Returns the name of the network device that corresponds to the IP address 'ipAddr' or NULL if not found.'*/
QString KNetworkConf::getDeviceName(QString ipAddr){
  Q3PtrList<KNetworkInterface> deviceList = netInfo->getDeviceList();
  Q3PtrListIterator<KNetworkInterface>  i(deviceList);
  KNetworkInterface *temp;
  while ((temp = i.current()) != 0)
  {
    if (temp->getIpAddress().compare(ipAddr) == 0)
    {
      return temp->getDeviceName();
    }
    ++i;
  }
  return NULL;
}


/** Looks in the output returned by ifconfig to see if there are the devices up or down.*/
void KNetworkConf::readFromStdout(){
  commandOutput = "";
  commandOutput += procUpdateDevice->readStdout();
}

/** Loads the info about the default gateway and host and domain names. */
void KNetworkConf::loadRoutingInfo(){
  //routingInfo = config->getNetworkInfoSlot();
  if (routingInfo == NULL)
    KMessageBox::error(this,i18n("Could not open file '/etc/sysconfig/network' for reading."),
                        i18n("Error Loading Config Files"));
  else
  {
    kleDefaultRoute->setText(routingInfo->getGateway());
    kcbGwDevice->clear();
    kcbGwDevice->insertStringList(deviceNamesList);
    if (!routingInfo->getGatewayDevice().isEmpty())
      kcbGwDevice->setCurrentText(routingInfo->getGatewayDevice());
/*    if (routingInfo->isForwardIPv4Enabled().compare("yes") == 0)
      cbEnableRouting->setChecked(true);
    else
      cbEnableRouting->setChecked(false);*/
  }
}

void KNetworkConf::loadDNSInfo(){
  if (dnsInfo == NULL)
    KMessageBox::error(this,i18n("Could not open file '/etc/resolv.conf' for reading."),
                        i18n("Error Loading Config Files"));
  else
  {
    kleHostName->setText(dnsInfo->getMachineName());
    kleDomainName->setText(dnsInfo->getDomainName());
    klbDomainServerList->clear();
    const QStringList nameServers = dnsInfo->getNameServers();
    for ( QStringList::ConstIterator it = nameServers.begin(); it != nameServers.end(); ++it)
    {
      klbDomainServerList->insertItem(*it);
    }
    klvKnownHosts->clear();
    knownHostsList = dnsInfo->getKnownHostsList();
    Q3PtrListIterator<KKnownHostInfo> it(knownHostsList);
    KKnownHostInfo *host;
    while ((host = it.current()) != 0)
    {
      ++it;
      if (!(host->getIpAddress().isEmpty()))
      {
        Q3ListViewItem * item = new Q3ListViewItem( klvKnownHosts, 0 );
        item->setText(0,host->getIpAddress());
        const QStringList aliasesList = host->getAliases();
        QString aliases;
        for ( QStringList::ConstIterator it = aliasesList.begin(); it != aliasesList.end(); ++it )
        {
          aliases += *it + ' ';
        }
        item->setText(1,aliases);
      }
    }
  }
}

void KNetworkConf::loadNetworkProfiles(){
  Q3PtrListIterator<KNetworkInfo> it(profilesList);
  KNetworkInfo *profile = NULL;

  klvProfilesList->clear();
  while ((profile = it.current()) != 0)
  {
    ++it;
    if (!profile->getProfileName().isEmpty())
    {
      Q3ListViewItem * item = new Q3ListViewItem( klvProfilesList, 0 );
      item->setText(0,profile->getProfileName());
    }
  }
}

/** Shows the help browser. Hopefully some day it will be one :-). */
void KNetworkConf::helpSlot(){
  KToolInvocation::invokeHelp();
}

/** No descriptions */
void KNetworkConf::aboutSlot(){
  KAboutApplicationDialog *about = new KAboutApplicationDialog(KGlobal::mainComponent().aboutData());

 // about->setLogo(locate("icon","knetworkconf.png"));
  //qDebug("locate icon= %s",locate("icon","knetworkconf.png").toLatin1());

  about->show();
}
/** No descriptions */
void KNetworkConf::enableApplyButtonSlot(){
 //if (!readOnly)
    //kpbApply->setEnabled(true);
  modified = true;
  emit networkStateChanged(true);
}
/** Puts the application in read-only mode. This happens when the user runing
the application t root. */
void KNetworkConf::setReadOnly(bool state){
  KNetworkConf::readOnly = state;
}
/** No descriptions */
void KNetworkConf::enableApplyButtonSlot(const QString &text){
  enableApplyButtonSlot();
}
/** No descriptions */
void KNetworkConf::enableApplyButtonSlot(bool){
  enableApplyButtonSlot();
}
/** No descriptions */
void KNetworkConf::removeServerSlot(){
  if (klbDomainServerList->currentItem() >= 0)
  {
    klbDomainServerList->removeItem(klbDomainServerList->currentItem());
    enableApplyButtonSlot();
  }
}
void KNetworkConf::moveUpServerSlot(){
  int curPos = klbDomainServerList->currentItem();
  int antPos = klbDomainServerList->currentItem() - 1;

  if (antPos >= 0)
  {
    Q3ListBoxItem *current = klbDomainServerList->item(curPos);
    Q3ListBoxItem *ant = current->prev();
    QString antText = ant->text();
    klbDomainServerList->removeItem(antPos);
    klbDomainServerList->insertItem(antText,curPos);
    enableApplyButtonSlot();
  }
}
void KNetworkConf::moveDownServerSlot(){
  int curPos = klbDomainServerList->currentItem();
  unsigned nextPos = klbDomainServerList->currentItem() + 1;

  if (curPos != -1)
  {
    if (klbDomainServerList->count() >= nextPos)
    {
      Q3ListBoxItem *current = klbDomainServerList->item(curPos);
      QString curText = current->text();
      klbDomainServerList->removeItem(curPos);
      klbDomainServerList->insertItem(curText,nextPos);
      klbDomainServerList->setSelected(nextPos,true);
     enableApplyButtonSlot();
    }
  }
}
/** Disables all buttons a line edit widgets when the user has read only access. */
void KNetworkConf::disableAll(){
  kleHostName->setReadOnly(true);
  kleDomainName->setReadOnly(true);
  tlDomainName->setEnabled(false);
  tlHostName->setEnabled(false);
  disconnect(klvCardList,SIGNAL(doubleClicked(Q3ListViewItem *)),this,SLOT(configureDeviceSlot()));
  klvCardList->setEnabled(false);
  kpbUpButton->setEnabled(false);
  kpbDownButton->setEnabled(false);
  kpbConfigureNetworkInterface->setEnabled(false);
  gbDefaultGateway->setEnabled(false);
  gbDNSServersList->setEnabled(false);
  gbKnownHostsList->setEnabled(false);
//  gbNetworkOptions->setEnabled(false);
}

/** Saves all the modified info of devices, routes,etc. */
void KNetworkConf::saveInfoSlot(){
  config->setProgramVersion(getVersion());

  if (!KAddressValidator::isValidIPAddress(kleDefaultRoute->text()) && (!(kleDefaultRoute->text().isEmpty())))
  {
    KMessageBox::error(this,i18n("The default Gateway IP address is invalid."),i18n(" Invalid IP Address"));
  }
  else
  {
    //Update DNS info
    routingInfo->setDomainName(kleDomainName->text());
    routingInfo->setHostName(kleHostName->text());
    dnsInfo->setDomainName(kleDomainName->text());
    dnsInfo->setMachineName(kleHostName->text());
    dnsInfo->setNameServers(getNamserversList(klbDomainServerList));
    dnsInfo->setKnownHostsList(getKnownHostsList(klvKnownHosts));

    //Update routing info
    routingInfo->setGateway(kleDefaultRoute->text());
    if (routingInfo->getGateway().isEmpty())
      routingInfo->setGatewayDevice("");

    if (!kleDefaultRoute->text().isEmpty())
      routingInfo->setGatewayDevice(kcbGwDevice->currentText());

    //Save all info
    //netInfo->setDeviceList(deviceList);
    netInfo->setRoutingInfo(routingInfo);
    netInfo->setDNSInfo(dnsInfo);

    config->saveNetworkInfo(netInfo);
    modified = false;
  }
}
/** Creates a QStringList with the IP addresses contained in the QListBox of name servers. */
QStringList KNetworkConf::getNamserversList(K3ListBox * serverList){
  QStringList list;
  for (unsigned i = 0; i < serverList->count(); i++)
  {
    list.append(serverList->text(i));
  }
  return list;
}
/** Creates a QPtrList<KKownHostInfo> with the info contained in the K3ListView of name servers. */
Q3PtrList<KKnownHostInfo> KNetworkConf::getKnownHostsList(K3ListView * hostsList){
  Q3PtrList<KKnownHostInfo> list;
  Q3ListViewItem *it = hostsList->firstChild();
  for (int i = 0; i < hostsList->childCount(); i++)
  {
    if (!(it->text(0).isEmpty()))
    {
      KKnownHostInfo *host = new KKnownHostInfo();
      host->setIpAddress(it->text(0));
//      host->setHostName(it->text(1));
      host->setAliases(QStringList::split(" ",it->text(1)));
      it = it->nextSibling();
      list.append(host);
    }
  }
  return list;
}

QString KNetworkConf::getVersion(){
  return version;
}
void KNetworkConf::setVersion(QString ver){
  KNetworkConf::version = ver;
}

/** Changes the state of device 'dev' to DEVICE_UP or DEVICE_DOWN.
Return true on success, false on failure.  */
void KNetworkConf::changeDeviceState(const QString &dev, int state){
	KDialog *dialog = new KDialog(this);
    KInterfaceUpDownDlg* widget = new KInterfaceUpDownDlg(this );
	dialog->setMainWidget(widget);
    dialog->setCaption( i18n("Changing device state"));
  if (state == DEVICE_UP)
    widget->label->setText(i18n("Enabling interface <b>%1</b>", dev));
  else
    widget->label->setText(i18n("Disabling interface <b>%1</b>", dev));

  dialog->setModal(true);
  dialog->show();

  procDeviceState = new Q3Process(this);
  QString cmd;
  procDeviceState->addArgument( KStandardDirs::locate("data",BACKEND_PATH) );

  //If the platform couldn't be autodetected specify it manually
  if (netInfo->getPlatformName() != QString())
  {
    procDeviceState->addArgument( "--platform" );
    procDeviceState->addArgument( netInfo->getPlatformName() );
  }
  procDeviceState->addArgument( "-d" );

  if (state == DEVICE_UP)
    procDeviceState->addArgument("enable_iface::"+dev+"::1" );
  else if (state == DEVICE_DOWN)
    procDeviceState->addArgument("enable_iface::"+dev+"::0" );

  connect( procDeviceState, SIGNAL(readyReadStdout()),this, SLOT(readFromStdoutUpDown()) );
  connect( procDeviceState, SIGNAL(readyReadStderr()),this, SLOT(readFromStdErrUpDown()) );
  connect( procDeviceState, SIGNAL(processExited()),this, SLOT(verifyDeviceStateChanged()) );
  connect( procDeviceState, SIGNAL(processExited()), dialog, SLOT(close()) );

  currentDevice = dev;
  commandOutput = "";

  if ( !procDeviceState->start() )
  {
    // error handling
    KMessageBox::error(this,
                        i18n("Could not launch backend to change network device state. You will have to do it manually."),
                        i18n("Error"));
    dialog->close();
  }

}
void KNetworkConf::readFromStdoutUpDown(){
  commandOutput.append(procDeviceState->readStdout());
}

void KNetworkConf::verifyDeviceStateChanged(){
  KNetworkInterface *dev;
  QPixmap activeEthernetDeviceImg(BarIcon("network_connected_lan_knc"));
  QPixmap inactiveEthernetDeviceImg(BarIcon("network_disconnected_lan"));
  QPixmap activeWirelessDeviceImg(BarIcon("network_traffic_wlan"));
  QPixmap inactiveWirelessDeviceImg(BarIcon("network_disconnected_wlan"));

  commandOutput = commandOutput.section('\n',1);
  if (commandErrOutput.length() > 0)
  {
        KMessageBox::error(this,
                            i18n("There was an error changing the device's state. You will have to do it manually."),
                            i18n("Could Not Change Device State"));

  }
  else if (commandOutput == "\n<!-- GST: end of request -->")
  {
    Q3ListViewItem *item = klvCardList->findItem(currentDevice,0,/*,ExactMatch*/true);
    if (item != NULL)
    {
      dev = getDeviceInfo(currentDevice);
      if (!dev->isActive())
      {
        dev->setActive(true);
        if (dev->getType() == "ethernet")
          item->setPixmap(0,activeEthernetDeviceImg);
        else if (dev->getType() == "wireless")
          item->setPixmap(0,activeWirelessDeviceImg);

        item->setText(3,i18n("Enabled"));
        item->setPixmap(3,SmallIcon("dialog-ok"));
//        config->runDetectionScript(netInfo->getPlatformName());
        config->listIfaces(netInfo->getPlatformName());
//        item->setText(1,dev->getIpAddress());
      }
      else
      {
        dev->setActive(false);
        if (dev->getType() == "ethernet")
          item->setPixmap(0,inactiveEthernetDeviceImg);
        else if (dev->getType() == "wireless")
          item->setPixmap(0,inactiveWirelessDeviceImg);

        item->setText(3,i18n("Disabled"));
        item->setPixmap(3,SmallIcon("process-stop"));
        if (dev->getBootProto().toLower() == "dhcp")
          item->setText(1,"");
      }
      enableButtonsSlot();
    }
  }
}
/** Returns a list of strings of all the configured devices. */
QStringList KNetworkConf::getDeviceList(){
  QStringList list;
  KNetworkInterface * device;
  Q3PtrList<KNetworkInterface> deviceList = netInfo->getDeviceList();
  for (device = deviceList.first(); device; device = deviceList.next())
  {
    list.append(device->getDeviceName());
  }
  return list;
}
/** No descriptions */
bool KNetworkConf::valuesChanged(KNetworkInterface *dev,
                                  QString bootProto,
                                  QString netmask,
                                  QString ipAddr,
                                  bool onBoot,
                                  QString desc,
				  QString broadcast){
  if ((dev->getBootProto() != bootProto) ||
      (dev->getNetmask() != netmask) ||
      (dev->getIpAddress() != ipAddr) ||
      ((dev->getOnBoot() == "yes") && !(onBoot)) ||
      ((dev->getOnBoot() == "no") && (onBoot)) ||
      (dev->getDescription() != desc) ||
      (dev->getBroadcast() != broadcast))
    return true;
  else
    return false;
}

/** Sets the QPushButton::autoResize() in true for all buttons. */
void KNetworkConf::makeButtonsResizeable(){
#ifdef __GNUC__
#warning "QT4 porting fixme"
#endif
  //kpbConfigureNetworkInterface->setAutoResize(true);
  //kcbGwDevice->setAutoResize(true);
  //kpbAddDomainServer->setAutoResize(true);
  //kpbEditDomainServer->setAutoResize(true);
  //kpbRemoveDomainServer->setAutoResize(true);
  //kpbUpButton->setAutoResize(true);
  //kpbDownButton->setAutoResize(true);
  //kpbAddKnownHost->setAutoResize(true);
  //kpbEditKnownHost->setAutoResize(true);
  //kpbRemoveKnownHost->setAutoResize(true);
}

void KNetworkConf::enableInterfaceSlot()
{
  KNetworkInterface *dev = getDeviceInfo(klvCardList->currentItem()->text(0));
  if (dev->isActive())
    changeDeviceState(dev->getDeviceName(),DEVICE_DOWN);
  else
    changeDeviceState(dev->getDeviceName(),DEVICE_UP);
}

void KNetworkConf::disableInterfaceSlot()
{
  KNetworkInterface *dev = getDeviceInfo(klvCardList->currentItem()->text(0));
  if (dev->isActive())
    changeDeviceState(dev->getDeviceName(),DEVICE_DOWN);
  else
    changeDeviceState(dev->getDeviceName(),DEVICE_UP);
}

/** Adds a new host to the K3ListView that has the known hosts. */
void KNetworkConf::addKnownHostSlot(){
  KDialog *dialog = new KDialog(this);
  KAddKnownHostDlg *dlg = new KAddKnownHostDlg(this);
  dialog->setMainWidget(dlg);
  dialog->setCaption(i18n("Add New Static Host"));
  QString aliases;

  dialog->exec();

  if (!dlg->kleIpAddress->text().isEmpty() && dlg->klbAliases->firstItem())
  {
    Q3ListViewItem * item = new Q3ListViewItem( klvKnownHosts, 0 );

    item->setText(0,dlg->kleIpAddress->text());

    for ( uint i = 0; i < dlg->klbAliases->count(); i++ )
      aliases += dlg->klbAliases->text(i) + ' ';

    item->setText(1,aliases);
    enableApplyButtonSlot();
  }
}

/** Removes a known host from the list view */
void KNetworkConf::removeKnownHostSlot()
{
  if (klvKnownHosts->currentItem() != 0)
  {
    klvKnownHosts->removeItem(klvKnownHosts->currentItem());
    enableApplyButtonSlot();
  }
}

/** Edits the info about a known host. */
void KNetworkConf::editKnownHostSlot()
{
  KDialog *dialog = new KDialog(this);
  KAddKnownHostDlg *dlg = new KAddKnownHostDlg(this);
  dialog->setMainWidget(dlg);
  dialog->setCaption(i18n("Edit Static Host"));
  Q3ListViewItem *item = klvKnownHosts->currentItem();
  dlg->kleIpAddress->setText(item->text(0));

  const QStringList aliases = QStringList::split( " ", item->text(1) );
  int n = 0;
  for ( QStringList::ConstIterator it = aliases.begin(); it != aliases.end(); ++it, ++n )
  {
    QString alias = *it;
    dlg->klbAliases->insertItem(alias,n);
  }

  dialog->exec();

  QString _aliases;
  if (!dlg->kleIpAddress->text().isEmpty() && dlg->klbAliases->firstItem() )
  {
    Q3ListViewItem * item = klvKnownHosts->currentItem();

    item->setText(0,dlg->kleIpAddress->text());

    for ( uint i = 0; i < dlg->klbAliases->count(); i++ )
      _aliases += dlg->klbAliases->text(i) + ' ';

    item->setText(1,_aliases);
    enableApplyButtonSlot();
  }

}

/** Shows the main window after the network info has been loaded. */
void KNetworkConf::showMainWindow()
{
  show();
}
/** No descriptions */
void KNetworkConf::readFromStdErrUpDown()
{
  commandErrOutput.append(procDeviceState->readStderr());
}
/** Sees if a device is active or not in the ifconfig output. Not very nice, but it works. Inthe future, this has to be managed by gst. */
bool KNetworkConf::isDeviceActive(const QString &device, const QString &ifconfigOutput){
    QString temp = ifconfigOutput.section(device,1,1);
    if (temp.isEmpty())
      return false;
    else
    {
      QString temp2 = temp.section("UP",0,0); //two firts lines of the device info.
      QString temp3 = temp2.section("\n",0,0); //Link encap:Ethernet  HWaddr 00:00:21:C5:99:A0
      QString temp4 = temp2.section("\n",1,1); //inet addr:192.168.1.1  Bcast:192.255.255.255  Mask:255.0.0.0
      temp3 = temp3.trimmed();
      temp4 = temp4.trimmed();
      QString temp5 = temp3.section(" ",4,4); //00:00:21:C5:99:A0
      QString temp6 = temp4.section(" ",1,1); // addr:192.168.1.1
      temp6 = temp6.section(":",1,1); //192.168.1.1
      QString temp7 = temp4.section(" ",3,3); //Bcast:192.255.255.255
      temp7 = temp7.section(":",1,1); //192.255.255.255
      QString temp8 = temp4.section(" ",5,5); // Mask:255.0.0.0
      temp8 = temp8.section(":",1,1); //255.0.0.0

      //If the ip address is empty it must be a dhcp interface, so fill these fields:
      if (temp6.isEmpty())
        return false;
    }
    return true;
}
void KNetworkConf::setReadOnlySlot(bool state)
{
  state = !state;
  gbDefaultGateway->setEnabled(state);
  kleDomainName->setEnabled(state);
  kleHostName->setEnabled(state);
  gbDNSServersList->setEnabled(state);
  gbKnownHostsList->setEnabled(state);
  klvCardList->setEnabled(state);
  kpbUpButton->setEnabled(state);
  kpbDownButton->setEnabled(state);
  kpbConfigureNetworkInterface->setEnabled(state);
}

/*Shows a context menu when right-clicking in the interface list*/
void KNetworkConf::showInterfaceContextMenuSlot(K3ListView* lv, Q3ListViewItem* lvi, const QPoint& pt)
{
  KMenu *context = new KMenu( this );
  Q_CHECK_PTR( context );
  context->insertItem( "&Enable Interface", this, SLOT(enableInterfaceSlot()));
  context->insertItem( "&Disable Interface", this, SLOT(disableInterfaceSlot()));
  Q3ListViewItem *item = klvCardList->currentItem();
  QString currentDevice = item->text(0);
  KNetworkInterface *dev = getDeviceInfo(currentDevice);

  if (dev->isActive())
  {
    context->setItemEnabled(0,false);
    context->setItemEnabled(1,true);
  }
  else
  {
    context->setItemEnabled(0,true);
    context->setItemEnabled(1,false);
  }
  context->insertSeparator(2);
  context->insertItem( "&Configure Interface...", this, SLOT(configureDeviceSlot()));
  context->popup(pt);
  //context->insertItem( "About &Qt", this, SLOT(aboutQt()) );
}

void KNetworkConf::enableSignals()
{
  connect(kleDefaultRoute,SIGNAL(textChanged(const QString&)),this,SLOT(enableApplyButtonSlot(const QString&)));
  connect(kleDomainName,SIGNAL(textChanged(const QString&)),this,SLOT(enableApplyButtonSlot(const QString&)));
  connect(kleHostName,SIGNAL(textChanged(const QString&)),this,SLOT(enableApplyButtonSlot(const QString&)));
}

void KNetworkConf::enableProfileSlot()
{
  //Get selected profile
  Q3ListViewItem *item = klvProfilesList->currentItem();
  if(!item)
	return;
  QString selectedProfile = item->text(0);

  //And search for it in the profiles list
  KNetworkInfo *profile = getProfile(netInfo->getProfilesList(),selectedProfile);
  if (profile != NULL)
  {
    profile->setProfilesList(netInfo->getProfilesList());
    config->saveNetworkInfo(profile);
    modified = false;
    //connect( config, SIGNAL(readyLoadingNetworkInfo()), this, SLOT(showSelectedProfile(selectedProfile)) );
  }
  else
    KMessageBox::error(this,
                        i18n("Could not load the selected Network Profile."),
                        i18n("Error Reading Profile"));

}

KNetworkInfo *KNetworkConf::getProfile(Q3PtrList<KNetworkInfo> profilesList, QString selectedProfile)
{
  Q3PtrListIterator<KNetworkInfo> it(profilesList);
  KNetworkInfo *net = NULL;

  while ((net = it.current()) != 0)
  {
    ++it;
    if (net->getProfileName() == selectedProfile)
      break;
  }
  return net;
}

void KNetworkConf::showSelectedProfile(QString profile)
{

}


#include "knetworkconf.moc"
