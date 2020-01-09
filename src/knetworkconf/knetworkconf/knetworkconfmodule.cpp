/***************************************************************************
                          knetworkconfmodule.cpp  -  description
                             -------------------
    begin                : Tue Apr 1 2003
    copyright            : (C) 2003 by Juan Luis Baptiste
    email                : juancho@linuxmail.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "knetworkconfmodule.h"

#include <kdeversion.h>
#include <klocale.h>
#include <kcmodule.h>
#include <QLayout> 
//Added by qt3to4:
#include <QVBoxLayout>
#include <version.h>
#include <kgenericfactory.h>


K_PLUGIN_FACTORY(KNetworkConfFactory, registerPlugin<KNetworkConfModule>();)
K_EXPORT_PLUGIN(KNetworkConfFactory("knetworkconfmodule"))

KNetworkConfModule::KNetworkConfModule(QWidget *parent,const QVariantList &)
  : KCModule(KNetworkConfFactory::componentData(), parent)
{
  QVBoxLayout *top = new QVBoxLayout(this);
  
  conf = new KNetworkConf(this);
  conf->setVersion(QString(KDE_VERSION_STRING));
  conf->setReadOnly(false);
  
  top->addWidget(conf);

  if (getuid() != 0){
    conf->setReadOnlySlot(true);
    conf->setReadOnly(true);
  }  

  connect(conf,SIGNAL(networkStateChanged(bool)),SLOT(configChanged(bool)));
  setButtons(KCModule::Apply|KCModule::Help); 
}

KNetworkConfModule::~KNetworkConfModule()
{
}

void KNetworkConfModule::configChanged(bool b)
{
  emit changed(b);
}

void KNetworkConfModule::load()
{
 // conf->loadNetworkDevicesInfo();
  //conf->loadRoutingAndDNSInfo();
  //conf->loadDNSInfoTab();
}

void KNetworkConfModule::save()
{
   conf->saveInfoSlot();
}
/*
int KNetworkConfModule::buttons()
{
  return KCModule::Ok|KCModule::Apply|KCModule::Help;
}
*/
bool KNetworkConfModule::useRootOnlyMsg() const
{
  return true;
}

QString KNetworkConfModule::rootOnlyMsg() const
{
  return "Changing the network configuration requires root access";
}

KAboutData* KNetworkConfModule::aboutData() const
{
  /*KAboutData* data = new KAboutData( "knetworkconf", 0, ki18n("KNetworkConf"),
    VERSION, ki18n(description), KAboutData::License_GPL,
    ki18n("(c) 2003, Juan Luis Baptiste"), KLocalizedString(), "http://www.merlinux.org/knetworkconf/", "jbaptiste@merlinux.org");
  data->addAuthor(ki18n("Juan Luis Baptiste"),ki18n("Lead Developer"),
                      "jbaptiste@merlinux.org");
  data->addCredit(ki18n("David Sansome"),ki18n("Various bugfixes and features"),
                      "me@davidsansome.com");

  return data; // Memory leak, oh well...*/
  
KAboutData *aboutData = new KAboutData( "knetworkconf", 0, ki18n("KNetworkConf"),
    KDE_VERSION_STRING, ki18n(description), KAboutData::License_GPL,
    ki18n("(c) 2003 - 2005, Juan Luis Baptiste"), KLocalizedString(), "http://www.merlinux.org/knetworkconf/", "juan.baptiste@kdemail.net");
  aboutData->addAuthor(ki18n("Juan Luis Baptiste"),ki18n("Lead Developer"),
                      "juan.baptiste@kdemail.net");
  aboutData->addCredit(ki18n("Carlos Garnacho and the Gnome System Tools Team"),ki18n("Provided the Network backend which KNetworkConf relies on."),
                      "garnacho@tuxerver.net","http://www.gnome.org/projects/gst/");
  aboutData->addCredit(ki18n("Helio Chissini de Castro"),ki18n("Conectiva Linux Support"),
                      "helio@conectiva.com.br");
  aboutData->addCredit(ki18n("Christoph Eckert"),ki18n("Documentation maintainer, and German translator"),
                      "mchristoph.eckert@t-online.de ");  
  aboutData->addCredit(ki18n("David Sansome"),ki18n("Various bugfixes and features"),
                      "me@davidsansome.com");                      
  aboutData->addCredit(ki18n("Gustavo Pichorim Boiko"),ki18n("Various bugfixes and Brazilian Portuguese translator"),"gustavo.boiko@kdemail.net");
  return aboutData;// Memory leak, oh well...                      
}

QString KNetworkConfModule::quickHelp() const
{
  return i18n("<h1>Network configuration</h1><p>This module allows you to configure your TCP/IP settings.</p>");
}

//#include "knetworkconfmodule.moc"

#include "knetworkconfmodule.moc"
