/*
** Copyright (C) 1999,2000 Toivo Pedaste <toivo@ucs.uwa.edu.au>
**
// Author: Damyan Pepper
// Author: Toivo Pedaste
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

// Standard headers
#include <stdlib.h>

#include <QtCore/QRegExp>
#include <QtCore/QDir>
//Added by qt3to4:
#include <QtGui/QPixmap>
#include <ctype.h>

// KDE headers
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

// kpackage headers
#include "kpackage.h"
#include "kplview.h"
#include "packageInfo.h"
#include "pkgInterface.h"
#include "managementWidget.h"
#include "utils.h"

// Global pixmap for
QPixmap *pict = NULL;

//////////////////////////////////////////////////////////////////////////////
// Constructor -- get the pixmap
packageInfo::packageInfo(QMap<QString, QString> _info, pkgInterface *type)
{
  interface = type;
  info = _info;

  item = NULL;
  packageState = UNSET;
  updated = false;
  url = QString();	// could have been url.clear(), but maybe this is more unisys with others
  depends = 0;
  files = 0;
}

// Another constructor, for a packge with a url
packageInfo::packageInfo(QMap<QString, QString> _info, const QString &_url)
{
  info = _info;
  url = _url;
  item = NULL;
}

packageInfo::~packageInfo()
{
}

// Return a property
QString packageInfo::getInfo(const QString &property)
{
  if (info.contains(property)) {
    QString result = info[property];
    if (result.isEmpty()) {
      return QString();
    } else {
      return result;
    }
  } else {
    return QString();
  }
}

// Check for existence of a property
bool packageInfo::hasInfo(const QString &property)
{
  if (info.contains(property)) {
    QString s = info[property];
    if (s.isEmpty()) 
      return false;
    else 
      return true;
  } else {
    return false;
  }
}

// Delete a property
void packageInfo::rmInfo(const QString &property)
{
  if (info.contains(property)) {
    info.remove(property);
  }
}

// Initialize fields if missing
void packageInfo::fixup()
{
  if (!info.contains("name")) {
    QString q;
    q.setNum((long)this);
    info.insert("name", q);
  }

  if (!info.contains("group")) {
    info.insert("group", i18n("OTHER"));
    kDebug() << "Package " << info["name"] << " has no group set.\n";
  }

  if (!info.contains("version")) {
    info.insert("version", "");
  }
}

// Set the file name
void packageInfo::setFilename(const QString &f)
{
  url = f;
}

// Get the url
QString packageInfo::getUrl()
{
  if (url.isEmpty()) {
    if (hasInfo("base") && hasInfo("filename")) {
      url = getInfo("base") + '/' + getInfo("filename");
    }
  }
  return url;
}

QString packageInfo::fetchFilename()
{
  QString f = getFilename();

  if (!f.isEmpty()) {
    return f;
  } else {
    QString aurl = getUrl();
    if (!aurl.isEmpty()) {
      return kpackage->fetchNetFile(aurl);
    } else {
      return getInfo("name");
    }
  }
}

bool packageInfo::isFileLocal()
{
  QString aurl = getUrl();
  if (!aurl.isEmpty()) {
    return KPACKAGE::isFileLocal(aurl);
  } 
  return false;
}

bool packageInfo::isInstallable()
{
  if (packageState != packageInfo::INSTALLED /* && !getInfo("filename").isNull()*/ ) 
    return true;
  else
    return false;
}

QString packageInfo::getFilename()
{
  QString cn = "";
  QString aurl = getUrl();
  if (!aurl.isEmpty()) {
    return KPACKAGE::getFileName(aurl,cn);
  } else {
    return "";
  }
}

int packageInfo::getDigElement(const QString &str, int *pos)
  // Extract the next element from the string
  // All digits
{
	QString s = str;

  if (*pos < 0)
    return -1;

  s = s.mid(*pos);
  if (s.isEmpty())
    return -1;

  QRegExp ndig("[^0-9]");

  int nf = 0;
  int val = 0;

  if ((s[0] >= '0') && (s[0] <= '9')) {
    nf = s.indexOf(ndig);
    if (nf >= 0) {
      val = s.left(nf).toInt();
    } else {
      val = s.toInt();
      nf = s.length();
    }
  }

  //  printf("n=%s %d %d\n",s.mid(nf,999).data(),nf,val);
  *pos += nf;
  return val;
}

