/*
** Copyright (C) 1999,2000 Toivo Pedaste <toivo@ucs.uwa.edu.au>
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/

#ifndef RPM_IFACE_H
#define RPM_IFACE_H

#include "packageInfo.h"
#include "pkgInterface.h"

class RPM : public pkgInterface
{
 Q_OBJECT

public:
  RPM();
  ~RPM();

  bool isType(char *buf, const QString &fname);

  packageInfo *getPackageInfo(char mode, const QString &name, const QString &version);
  QStringList getChangeLog(packageInfo *p);

  bool filesTab(packageInfo *p);
  // If files tab is to be enabled

  bool changeTab(packageInfo *p);
  // If change log tab is to be enabled
  
  void getIFileList( packageInfo *p );
  void getFFileList( packageInfo *p );
  
  packageInfo *getPackageInfo( const QString &name);
  packageInfo *getFPackageInfo( const QString &name);

  QStringList depends(const QString &name, int src);
  QStringList verify(packageInfo *p, const QStringList &files);

  QStringList FindFile(const QString &name, bool seachAll=false);

  QString provMap(const QString &p);

private:

  bool rpmSetup;
  QStringList infoList;

  QStringList getIChangeLog( packageInfo *p );
  QStringList getUChangeLog( const QString &fn );

  QString quotePath( const QString &path);
  QString packageQuery();
  
 };

#endif
