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

#include <kdebug.h>

#include <ktoolinvocation.h>
#include "kpackage.h"
#include "packageDepends.h"
#include "pkgInterface.h"
#include "managementWidget.h"

packageDepends::packageDepends
    (QWidget *parent)
  : KTextBrowser(parent)
{
  QPalette p;
  p.setColor(backgroundRole(), palette().base().color());
  setPalette(p);
}

packageDepends::~packageDepends()
{
}

void packageDepends::changePackage(packageInfo *p)
{

  QStringList lst;
  QRegExp colon("\\:$");

  if (p) {
    if (p->depends->count() > 0) {
      lst = *p->depends;
    } else {
      kpinterface[0]->getPackageDepends(p);
    }
    
    
    QString stmp;
    stmp += "<html><head></head><body>";
    stmp += "<h1 style='font-family: serif;'>";
    stmp += p->getInfo("name");
    stmp += "</h1><hr/>";
    for ( QStringList::const_iterator s = lst.constBegin();
	  s != lst.constEnd();
	  ++s) {
	    if (colon.indexIn(*s) >= 0) {
	      stmp += "<h2>";
	      stmp += *s;
	      stmp += "</h2>\n";
	    } else {
	      stmp += *s;
	      stmp += "<br>\n";
	    }
	  }
	  stmp += "</body></html>";
    setHtml(stmp);
    // kDebug() << stmp << "\n";
  }
  update();
}

#include "packageDepends.moc"