QString packageInfo::getNdigElement(const QString &string, int *pos)
  // Extract the next element from the string
  // All  all non-digits
{
  QString s(string);

  if (*pos < 0)
    return QString();

  s = s.mid(*pos);
  if (s.isEmpty())
    return QString();

  QString str;
  int nf = 0;

  QRegExp idig("[0-9]");

  if ((s[0] < '0') || (s[0] > '9') ) {
    nf = s.indexOf(idig);
    if (nf <  0)
      nf = s.length();
    str = s.left(nf);
    for (int i = 0; i < str.length() ; i++) {
      // Strange Debian package sorting magic
      if (!str[i].isLetter()) {
	char t = str[i].toLatin1();
	t += 128;
	str[i] = t;
      }
    }
  }
  *pos += nf;
  return str;
}


int packageInfo::pnewer(const QString &s, const QString &sp)
{
  int ns = 0, nsp = 0, vs, vsp;

//         kDebug() << "S=" << s  << " SP=" << sp  << "\n";
  while (true) {
    vs = getDigElement(s,&ns);
    vsp = getDigElement(sp,&nsp);
//        kDebug() << "s=" << ns << " " << vs << " sp=" << nsp << " " << vsp << "\n";
    if (vs < 0 && vsp < 0)
      return 0;
    if (vs < 0 && vsp < 0)
      return 1;
    if (vs < 0 && vsp < 0)
      return -1;
    if (vsp > vs)
      return 1;
    else if (vs > vsp)
      return -1;
//    kDebug() << "SS=" << s.mid(ns) << " SSP=" << sp.mid(nsp) << "\n";

    // a element starting with ~ isn't newer that and empty element
    QString se = s.mid(ns);
    QString sep = sp.mid(nsp);
    if (se.isEmpty() && sep[0] == '~')
      return -1;
    if (se[0] == '~' && sep.isEmpty())
      return 1;
    
    QString svs = getNdigElement(s,&ns);
    QString svsp = getNdigElement(sp,&nsp);
//        kDebug() << "vs=" << ns << " " << svs << " sp=" << nsp << " " << svsp << "\n";
    if (svs.isEmpty() && svsp.isEmpty())
      return 0;
    if (svs.isEmpty() && !svsp.isEmpty())
      return 1;
    if (!svs.isEmpty() && svsp.isEmpty())
      return -1;

    if (svsp.isNull()) { // Allow for QT strangeness comparing null string
      svsp = "";
    }
    if (svs.isNull()) {
      svs = "";
    }
    int n = svsp.compare(svs);
//        kDebug() << "svsp=" << svsp << "=" << svsp.length() <<  " svs=" << svs << "=" <<svs.length() <<  " n=" << n << "\n";
    if (n != 0)
      return n;
  }
}

static bool split(QString orig, char separator, QString &first, QString &second)
{
  int pos = orig.indexOf(separator);
  if (pos > 0) {
    first = orig.mid(0,pos);
    second = orig.mid(pos+1);
    return true;
  }
  return false;
}

int packageInfo::newer(packageInfo *p)
{
  QString mySerial;  // Serial number of this package
  QString myVersion; // Version of this package
  QString myRelease; // Release of this package

//  QString sn = getInfo("name");
//  kDebug() << "packageInfo::newer " << sn << "\n";
  
// Version of this package
  QString s = getInfo("version");

  (void) split(s, ':', mySerial, s);
  if (!split(s, '-', myVersion, myRelease))
  {
    myVersion = s;
  }

//  kDebug() << "my=" << mySerial << "=" << myVersion << "=" << myRelease << "\n";
// Version of other package
  QString hisSerial;  // Serial number of the other package
  QString hisVersion; // Version of the other package
  QString hisRelease; // Release of the other package

  s = p->getInfo("version");
  if (p->hasInfo("release")) {
    s = s + '-' + p->getInfo("release");
  }
  if (p->hasInfo("serial")) {
    s = p->getInfo("serial") + ':' + s;
  }

  (void) split(s, ':', hisSerial, s);
  if (!split(s, '-', hisVersion, hisRelease))
  {
    hisVersion = s;
  }
//  kDebug() << "his=" << hisSerial << "=" << hisVersion << "=" << hisRelease << "\n";
  
  int n =  pnewer(mySerial,hisSerial);
  if (n) {
//    kDebug() << "W1=" << n << "mySerial=" <<  mySerial << " hisSerial=" <<  hisSerial <<"\n";
    return n;
  } else {
    n = pnewer(myVersion,hisVersion);
    if (n) {
//      kDebug() << "W2=" << n << "myVersion=" <<  myVersion << " hisVersion=" <<  hisVersion <<"\n";
      return n;
    } else {
      n = pnewer(myRelease,hisRelease);
//      kDebug() << "W3=" << n << "myRelease=" <<  myRelease << " hisRelease=" <<  hisRelease <<"\n";
      return n;
    }
  }
}

