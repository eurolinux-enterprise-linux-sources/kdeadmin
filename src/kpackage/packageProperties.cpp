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

#include <stdio.h>
#include <kdebug.h>

#include <ktoolinvocation.h>
#include "kpackage.h"
#include "packageProperties.h"
#include "pkgInterface.h"
#include "managementWidget.h"

packagePropertiesWidget::packagePropertiesWidget
  (QWidget *parent)
  : KTextBrowser(parent)
{
  QStringList  pList();
  QStringList  cList();

  //  hide();
  package = 0;
  QPalette p;
  p.setColor(backgroundRole(), palette().base().color());
  setPalette(p);
  initTranslate();
}

packagePropertiesWidget::~packagePropertiesWidget()
{
}

void packagePropertiesWidget::iList(const QString &txt, const QString &itxt)
{
  trl[txt] = itxt;
  pList.append(txt);
}


void packagePropertiesWidget::initTranslate()
{

  trl["name"] = i18n("name");

  iList("summary", i18n("summary"));
  iList("version", i18n("version"));
  iList("old-version", i18n("old-version"));
  iList("status", i18n("status"));
  iList("group", i18n("group"));
  iList("size", i18n("size"));
  iList("file-size", i18n("file-size"));
  iList("description", i18n("description"));
  iList("url", i18n("url")); 
  iList("architecture", i18n("architecture"));

  iList("unsatisfied dependencies", i18n("unsatisfied dependencies"));
  iList("pre-depends", i18n("pre-depends"));
  iList("dependencies", i18n("dependencies"));
  iList("depends", i18n("depends"));
  iList("conflicts", i18n("conflicts"));
  iList("provides", i18n("provides"));
  iList("recommends", i18n("recommends"));
  iList("replaces", i18n("replaces"));
  iList("suggests", i18n("suggests"));
  iList("priority", i18n("priority"));

  iList("essential", i18n("essential"));
  iList("install time", i18n("install time"));
  iList("config-version", i18n("config-version"));
  iList("distribution", i18n("distribution"));
  iList("vendor", i18n("vendor"));
  iList("maintainer", i18n("maintainer"));
  iList("packager", i18n("packager"));
  iList("source", i18n("source"));
  iList("build-time", i18n("build-time"));
  iList("build-host", i18n("build-host"));
  iList("base", i18n("base"));
  iList("filename", i18n("filename"));
  iList("serial", i18n("serial"));
  iList("flags", i18n("flags"));
  iList("channels", i18n("channels"));
  iList("reference urls", i18n("reference urls"));

  iList("also in", i18n("also in"));
  iList("run depends", i18n("run depends"));
  iList("build depends", i18n("build depends"));
  iList("available as", i18n("available as"));
}

