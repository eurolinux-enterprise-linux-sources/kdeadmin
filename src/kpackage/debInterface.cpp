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

//Added by qt3to4:
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtGui/QApplication>

#include <kactioncollection.h>
#include "klocale.h"
#include <kaction.h>
#include <kstandardaction.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kshell.h>

#include "kpackage.h"
#include "debInterface.h"
#include "pkgOptions.h"
#include "managementWidget.h"
#include "findf.h"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

DEB::DEB()
{
 icon = "deb";

  pict = UserIcon(icon);
  bad_pict = UserIcon("dbad");
  updated_pict = UserIcon("dupdated");
  new_pict = UserIcon("dnew");


  hasRemote = true;


  paramsInst.append(new param(i18n("Download only"),false,false,"--download"));
 
 // paramsUninst.append(new param(i18n("Purge Config Files"),false,false,"--purge"));
 
  env = "DEBIAN_FRONTEND=readline; export DEBIAN_FRONTEND; ";

  hasSearchAll = true;

  
}

//////////////////////////////////////////////////////////////////////////////

DEB::~DEB()
{
}

//////////////////////////////////////////////////////////////////////////////
packageInfo *DEB::getPackageInfo(const QString &name)
{
//       kDebug() << "DEB::getPackageInfo\n";
    if (hostName.isEmpty()) {
      return getIPackageInfo(name);
    } else {
      return getIRPackageInfo(name);
    }
}

packageInfo *DEB::getIPackageInfo( const QString &)
{
    return 0;
}

packageInfo *DEB::getIRPackageInfo( const QString &name)
{
  // query an remote installed package
  packageInfo *pki = 0;
  
  connect(kpty, SIGNAL(textIn(const QString &, bool)), this,
	  SLOT(textIn(const QString &, bool)));

  QString s = "dpkg --status ";
  s += name;
  QStringList list =  kpty->run(s);
  
  disconnect(kpty, SIGNAL(textIn(const QString &, bool)), this,
	  SLOT(textIn(const QString &, bool)));
  
  return pki;
}

//////////////////////////////////////////////////////////////////////

packageInfo *DEB::getFPackageInfo( const QString &name)
{
  // query an uninstalled package
  packageInfo *pki = 0;

  QString s = "dpkg --info ";
  s += KShell::quoteArg(name);

  QStringList list =  kpty->run(s);
  for ( QStringList::const_iterator it = list.constBegin();  it != list.constEnd(); ++it ) {
    //    kDebug() << "U=" << *it << "\n";
    if ((*it).indexOf("Package:") >= 0) {
      //      kDebug() << "found\n";
      while (it != list.constBegin()) {
	list.erase(list.begin());
      }
      break;
    }
  }

  foreach (QString t, list) {
    t.remove(1,1);
  }
  if (list.count() > 1) {
    pki = collectInfo(list, this); 
    if (pki) {
      pki->updated = true;
     }
  }

  return pki;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

QStringList DEB::getChangeLog(packageInfo *p) {
  QString fn( p->getFilename());
  if(!fn.isEmpty())
    return QStringList();
  else
    return getIChangeLog(p);
}

QStringList DEB::getIChangeLog(packageInfo *p)
{
  QString from;
  QStringList ret;
  QString name = p->getInfo("name");

  from = "zcat /usr/share/doc/";
  from += name;
  from += "/changelog.Debian.gz";

  ret = kpty->run(from);

  if (!kpty->Result)
    return ret;
  else {
    from = "zcat /usr/share/doc/";
    from += name;
    from += "/changelog.gz";

    ret = kpty->run(from);
    if (!kpty->Result)
      return ret;
    else
      return QStringList();
  }
}

//////////////////////////////////////////////////////////////////////////////

bool DEB::filesTab(packageInfo *p) {
  if (p->packageState == packageInfo::INSTALLED) {
    return true;
  } else if (p->isFileLocal()) {
    return true;
  }
  return false;
}

bool DEB::changeTab(packageInfo *p) {
  if (p->packageState == packageInfo::INSTALLED) {
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
// query files from an installed package
void DEB::getIFileList(packageInfo *p)
{
  QString s = "dpkg -L ";
  s += p->getInfo("name");
  
  QStringList filelist = kpty->run(s);

  p->files = new QStringList(filelist);
}

// query files from a package file
void DEB::getFFileList(packageInfo *p)
{
  QString fn = p->getFilename();
  QString s = "dpkg --contents ";
  s += "'";	//krazy:exclude=duoblequote_chars
  s += fn;
  s += "'";	//krazy:exclude=duoblequote_chars

  QStringList filelist = kpty->run(s);

  int pt = -1;
  for ( QStringList::Iterator it = filelist.begin();
       it != filelist.end(); ++it ) {
//        kDebug() << "F=" << *it << "\n";
    if (pt < 0) {
      pt = (*it).lastIndexOf(' ');
    }
    (*it) = (*it).mid(pt + 1 + 1);
  }
  p->files = new QStringList(filelist);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
QStringList DEB::FindFile(const QString &name, bool searchAll)
{
  QString s;
  QStringList filelist;
  
  if (searchAll) {
    s = "apt-file search ";
    s += name;
    filelist = kpty->run(s);
    if (kpty->Result == 127) {
      kpackage->findialog->disableSearchAll();
    }
  }
  if (!searchAll || kpty->Result) {
    s = "dpkg -S ";
    s += name;
    filelist = kpty->run(s);
  }
  
  QString sp;
  QMutableListIterator<QString> it(filelist);
  while (it.hasNext()) {
    QString s = it.next();
    
    int p =  s.indexOf(": ");
    if( p !=-1 ) {
      it.setValue(s.replace(p, 2, "\t"));
    }
     
   if (s.isEmpty() || s == sp) {
      it.remove();
    }
    
   sp = s;
  }
  
  if (filelist.count() == 1) {
    QStringList::Iterator it = filelist.begin();
    if ((*it).indexOf("not found") >= 0) {
      filelist.erase(it);
    }
  }

  return filelist;
}

//////////////////////////////////////////////////////////////////////////////

void DEB::makeMenu(KActionCollection* act)
{
  kDebug() << "DEB::makeMenu\n";
  fileM = act->addAction( "apt_file");
  fileM->setText(i18n("&Apt-File Update"));
  connect(fileM, SIGNAL(triggered()), this, SLOT(fileS()));
  
  pkgInterface::makeMenu(act);
}

void DEB::setMenu(KActionCollection* act, bool enable)
{
  fileM->setEnabled(enable);
  
  pkgInterface::setMenu(act, enable);
}

void DEB::fileS()
{
  if (ifExe("apt-file") || !hostName.isEmpty()) {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    if (kprun->run(env + "apt-file update", "APT file update")) {
      kprun->exec();
    }
    QApplication::restoreOverrideCursor();
  } else {
    KpMsg("Error",i18n("The %1 program needs to be installed", QString("apt-file")), true);
  }
}

#include "debInterface.moc"