bool packageInfo::display(int treeType)
{
  switch (treeType) {
  case managementWidget::INSTALLED:
    if (packageState == INSTALLED || packageState == BAD_INSTALL)
      return true;
    break;
  case managementWidget::NEW:
    if (getInfo("flags") == "new")
      return true;
    break;
  case managementWidget::UPDATES:
    if  ((packageState == UPDATED) )
      return true;
    break;
  case managementWidget::AVAILABLE:
    if  ((packageState == UPDATED) || (packageState == NEW))
	return true;
    break;
  case managementWidget::ALL:
      return true;
    break;
  };
  return false;
}

//////////////////////////////////////////////////////////////////////
// Place the package in a QListView

KpTreeListItem *packageInfo::place(KpTreeList *tree, bool insertI)
{
  KpTreeListItem *search = tree->firstChild(), *parent=NULL, *child=NULL;
  QString qtmp, tmp;
  bool doit = false;

  doit = true;
  if (packageState == NOLIST || packageState == HIDDEN)
    doit = false;

  if (doit) {
    qtmp = getInfo("group");
    int cnt = 0;

    QStringList list = qtmp.split("/");
//                kDebug() << "Q=" << qtmp << "\n";
    for (int i = 0; i < list.size(); ++i) {
      KpTreeListItem *group;
      if ( search && (group=findGroup(list[i], search)) ) {
	 parent = group;
	 parent->setOpen(true);
	 search = group->firstChild();
      } else {
	if (parent) {
	  group = new KpTreeListItem(parent, 0, interface->folder, list[i]);
	} else {
	  group = new KpTreeListItem(tree, 0, interface->folder, list[i]);
	}
	parent = group;
	parent->setOpen(true);
	search = NULL;
      }
     cnt++;
    }

    tmp = *info.find("name");

    if(item)
      delete item;

    QString sz = "";
    if (hasInfo("size")) {
      sz = info["size"].trimmed();
      sz = sz.rightJustified(6,' ');
    }

    QString ver = "";
    if (hasInfo("version")) {
      ver = info["version"];
    }

    QString over = "";
    if (hasInfo("old-version")) {
      over = info["old-version"];
    }
    QString summary = "";
    if (hasInfo("summary")) {
      summary = info["summary"];
    }
       
//    kDebug() << "N=" << tmp << " V=" << ver << " O=" << over << "\n";

    QPixmap pic;
    if (packageState == BAD_INSTALL) {
      pic = interface->bad_pict;
    } else if (packageState == UPDATED) {
      pic = interface->updated_pict;
    } else if (packageState == NEW) {
      pic = interface->new_pict;
    } else if (packageState == INSTALLED) {
      pic = interface->pict;
    } else {
      pic = interface->pict;
    }

    if (child) {
      item =  new KpTreeListItem(child, this, pic, tmp, "", summary, sz, ver, over);
    } else {
      item = new KpTreeListItem(parent, this, pic, tmp, "", summary, sz, ver, over);
    }

    if (insertI) {
       parent->setOpen(true);
    } else {
       parent->setOpen(false);
    }

    return item;
  } else {
    return 0;
  }
}

//////////////////////////////////////////////////////////////////////

// Get the QListViewItem
KpTreeListItem *packageInfo::getItem()
{
  return item;
}

void packageInfo::deleteItem()
{
  delete item;
  item = 0;
}