void packagePropertiesWidget::changePackage(packageInfo *p)
{

  QStringList tList;

  package = p;
  cList.clear();

  if (p) {
    // properties in the package that exist in the translate list added in that order
    foreach (const QString &s, pList) {
      if (p->info.contains(s)) {
	cList.append(s);
      }
    }
      
    // append other properties in the package to the end
    QMapIterator<QString, QString> it(p->info);
    while (it.hasNext()) {
      it.next();
      if (!trl.contains(it.key())) {
	tList << it.key();
      }
    }
    cList = cList + tList;
    
    QRegExp http("(http://[\\w.~\\/\?#%<]+[\\w~\\/\\?#%<])");
    
    stmp = "";
    stmp += "<html><head></head><body>";
    stmp += "<h1 style='font-family: serif;'>";
    stmp += p->getInfo("name");
    stmp += "</h1><hr/>";
    stmp += "<table style='width: 100%; border: none; border-spacing: 8px;'>";
    for ( QStringList::const_iterator s = cList.constBegin();
	  s != cList.constEnd();
	  ++s) {
      QString pr = trl[*s];
      QString propName;
      if(!pr.isEmpty()) {
	propName = pr;
      } else {
	propName = *s;
      }
      stmp += "<tr>";
      stmp += "<td style='vertical-align: top; font-weight: bold'>";
      stmp += propName;
      stmp += "</td><td>";
      QString f = p->getInfo(*s);
      if (*s == "maintainer" || *s == "packager") {
		f.replace(QRegExp("<"),"&lt;");
		f.replace(QRegExp(">"),"&gt;");
      }
      if (*s == "filename") {
	int p = f.lastIndexOf('/');
	if (p >= 0) {
	  f.insert(p+1,"\n");
	};
	stmp += f;
      } else if (*s == "depends" || *s == "conflicts"  ||
		  *s == "replaces" ||
		 *s == "suggests"  || *s == "recommends" ||
		 *s == "pre-depends" || *s == "unsatisfied dependencies") {
	depends(f);
      } else if (*s == "description") {
	f.replace(QRegExp("(http://[\\w.~\\/\?#%-]+[\\w~\\/\\?#%-])"),"<a href=\"\\1\">\\1</a>");
	stmp += f;
     } else if (*s == "url") {
        if (f.right(1) == " ") f.remove(f.length()-1, 1);
        if  (f.startsWith("http:") || f.startsWith("ftp:"))  /*if (!(f == "(none)")) */
        stmp += "<a href=\"" + f +"\">" + f + "</a>";
        else stmp += i18n("none");
      } else {
	stmp += f;
      }
      stmp += "</td>";
      stmp += "</tr>";
    }
    stmp += "</table>";
    stmp += "</body></html>";
    //     setText(stmp);
         setHtml(stmp);
  }
  update();
}

void  packagePropertiesWidget::depends(const QString &f) {
  //  printf("d=%s\n",f.data());

  int i = 0;
  const QStringList list = f.split(',');
  for ( QStringList::const_iterator it = list.begin(); it != list.end(); ++it ) {
    if (i++ > 0)
      stmp += ',';
    dor((*it));
  }
}

void  packagePropertiesWidget::dor(const QString &f) {
  //  printf("o=%s\n",f.data());

  int i = 0;
  const QStringList list = f.split('|');
  for ( QStringList::const_iterator it = list.begin(); it != list.end(); ++it ) {
    if (i++ > 0)
      stmp += '|';
    delement((*it));
  }
}

void  packagePropertiesWidget::delement(const QString &f) {
  int n = f.indexOf('(');
  if (n < 0) {
    n = f.length();
  }

  QString u = f.left(n);
  QString uf =  package->interface->provMap(u.trimmed());

  bool inst = false, uninst = false;
  if (kpackage->management->dirInstPackages.value(uf)) {
    inst = true;
  } else if (kpackage->management->dirUninstPackages.value(uf)) {
    uninst = true;
  }

  if (uninst)
    stmp += "<i>";
  if (inst||uninst) {
    stmp += "<a href=\"";
    stmp += uf;
    stmp += "\">";
    stmp += u;
    stmp += "</a>";
  } else {
    stmp += u;
  }
  if (uninst)
    stmp += "</i>";
  if (n < (signed)f.length())
    stmp += f.mid(n).replace(QRegExp("<"),"&lt;");
}

void  packagePropertiesWidget::setSource(const QString &name) {
  QString s = name;

  if (s.startsWith("http:") || s.startsWith("ftp:"))
  {
		  KToolInvocation::invokeBrowser( s );
    return;
  }
  
  if (s.startsWith("file:")) {
    s = s.mid(5);
  }
  else if (s.at(1) == '/') {
    s = s.mid(1);
  }
  
  QString ind = s;
  packageInfo *p = kpackage->management->dirInstPackages.value(ind);
  if (p) {
     kpackage->management->treeList->changePack(p->getItem(), package->getItem() != 0);
  } else {
    p = kpackage->management->dirUninstPackages.value(ind);
    if (p) {
       kpackage->management->treeList->changePack(p->getItem(), package->getItem() != 0);
    } else {
//         kDebug() << "Nfound=" << ind;
    }
  }
}
#include "packageProperties.moc"
