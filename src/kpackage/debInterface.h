/*
** Copyright (C) 1999,2007 Toivo Pedaste <toivo@ucs.uwa.edu.au>
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

#ifndef DEBINTERFACE_H
#define DEBINTERFACE_H

#include <kaction.h>
#include <pkgInterface.h>

//Added by qt3to4:
//#include <Qt3Support/Q3PtrList>

#define APT_SOURCE "/etc/apt/sources.list"

class DEB: public pkgInterface
{
  Q_OBJECT

public:
  DEB();
  ~DEB();

  virtual packageInfo *getPackageInfo(const QString &name);
  virtual QStringList getChangeLog(packageInfo *p);

  bool filesTab(packageInfo *p);
  // If files tab is to be enabled

  bool depTab(packageInfo *p);
  // If dependency tab is to be enabled

  bool changeTab(packageInfo *p);
  // If change log tab is to be enabled

  packageInfo *getFPackageInfo(const QString &name);
  // Get package Info from file
  
  QStringList FindFile(const QString &name, bool seachAll=false);

protected:
  packageInfo *getIPackageInfo(const QString &name);
  packageInfo *getIRPackageInfo(const QString &name);

  QStringList getIChangeLog(packageInfo *p);

  void getIFileList(packageInfo *p);
  void getFFileList(packageInfo *p);


 void listRemotePackages(QList<packageInfo *> &pki);

  void makeMenu(KActionCollection* act);
  void setMenu(KActionCollection* act, bool enable);

 
private slots:
 void fileS();
 
private:

  QStringList getRFileList(packageInfo *p);

};

#endif // DEBINTERFACE_H
