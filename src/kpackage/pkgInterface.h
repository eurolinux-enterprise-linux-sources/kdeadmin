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


#ifndef PKGINTERFACE_H
#define PKGINTERFACE_H

#include <QtCore/QList>

#include <QtCore/QStringList>
//Added by qt3to4:
#include <QtGui/QPixmap>

#include <kglobal.h>
#include <kstandarddirs.h>

#include "packageInfo.h"
#include "procbuf.h"
#include "managementWidget.h"

class packageInfo;
class pkgOptions;
class Locations;
class LcacheObj;
class cacheObj;
class KActionCollection;
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//  flags to install and uninstall
class param
{
public:
  param(const QString &nameP, bool initP,  bool invertP, const QString &flagP);
  param(const QString &nameP, bool initP,  bool invertP, const QString &flagP, const QString &flagAP);
  ~param();

  QString name;  // Name of flag
  bool init;         // Initial value
  bool invert;       // Whether it needs to be inverted
  QString flag; // text flag on command
  QString flagA; // text flag on command
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class pkgInterface: public QObject
{
  Q_OBJECT

public:
  pkgInterface();
   ~pkgInterface();

  virtual void makeMenu(KActionCollection* act);
  virtual void setMenu(KActionCollection* act, bool enable);

  bool ifExe(QString exe);
  // Check if this executable exists
  
   virtual packageInfo *getPackageInfo(const QString &name) ;
  // get info on installed  package

   virtual void getFileList(packageInfo *p);
  // get list of files in the package

   virtual void getIFileList(packageInfo *p);
   // get list of files in installed package
   
   virtual void getFFileList(packageInfo *p);
   // get list of files in package file
   
   QStringList depends(const QString &name, int src);
  // check dependencies for package

   QStringList verify(packageInfo *p, const QStringList &files);
  // check the installed files in a package

   virtual QStringList FindFile(const QString &name, bool seachAll=false);
  // search for packages containing a file

   virtual QStringList getChangeLog(packageInfo *p);
  // Get change log

   virtual bool filesTab(packageInfo *p);
  // If files tab is to be enabled

    bool depTab(packageInfo *p) ;
  // If dependency tab is to be enabled

   virtual bool changeTab(packageInfo *p);
  // If change log tab is to be enabled

  bool parseName(const QString &name, QString *n, QString *v);
  // breakup file name into package name and version

   void listPackages(QList<packageInfo *> &pki);
  // scan various locations for list of packages

  void listRemotePackages(QList<packageInfo *> &pki);

  void listPackagesFile(QList<packageInfo *> &pki);

   QStringList getPackageState( const QStringList &packages);
  // update installed/uninstalled state of packages

  void getPackageDepends(packageInfo *p);
  // Get package depends info
      
  virtual packageInfo *getFPackageInfo(const QString &name);
  // Get package Info from file

   void listInstalledPackages(QList<packageInfo *> &pki);
  // produce list of currently installed packages

   QStringList  listInstalls(const QStringList &packs, bool install, bool &cancel);
  // Convert list of packages requested to install to list of all packages to install

   void smerge(packageInfo *p);
  // merge in package info entry

  packageInfo *collectPath(const QString &name);
  // 
      
  packageInfo *collectDir(const QString &name, const QString &size, const QString &dir);
  // build packageInfo object from directory entry

   QString  provMap(const QString &p);
  // convert from package depends to package

  QString setOptions(int flags, QList<param *> &params);
  // convert un/install flags to text

   packageInfo* collectInfo(QStringList &ln,  pkgInterface *pkgInt = 0);
  // Parse package info

   QString doUninstall(int uninstallFlags, const QString &packs, bool &test);
   QString doInstall(int installFlags, const QString &packs, bool &test);

   void removeLastEmpty(QStringList &list); 

   QString uninstall(int uninstallFlags, QList<packageInfo *> p,
				  bool &test);
   QString uninstall(int uninstallFlags, packageInfo *p,
				  bool &test);
  
   QString install(int installFlags, QList<packageInfo *> p,
				  bool &test);
   QString install(int installFlags, packageInfo *p,
				  bool &test);

  QString getDelimiter(short ptype);
   // return name version delimiter character
   
  ///////////// DATA ///////////////////////
  
  pkgOptions *uninstallation, *installation;

  QAction *updateM, *upgradeM, *fixupM, *fileM;

  QString icon;
  // name icon file
 QPixmap pict, bad_pict, new_pict, updated_pict;
  // icons for package states
  QPixmap folder;
  // icon for package group
  QPixmap markInst;
  QPixmap markUnInst;
  // icon indicating mark for install/uninstall

  Locations *locatedialog;
  // dialog for setting the locations of  uninstalled packages
  LcacheObj *packageLoc;
  // List of locations of uninstalled pacckages

  bool dirOK;
  // variables related to reading packages from directories

  QString packagePattern;
  QString queryMsg;
  // Parameters for reading packages from directories

  QList<param *> paramsInst;
  QList<param *> paramsUninst;
  
  QRegExp pFile, deb, rpm, slack, fin;
  // Regular expressions that match package names
  
  bool noFetch;
  // kpackage doesn't fetch this type of package itself

  bool defaultHandle;
  // This package type defaults to on

  QString errExe;
  // The name of an executable that wasn't found

  procbuf reader;
  QString procMsg;
  // for running processes

  QString DELMSG;

  bool hasRemote;
  // can access on remote host

  bool hasSearchAll;
  // can search uninstalled packages for files

  bool hasProgram;
  // the program needed to handle this package type is available
  
  int pCnt;
  // Count of packages read in
  
  QString env;
  
  QStringList packList;
  // Buffer for splitting up package entries

  short lastType;
  // The type of the last package

public slots:
   void textIn(const QString &s);
  // Process remote packace info
   
  void updateS();	
  void upgradeS();
  void fixupS();
 
};

#endif // PKGINTERFACE_H
