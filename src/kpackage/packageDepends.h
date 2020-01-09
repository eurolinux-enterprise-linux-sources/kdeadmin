/*
** Copyright (C) 1999,2007 Toivo Pedaste <toivo@ucs.uwa.edu.au>
**
// This widget displays package dependencies
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

#ifndef PACKAGEDEPENDS_H
#define PACKAGEDEPENDS_H

// Standard Headers

// Qt Headers
#include <QtCore/QStringList>
#include <QtCore/QRegExp>

// KDE Headers
#include <klocale.h>
#include <ktextbrowser.h>

// kpackage Headers

class packageInfo;

class packageDepends : public KTextBrowser
{
  Q_OBJECT
  ///////////// METHODS ------------------------------------------------------
  public:

    packageDepends(QWidget *parent=0);
  // constructor

    ~packageDepends();
  // destructor
    
    void changePackage(packageInfo *p);

};

#endif