//////////////////////////////////////////////////////////////////////////////
bool packageInfo::smerge( const QString &exp) {

  QString pname = getInfo("name") + exp;

  packageInfo *pi = kpackage->management->dirInfoPackages.value(pname);
  if (pi) {
    QMap<QString,QString>::Iterator it;

    for ( it = pi->info.begin(); it != pi->info.end(); ++it ) {
      if (!(it.key() == "size" && !info["size"].isEmpty()) ||
	  !(it.key() == "file-size"  && !info["file-size"].isEmpty())) {
	info.insert(it.key(), it.value());
      }
      ++it;
    }
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
void packageInfo::pkgFileIns(const QString &)
{
 // info.insert("filename", fileName);
 // info.insert("base", "/");

  if (pkgInsert(kpackage->management->allPackages, false)) {
    packageState = packageInfo::NEW;
    place(kpackage->management->treeList,true);

    QString pname = getInfo("name");
    kpackage->management->dirUninstPackages.insert(pname,this);
  }
}

void packageInfo::pkgUpdate(packageInfo *pc, packageInfo *pi, bool installed, bool hidden)
{
  if (pc->packageState != BAD_INSTALL) {
    if (installed)
      pc->packageState = INSTALLED;
    else if (pi) { // installed version exists
      if (hidden) {
	pc->packageState = HIDDEN;
      } else {
	QString version = pi->getInfo("version");
	if (version.isEmpty()) {
	  if (pi->packageState == NOLIST)
	    pc->packageState = NEW;
	  else
	    pc->packageState = UPDATED;
	} else {
	  pc->packageState = UPDATED;
	  if (pi->hasInfo("old-version")) {
	    pc->info.insert("old-version",
			pi->getInfo("old-version"));
	  } else {
	    pc->info.insert("old-version",version);
	  }
	  QString group = pc->getInfo("group");
	  if (group == "NEW") {
	    if (pi->hasInfo("group")) {
	      pc->info.replace("group",
			   pi->getInfo("group") );
	    }
	  }
	}
      }
    } else
      pc->packageState = NEW;
  }
}

//////////////////////////////////////////////////////////////////////////////
bool packageInfo::pkgInsert(QList<packageInfo *> &pki,
			 bool installed, bool infoPackage)
{
  QString pname = getInfo("name");
//  kDebug() << "U=" << pname << " I=" << installed << " IP=" << infoPackage << "\n";

  bool shouldUpdate = true;
  bool hidden = false;

  packageInfo *pi = kpackage->management->dirInstPackages.value(pname);
  if (pi) { // installed version exists
    if ((pi->packageState != BAD_INSTALL)
	&& (pi->packageState != NOLIST))  {
//      kDebug() << "U1=" << pname << "\n";
      
      if (newer(pi) >= 0) {
//        kDebug() << "H=" << pname << "\n";
	hidden = true;
      }
    }
  }

  packageInfo *pu = kpackage->management->dirUninstPackages.value(pname);
  if (pu) { // available version exists
    if ((pu->packageState != BAD_INSTALL)
	&& (pu->packageState != NOLIST))  {
//       kDebug() << "U2=" << pname << "\n";
      if (newer(pu) > 0) {
//        kDebug() << "H1=" << pname << "\n";
	if (installed) {
	  pu->packageState = UPDATED;
	  shouldUpdate = true;
	  pkgUpdate(pu,this,false,false);
	} else {
	  shouldUpdate = false;
	}
      } else {
//          kDebug() << "H2=" << pname << "\n";
	  kpackage->management->dirUninstPackages.take(*(pu->info.find("name")));
	  pki.removeAll(pu);
      }
    }
  } 

  if (!hasInfo("version")) {
    shouldUpdate = true;
  }
  
//  kDebug() << "shouldUpdate=" << shouldUpdate << " hidden=" << hidden << "\n";
  if (shouldUpdate) {
    pkgUpdate(this,pi,installed,hidden);
    
    pki.append(this);
//    kDebug() << "Inst=" << installed << " Info=" << infoPackage << " hidden=" << hidden << "\n";
    
    if (installed && !hidden) {
      if (infoPackage)
	kpackage->management->dirInfoPackages.insert(pname,this);
      else
	kpackage->management->dirInstPackages.insert(pname,this);
    } else
      kpackage->management->dirUninstPackages.insert(pname,this);
    return true;
  } else {
    return false;
  }
}

void packageInfo::updateItem()
{
  if (item) {
    item->setText(0,getInfo("name"));
    item->setText(2,getInfo("summary"));
    item->setText(3,getInfo("size"));
    item->setText(4,getInfo("version"));
  }
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

