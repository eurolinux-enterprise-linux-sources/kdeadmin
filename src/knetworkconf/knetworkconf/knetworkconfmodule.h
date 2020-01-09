/***************************************************************************
                          knetworkconfmodule.h  -  description
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

#ifndef KNETWORKCONFMODULE_H
#define KNETWORKCONFMODULE_H

#include "knetworkconf.h"
#include <kaboutdata.h>
#include <kcmodule.h>
#include <KPluginFactory>
class KNetworkConfModule : public KCModule
{
  Q_OBJECT

public:
  explicit KNetworkConfModule(QWidget * parent = 0, const QVariantList &list = QVariantList());
  ~KNetworkConfModule();

  void load();
  void save();
  int buttons();

  bool useRootOnlyMsg() const;
  QString rootOnlyMsg() const;
  KAboutData* aboutData() const;
  QString quickHelp() const;

private slots:
  void configChanged(bool);

private:
  KNetworkConf* conf;
};

#endif
