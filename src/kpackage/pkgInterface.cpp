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

#include <QtGui/QApplication>
#include <QtCore/QTextStream>
#include <QtCore/QSet>
#include <QtCore/QFile>
#include <QtCore/QDir>

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kapplication.h>

#include "kpTerm.h"
#include "kpackage.h"
#include "pkgInterface.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
param::param(const QString &nameP, bool initP,  bool invertP, const QString &flagP)
{
  name = nameP;
  init = initP;
  invert = invertP;
  flag = flagP;
  flagA = "";
}

param::param(const QString &nameP, bool initP,  bool invertP,  const QString &flagP,  const QString &flagAP )
{
  name = nameP;
  init = initP;
  invert = invertP;
  flag = flagP;
  flagA = flagAP;

}

param::~param()
{
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
pkgInterface::pkgInterface( ) : QObject(), new_pict(), updated_pict()
{
  packageLoc = 0;

  hasProgram = ifExe("smart");
  if (!hasProgram && hostName.isEmpty()) {
    KpMsg("Error",i18n("Kpackage requires the SMART Package Manager to be installed in order to function"), true);
    exit(1);	// exit the application
  }
  
  DELMSG = i18n("'Delete this window to continue'");

  folder = SmallIcon("folder");
  markInst = UserIcon("tick");
  markUnInst = UserIcon("noball");
  bad_pict = UserIcon("dbad");

  pFile.setPattern("/([^/]*)\\s");
  deb.setPattern("(.+)_(.+)_.+\\.deb");
  rpm.setPattern("(.+)\\-(.+\\-.+)\\..+\\.rpm");
  slack.setPattern("(.+)\\-(.+\\-.+)\\-.+\\-\\.\\d+\\.tgz");
  fin.setPattern("^(.+)\\..+$");
  
  queryMsg = i18n("Querying SMART package list: ");
  procMsg = i18n("KPackage: Waiting on SMART");

  packagePattern = "*.deb *.rpm *.tgz";
  locatedialog = 0;


  hasRemote = false;
  defaultHandle = 1;
  hasSearchAll = false;
  
}
//////////////////////////////////////////////////////////////////////////////
pkgInterface::~pkgInterface()
{
}

//////////////////////////////////////////////////////////////////////////////

  void pkgInterface::makeMenu(KActionCollection* act)
{
  updateM = act->addAction( "update");
  updateM->setText(i18n("&Update"));
  updateM->setIcon(KIcon("update"));
  connect(updateM, SIGNAL(triggered()), this, SLOT(updateS()));

  upgradeM = act->addAction( "upgrade");
  upgradeM->setText(i18n("U&pgrade"));
  upgradeM->setIcon(KIcon("upgrade"));
  connect(upgradeM, SIGNAL(triggered()), this, SLOT(upgradeS()));

  fixupM =  act->addAction( "fixup" );
  fixupM->setText(i18n("&Fixup"));
  connect(fixupM, SIGNAL(triggered()), this, SLOT(fixupS()));
}

void pkgInterface::setMenu(KActionCollection*, bool enable)
{
  updateM->setEnabled(enable);
  upgradeM->setEnabled(enable);
  fixupM->setEnabled(enable);
}

//////////////////////////////////////////////////////////////////////////////
 bool pkgInterface::parseName(const QString &name, QString *n, QString *v) {
   QStringList res;
  
   if (deb.indexIn(name) >= 0) {
     *n = deb.cap(1);
     *v = deb.cap(2);
   } else {
     if (rpm.indexIn(name) >= 0) {
       *n = rpm.cap(1);
       *v = rpm.cap(2);
     } else {
       if (slack.indexIn(name) >= 0) {
	 *n = slack.cap(1);
	 *v = slack.cap(2);
       } else {
	 if (fin.indexIn(name) >= 0) {
	   *n = fin.cap(1);
	   *v = QString("");
	 }
       }
     }
   }
//         kDebug() << "n=" << *n << " v=" << *v << "\n";
   return true;
 }
 
 QStringList pkgInterface::depends(const QString &, int ) {return QStringList();}

bool pkgInterface::ifExe(QString exe) {
  if (!KGlobal::dirs()->findExe( exe ).isNull()) {
    return true;
  } else {
    kDebug() << "Program not found: " << exe << "\n";
    errExe = exe;
    return false;
  }
}

void pkgInterface::smerge(packageInfo *)
{ }

packageInfo *pkgInterface::collectPath(const QString &name)
{
  int p = name.lastIndexOf("/");
  if (p >= 0) {
    QString dir = name.left( p );
    QString fname = name.mid(p + 1);
//        kDebug() << "F=" << fname << " D=" << dir << "\n";
    return collectDir(fname,"",dir);
  } else {
    return collectDir(name,"","");
  }
}

packageInfo *pkgInterface::collectDir(const QString &name, const QString &size, const QString &dir)
{
    kDebug() << "collectDir " << name << " " << size << " " << dir << "\n";
    QString n,v;
    
    if (parseName(name, &n, &v)) {
     QMap<QString, QString> a;

     a.insert("group", "NEW");
     a.insert("name", n);
     a.insert("version", v);
     a.insert("file-size", size);
     a.insert("filename", name);
     a.insert("base", dir);

     packageInfo *i = new packageInfo(a,this);
     i->packageState = packageInfo::AVAILABLE;
    return i;
    }
  return 0;
}




//////////////////////////////////////////////////////////////////////////////

QString  pkgInterface::provMap(const QString &p)
{
  //  kDebug() << "provMap=>" << p;
  return p;
}
/////////////////////////////////////////////////////////////////////////////

QString pkgInterface::getDelimiter(short ptype)
{
  if (ptype == DEBt) {
    return "_";
  } 
    
  return "-";
}

//////////////////////////////////////////////////////////////////////////////
packageInfo *pkgInterface::collectInfo(QStringList &ln, pkgInterface *pkgInt)
{
  QMap<QString, QString> a;

  QString key, val;
  bool bad_install = false;
  bool haveName = false;
  short pType = lastType;

  QRegExp isDeb("\\.deb\\s");
  QRegExp isRpm("\\.deb\\s");
  QRegExp isSlack("\\.tgz\\s");
  
  for (int i = 0; i < ln.size(); ++i) {
  loop:
      
  int col = (ln[i]).indexOf(':');
  key = ((ln[i]).left(col)).toLower();
  if (key[0] == ' ') {
    key.remove(0,1);
  }
  val = (ln[i]).mid(col+2);
// kDebug() << key << "," << val << "\n";
  if (key == "name" || key == "package") {
    a.insert("name", val);
    haveName = true;
  } else if (key == "summary") {
    a.insert("summary", val);
  } else if (key == "description") {
    if (!a.contains("summary")) {
      a.insert("summary",val);
    }
    QString desc;
    while (++i < ln.size()) {
    (ln[i]).replace(QRegExp("<"),"&lt;");
      (ln[i]).replace(QRegExp(">"),"&gt;");
      if ((ln[i])[0] == ' ') {
	if ((ln[i])[1] == '*') {
	  ln[i] = "<br>" + ln[i];
	} else if ((ln[i])[1] == '.') {
	  ln[i] = "<p>";
	}
	desc += ln[i];
      } else {
	a.insert("description", desc);
	goto loop;
      }
    }
    a.insert("description", desc);
    break;
  } else if (key == "section") {
    a.insert("group",  val);
  } else if (key == "priority") {
    if (val  != "0") {
      a.insert("priority",  val);
    }
  } else if (key == "reference urls") {
    if (!val.isEmpty()) {
      a.insert("reference urls", val);
    }
  } else if (key == "flags") {
    if (!val.isEmpty()) {
      a.insert("flags", val);
    }
  } else if (key == "version") {
    a.insert("version", val);
  } else if (key == "installed size") {
    a.insert("size",  val);
  } else if (key == "installed-size") {
    a.insert("size",  val);
  } else if (key == "channels") {
    a.insert("channels",  val);
    if (val.contains("DEB")) {
      pType = DEBt;
    } else if (val.contains("RPM")) {
      pType = RPMt;
    } else if (val.contains("SLACK")) {
      pType = SLACKt;
    }
  } else if (key == "urls") {
    QString desc;
    
    while (++i < ln.size()) {
//	kDebug() << "id=" << i << " s= " << ln.size() << " " << ln[i] << "\n";
      if (isDeb.indexIn(ln[i]) >= 0) {  
	pType = DEBt;
      } else if (isRpm.indexIn(ln[i]) >= 0) {  
	pType = RPMt;
	pkgInt = kpinterface[RPMt];
      } else if (isSlack.indexIn(ln[i]) >= 0) {  
	pType = SLACKt;
      } 
    }
  } else {
    a.insert(key, val);
  }
   //     kDebug() << "C=" << key << "," << val <<"\n";
  }

  QString packName = a["name"] +  getDelimiter(pType) + a["version"];
//      kdDebug() << "CI " << packName << " " << pType << " C=" << kpackage->management->isInstalled.contains(packName) << " " << lastType << "\n";
  lastType = pType;
  
  if (pType == DEBt) {
    pkgInt = kpinterface[DEBt];
  } else if (pType == RPMt) {
    pkgInt = kpinterface[RPMt];
  } else if (!pkgInt) {
    pkgInt = this;
  }

  if (haveName) {
    packageInfo *i = new packageInfo(a,pkgInt);
    if (bad_install) {
      i->packageState = packageInfo::BAD_INSTALL;
    } else if (!kpackage->management->isInstalled.contains(packName)) {
      i->packageState = packageInfo::AVAILABLE;
//           kDebug() << "    A=" << i->packageState << " [" << packName << "]\n";
    } else {
      i->packageState = packageInfo::INSTALLED;
//       kDebug() << "    I=" << i->packageState << " [" << packName << "]\n";
    }
    i->packType = pType;
    i->fixup();
//      kDebug() << "==" << packName << "\n";
    return i;
  } else {
    return 0;
  }
}

//////////////////////////////////////////////////////////////////////////////
void pkgInterface::removeLastEmpty(QStringList &list) 
{    
  if (list.count() > 0) {
    QMutableListIterator<QString> i(list);
    i.toBack();
    if (i.peekPrevious().isEmpty()) {
      i.previous();
      i.remove();
    }
    if (i.peekPrevious().isEmpty()) {    
      i.previous();
      i.remove();
    }
  }
  // kDebug() << "R1" << list <<"\n"; 
}

QStringList  pkgInterface::listInstalls(const QStringList &packs, bool install, bool &)
{
  kDebug() << "pkgInterface::listInstalls\n";
  // Get list of dependencies
  
  QString match;
  QString s;
  if (install) {
    s = "smart install --dump-noversion ";
  } else {
    s = "smart remove --dump-noversion ";
  }

  foreach (const QString &pk, packs) {
    s +=  pk;
    s += ' ';
  }

  QStringList list = kpty->run(s, true);
// kDebug() << "L=" << list << "\n";
  if (!kpty->Result) {

    removeLastEmpty(list);
    QMutableListIterator<QString> i(list);
    i.toBack();
//    kDebug() << "L2=" << list << "\n";
  
    // Skip non-blank lines (wanted text)
    while (i.hasPrevious()) {
      if (!i.peekPrevious().isEmpty() && !i.peekPrevious().contains(' ')) {
       i.previous();
     } else {
        break;
      }
    }
   
   // Delete rest
    while (i.hasPrevious()) {
      i.previous();
      i.remove();
    }
//    kDebug() << "L3=" << list << "\n";
  
    return list;
  } else {
    return QStringList();
  }
}

//////////////////////////////////////////////////////////////////////////////

QStringList pkgInterface::getPackageState( const QStringList &packages)
{
  kDebug() << "pkgInterface::getPackageState\n";
  // Get state of packages
  
  QApplication::setOverrideCursor( Qt::WaitCursor );
  kpackage->setStatus(i18n("Querying package state"));
  
  QString match;
  QString s;
  s = "smart query --installed ";
 
  foreach (const QString &n, packages) {
    s = s + n + ' ';
  }
  
  QStringList list = kpty->run(s);
  // kDebug() << "s=" << s << "LS=" << list.count()  << "\n";
  // kDebug() << "Status=" << list << "\n";
 
  removeLastEmpty(list);
  
  QMutableListIterator<QString> i(list);
  
  i.toFront();
  
  // Remove non-blank lines forward (cache access info)
  while (i.hasNext()) {
    if (!i.peekNext().isEmpty()) {
      i.next();
      i.remove();
    } else {
      break;
    }
  }
  
  // Remove blank lines forward (cache access info)
  while (i.hasNext()) {
    if (i.peekNext().isEmpty()) {
      i.next();
      i.remove();
    } else {
      break;
    }
  }
  
//  kDebug() << "Status=" << list << "\n";
  QApplication::restoreOverrideCursor();
  return list;
}

void pkgInterface::getPackageDepends(packageInfo *p)
{
  // Get state of packages
  
  QApplication::setOverrideCursor( Qt::WaitCursor );
  kpackage->setStatus(i18n("Querying package dependencies"));
  
  QString match;
  QString s;
  s = "smart query --show-requires  --show-provides --show-requiredby ";
 
  s += p->getInfo("name") + getDelimiter(p->packType) + p->getInfo("version");
  
  QStringList list = kpty->run(s);
//          kDebug() << "s=" << s << "LS=" << list.count()  << "\n";
//          kDebug() << "Status=" << list << "\n";
 
  removeLastEmpty(list);
  
  QMutableListIterator<QString> i(list);
  i.toFront();
  
  // Remove non-blank lines forward (cache access info)
  while (i.hasNext()) {
    if (!i.peekNext().isEmpty()) {
      i.next();
      i.remove();
    } else {
      break;
    }
  }
  
  // Remove blank lines forward (cache access info)
  while (i.hasNext()) {
    if (i.peekNext().isEmpty()) {
      i.next();
      i.remove();
    } else {
      break;
    }
  }
  
//  kDebug() << "Status=" << list << "\n";
  p->depends = new QStringList(list);
  QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////

void pkgInterface::listPackages(QList<packageInfo *> &pki)
{ 
  listInstalledPackages(pki);
  if (hostName.isEmpty()) {
    listPackagesFile(pki);
  } else {
    listRemotePackages(pki);
  }
}

void pkgInterface::listInstalledPackages(QList<packageInfo *> &)
{
// Get list of installed packages
  QStringList  plist;
 
  kpackage->setStatus(i18n("Querying installed package list"));

  QString cmd = "smart query --installed";

  QStringList list = kpty->run(cmd);
//  kDebug() << "s=" << cmd << "LS=" << list.count()  << "\n";
//  kDebug() << "Status=" << list << "\n";
  
 
  if (kpty->Result == 127) { // smart command not found
    QApplication::restoreOverrideCursor();
    QString sm = i18n("'smart' package manager is required");
    if (list.count() == 0) {
      KpMsgE(sm, true);
    } else {
      KMessageBox::detailedSorry(kpkg,sm,list[0]);
    }
    kpackage->fileQuit();
  } else if (kpty->Result == 1) { // smart not initialised
    list = kpty->run(cmd,true,true);
    kpackage->displayMsg = i18n("Press 'Update' to display available packages ");
  }
  
  if (list.count() > 0) {

    QString s;

    for ( QStringList::const_iterator it = list.constBegin();  it != list.constEnd(); ++it ) {
      kpackage->management->isInstalled.insert(*it);
    }
  }
  
  QMutableListIterator<QString> i(list);
  
  // Remove non-blank lines forward (cache access info)
  while (i.hasNext()) {
    if (!i.peekNext().isEmpty()) {
      i.next();
      i.remove();
    } else {
      break;
    }
  }
}

void pkgInterface::listPackagesFile(QList<packageInfo *> &pki)
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  kpackage->setStatus(i18n("Generating package information"));
  
  QString tempFile = KStandardDirs::locateLocal("tmp","smart.list");
  
  QString cmd = "/bin/sh -c 'smart info --urls >" + tempFile + "'";	//krazy:exclude=duoblequote_chars
  QStringList list = kpty->run(cmd);
    
  kpackage->setStatus(i18n("Processing package information"));
  
  QStringList lines;
  QFile file( tempFile );
  if ( file.open( QIODevice::ReadOnly ) ) {
    QTextStream stream( &file );
    QString line;
    while ( !stream.atEnd() ) {
      line = stream.readLine(); // line of text excluding '\n'
      if (!line.isEmpty()) {
//	kDebug() << line << "\n";
	lines += line;
      } else {  
//	kDebug() << line << "-----------------------------\n";
	packageInfo *p = 0;
	p = collectInfo(lines);
	if (p) {
	  if (!p->pkgInsert(pki,  p->packageState == packageInfo::INSTALLED)) {
	    delete p;
	  }
	  lines.clear();
	}
      }
    }
    file.close();
  } else {
  }
  kpackage->setStatus("");
  QApplication::restoreOverrideCursor();
}


void pkgInterface::listRemotePackages(QList<packageInfo *> &)
{
  pCnt = 0;
  
  kDebug() << "pkgInterface::listRemotePackages\n";
  
  QStringList  plist;

  kpackage->setStatus(i18n("Querying SMART package list: %1", hostName));
  kpackage->setPercent(20);
  
  
  connect(kpty, SIGNAL(textLine(const QString &)), this,
	  SLOT(textIn(const QString &)));

 
 QString cmd = "smart info --urls | cat";
 // QString cmd = "cat /home/toivo/s.info";

  QStringList list = kpty->run(cmd);
  kpackage->setStatus(i18n("Processing SMART package list: %1", hostName));
  //  kDebug() << "P=" << list.count() <<"\n";
  
  disconnect(kpty, SIGNAL(textLine(const QString &)), this,
	     SLOT(textIn(const QString &)));

 
  kpackage->setStatus(i18n("SMART"));
  kpackage->setPercent(100);
}

void pkgInterface::textIn(const QString &s)
{
  int PMAX = 30000;
  QString ss = s;
  ss.chop(1);
  // kDebug() << " textIn=[" << ss << "]\n";
 
  packageInfo *p = 0;
   
  if (ss.isEmpty()) {
//    kDebug() << "===" << packList << "\n";
    pCnt++;
    if (pCnt % 250 == 0) {
      kpackage->setPercent(20 + (pCnt * 80) / PMAX);
    }
      
    p = collectInfo(packList);
    if (p) {
      if (!p->pkgInsert(kpackage->management->allPackages, p->packageState == packageInfo::INSTALLED)) {
	delete p;
      }
      kpty->listClear();
      packList.clear();
      kpty->listClear();
    }
  } else {
    packList << ss;
  }
}

/////////////////////////////////////////////////////////////////////////////

void pkgInterface::getFileList(packageInfo *p)
{
  if (p->packageState == packageInfo::INSTALLED) {
    getIFileList(p);
  } else {
    if (!p->getFilename().isEmpty()) {
      getFFileList(p);
    }
  }
}


void pkgInterface::getIFileList(packageInfo *p)
{
    QApplication::setOverrideCursor( Qt::WaitCursor );
  QString pn = "smart info --paths " + p->getInfo("name") + getDelimiter(p->packType) + p->getInfo("version");
  
  
  QStringList res = kpty->run(pn);
//  kDebug() << "Status1=" << res << "\n";  

  QMutableListIterator<QString> i(res);
  i.toBack();
  
  // remove blank lines
  while (i.hasPrevious()) {
    if (i.peekPrevious().isEmpty()) {
      i.previous();
      i.remove();
    } else {
      break;
    }
  }
  
  // Skip lines starting with space
  while (i.hasPrevious()) {
    if (i.peekPrevious().left(1) == " ") {
      i.previous().remove(0,1);
    } else {
//      i.remove();
      break;
    }
  }
   
   // Delete rest
  while (i.hasPrevious()) {
    i.previous();
    i.remove();
  }

  p->files = new QStringList(res);
  QApplication::restoreOverrideCursor();
// kDebug() << "Status=" << res << "\n";  
}

//////////////////////////////////////////////////////////////////////////////
QStringList pkgInterface::verify(packageInfo *, const QStringList &files)
{
  int  p = 0;
  uint c = 0;
  QStringList errorlist;
  QDir d;

  if (hostName.isEmpty()) {

    uint step = (files.count() / 100) + 1;

    kpackage->setStatus(i18n("Verifying"));
    kpackage->setPercent(0);

    for( QStringList::ConstIterator it = files.begin();
	 it != files.end();
	 it++)
      {
	// Update the status progress
	c++;
	if(c > step) {
	  c=0; p++;
	  kpackage->setPercent(p);
	}

	if (!d.exists(*it)) {
	  errorlist.append(*it);
	}
      }

    kpackage->setPercent(100);
  }
  return errorlist;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

QString pkgInterface::install(int installFlags, QList<packageInfo *> p, bool &test)
{
  QString packs = "";
  
  foreach (packageInfo *i, p) {
    QString fname = i->fetchFilename();
    if (!fname.isEmpty()) {
      packs += fname;
      packs += ' ';
    }
  }
  return doInstall(installFlags, packs, test);
}


QString pkgInterface::install(int installFlags, packageInfo *p, bool &test)
{
  QString fname = p->fetchFilename();

  return doInstall(installFlags, fname, test);
}



QString pkgInterface::doInstall(int installFlags, const QString &packs, bool &test)
{
  QString s = env + "smart install -y  ";
//  QString s = env + "dpkg -i  ";
  s += setOptions(installFlags, paramsInst);
  s +=  packs;

  kDebug() << "iCMD=" << s << "\n";

  if ((installFlags>>0 & 1) || (installFlags>>5 & 1))
    test = true;

  return s;
}

//////////////////////////////////////////////////////////////////////////////

QString pkgInterface::uninstall(int uninstallFlags, QList<packageInfo *> p, bool &test)
{
  QString packs;

//  for (i = p->first(); i!= 0; i = p->next())  {
  foreach (packageInfo *i, p) {
    packs += i->getInfo("name");
    packs += ' ';
  }
  return doUninstall( uninstallFlags, packs, test);
}

QString pkgInterface::uninstall(int uninstallFlags, packageInfo *p, bool &test)
{
  QString packs( p->getInfo("name"));

  return doUninstall(uninstallFlags, packs, test);
}

QString pkgInterface::doUninstall(int uninstallFlags, const QString &packs, bool &test)
{
  QString s = env + "smart remove -y ";
  s += setOptions(uninstallFlags, paramsUninst);
  s +=  packs;

  kDebug() << "uCMD=" << s << "\n";

  if (uninstallFlags>>2 & 1)
    test = true;

  return s;
}

//////////////////////////////////////////////////////////////////////////////

void pkgInterface::updateS()
{
  if (kprun->run("smart update", "SMART update")) {
    if (kprun->exec())
      kpackage->management->collectData(true);
  }
}

void pkgInterface::upgradeS()
{
  if (kprun->run(env + "smart upgrade -y", "SMART upgrade")) {
    if (kprun->exec())
      kpackage->management->collectData(true);
  }
}

void pkgInterface::fixupS()
{
  if (kprun->run(env + "smart fix -y", "SMART fix")) {
    if (kprun->exec())
      kpackage->management->collectData(true);
  }
}


//////////////////////////////////////////////////////////////////////////////

bool pkgInterface::depTab(packageInfo *p) {
  if (p->packageState == packageInfo::INSTALLED) {
    return true;
  }
  return false;
}

QString pkgInterface::setOptions(int flags, QList<param *> &params)
{
  int i;
  QString s;

  i = 0;
//  for ( p=params.first(); p != 0; p=params.next(), i++ ) {
  foreach (param *p, params) {
    if ((flags>>i & 1) ^ p->invert) {
      s += p->flag + ' ';
    } else {
      if (!p->flagA.isEmpty())
	s += p->flagA + ' ';
    }
    i++;
  }
  return s;
}

///////////////////////////////////////////////////////////////////////////////////

packageInfo *pkgInterface::getPackageInfo(const QString &) {
  return 0;
}

void pkgInterface::getFFileList(packageInfo *) {
}

QStringList pkgInterface::FindFile(const QString &, bool ) {
   QStringList s;
   return s;
}

 QStringList pkgInterface::getChangeLog(packageInfo *) {
   QStringList s;
   return s;
 }
   
 bool pkgInterface::filesTab(packageInfo *) {
   return false;
 }
 
 bool pkgInterface::changeTab(packageInfo *) {
   return false;
 }
 
 packageInfo *pkgInterface::getFPackageInfo(const QString &) {
   return 0;
 }
 
 


#include "pkgInterface.moc"
