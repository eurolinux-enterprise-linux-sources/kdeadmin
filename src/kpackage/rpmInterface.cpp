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


//////////////////////////////////////////////////////////////////////////////
///
///               RPM Program version
///
//////////////////////////////////////////////////////////////////////////////

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "kpPty.h"
#include "kpackage.h"
#include "rpmInterface.h"

RPM::RPM():pkgInterface()
{  
icon = "rpm";

 pict = UserIcon(icon);
 updated_pict = UserIcon("rupdated");
 new_pict = UserIcon("rnew");

  hasRemote = true;

  paramsInst.append(new param(i18n("Download only"),false,false,"--download"));
  
  infoList.append("name/%{NAME}");
  infoList.append("version/%{VERSION}");
  infoList.append("release/%{RELEASE}");
  infoList.append("summary/%{SUMMARY}");
  infoList.append("url/%{URL}");
  infoList.append("architecture/%{ARCH}");
  infoList.append("group/%{GROUP}");
  infoList.append("distribution/%{DISTRIBUTION}");
  infoList.append("vendor/%{VENDOR}");
  infoList.append("packager/%{PACKAGER}");
  infoList.append("installtime/%{INSTALLTIME:date}");
  infoList.append("buildtime/%{BUILDTIME:date}");
  infoList.append("size/%{SIZE}");
  infoList.append("provides/[%{PROVIDES}, ]");
  infoList.append("requires/[%{REQUIRENAME} (%{REQUIREFLAGS:depflags} %{REQUIREVERSION}), ]");
  infoList.append("description/[%{DESCRIPTION}]");

  hasProgram = ifExe("rpm");
}

RPM::~RPM(){}

/////////////////////////////////////////////////////////////////////////////

QStringList RPM::getChangeLog(packageInfo *p)
{
  QStringList clog;
  QString fn( p->getFilename());

  if(!fn.isEmpty())
    return getUChangeLog(fn);
  else
    return getIChangeLog(p);

  return clog;
}


// query an installed package
QStringList RPM::getIChangeLog(packageInfo *p)
{
  QString name = p->getInfo("name");

  QString cmd = "rpm -q --changelog ";
  cmd += name;

  QStringList filelist = kpty->run(cmd);

  return filelist;
}


// query an uninstalled package
QStringList RPM::getUChangeLog(const QString &fn)
{
  QString cmd = "rpm -q --changelog -p ";
  cmd += quotePath(fn);

  QStringList filelist = kpty->run(cmd);

  return filelist;
}


bool RPM::filesTab(packageInfo *p) {
  if (p->packageState == packageInfo::INSTALLED) {
    return true;
  } else if (p->isFileLocal()) {
    return true;
  } 
  return false;
}

bool RPM::changeTab(packageInfo *p) {
  if (p->packageState == packageInfo::INSTALLED) {
    return true;
  } else if (p->isFileLocal()) {
    return true;
  } 
  return false;
}

//////////////////////////////////////////////////////////////////////////////

// query an installed package
void RPM::getIFileList(packageInfo *p)
{
  QString name = p->getInfo("name");

  QString cmd = "rpm -q -l ";
  cmd += name;

  QStringList filelist = kpty->run(cmd);

  p->files = new QStringList(filelist);
}


// query an uninstalled package
void RPM::getFFileList(packageInfo *p)
{
  QString fn = p->getFilename();
  QString cmd = "rpm -q -l -p ";
  cmd += quotePath(fn);

  QStringList filelist = kpty->run(cmd);

  p->files = new QStringList(filelist);
}

//////////////////////////////////////////////////////////////////////////////

packageInfo *RPM::getPackageInfo( const QString &name )
{
  // query an installed package!
  QString cmd = "rpm -q";
  cmd += packageQuery();
  cmd += " ";
  cmd += name;

  QStringList infoList = kpty->run(cmd);
  packageInfo *pki = collectInfo(infoList);
  if (pki) {
    pki->packageState = packageInfo::INSTALLED;
//    collectDepends(pki,name,0);
  }
  return pki;
}

packageInfo *RPM::getFPackageInfo( const QString &name )
{
  // query an uninstalled package
  QString cmd = "rpm -q";
  cmd += packageQuery();
  cmd += " -p ";
  cmd += quotePath(name);

  QStringList infoList =  kpty->run(cmd);
  packageInfo *pki = collectInfo(infoList);
  if (pki) {
    pki->updated = TRUE;
    pki->packageState = packageInfo::AVAILABLE;
    if (pki->hasInfo("install time"))
      pki->info.remove("install time");
//    collectDepends(pki,name,1);
  }

  return pki;
}

//////////////////////////////////////////////////////////////////////////////

QStringList RPM::FindFile(const QString &name, bool) {
  QString cmd = "rpm -q -a --filesbypkg";
    
  QStringList list =  kpty->run(cmd);
  QStringList retlist;
  if (kpty->Result > 0) {
    list.clear();
  } else {
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
      int p =  (*it).find(" ");
      int nm = (*it).find(name,p);
      if (nm >= 0) {
        (*it).replace(p, 1, "\t");
        retlist.append(*it);
      }
    }
  }

  return retlist;
}

//////////////////////////////////////////////////////////////////////////////
QString RPM::quotePath( const QString &path) {
  QString s = path;
  s = s.replace(" ","\\ ");
  return ( "'" + s + "'" );
}

QString RPM::packageQuery() {
  QString cmd =  " --queryformat '";
  for ( QStringList::const_iterator it = infoList.constBegin(); it != infoList.constEnd(); ++it ) {
    QStringList s = QStringList::split("/",*it);
    cmd += "==";
    cmd += s[0];
    cmd += "\\n";
    cmd += s[1];
    cmd += "\\n";
  }
  cmd += "==\\n'";
  return cmd;
}
