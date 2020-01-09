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

#ifndef KPLVIEW_H
#define KPLVIEW_H

// Standard Headers

// Qt Headers
#include <QtGui/QPushButton>
#include <Qt3Support/Q3PtrList>

#include <QtGui/QLayout>
//Added by qt3to4:
#include <QtGui/QMouseEvent>
#include <QtGui/QPixmap>

// KDE headers
#include "k3listview.h"

// ksetup headers
#include "packageInfo.h"

class packageDisplayWidget;

////////////////////////////////////////////////////////////////////////
class KpTreeList: public K3ListView
{
  Q_OBJECT

public:
  KpTreeList ( QWidget * parent = 0);

  void contentsMousePressEvent ( QMouseEvent * e );

  bool inMark(int x);
  
  KpTreeListItem *firstChild();
  KpTreeListItem *currentItem();
  void clear();

  KpTreeListItem *markPkg;

  Q3PtrList<KpTreeListItem> stack;
  // Stack of jumped to packages

  void sweep(bool init);
  // sweep tree adjusting visibility
  void expand();
  // sweep tree expanding everything

  void findMarked(KpTreeListItem *item, Q3PtrList<KpTreeListItem> &selList);
  // generate list of marked tree items
  void clearMarked(KpTreeListItem *item);
  // unmark marked tree items

  // mark all packages in the selected view
  void markAll(KpTreeListItem *item) ;

  void expandTree(KpTreeList *list);
  // expand package tree

  void expandTree(KpTreeListItem *item);
  // expand package sub-tree

  void collapseTree(KpTreeList *list);
  // semi-collapse package tree

  void collapseTree(KpTreeListItem *item);
  // semi-collapse package sub-tree

  void countMarked(KpTreeListItem *, int &cntInstall, int &cntUnInstall);
  // Count marked packages that can be installed/uninstalled

  void packageHighlighted(Q3ListViewItem *item, packageDisplayWidget *packageDisplay);
  // A package has been highlighted in the list tree

  KpTreeListItem *search(const QString &str, const QString &head,
			KpTreeListItem  *start = 0);
  KpTreeListItem *search(const QString &str, bool subStr, bool wrap, bool start);
  // search for a package in tree
  KpTreeListItem *changePack(KpTreeListItem *searchResult, bool push = true);
  // Change to other package

  void stackRemove(KpTreeListItem *pack);
  // Remove entry from package stack

  void writeTreePos();
  void readTreePos();
  // save and restore column positions

 int treeType;

public slots:
  void next();
  // Package stack forward

  void previous();
  // Package stack back

private:
  int  sweepChild(KpTreeListItem *it);
  void  expandChild(KpTreeListItem *it);

  bool searchChild(KpTreeListItem *it);
  // recurse thru the display tree looking for 'str'

  bool notPress;
  // flag to packageHighlighted

  KpTreeListItem *searchCitem;
  bool searchSkip, searchSubstr;
  QString searchStr;
  KpTreeListItem  *searchResult;
  // globals used by searchChild for start from current position,
  // skip to current item before search flag, substring search flag,
  // search string, result item (if found)

  // flag skipping in searchChild

  KpTreeListItem *topLevelItem(int i);
 
signals:
  void updateMarked();
  void cleared();
};

////////////////////////////////////////////////////////////////////////
class KpTreeListItem : public Q3ListViewItem
{
  public:
   KpTreeListItem( Q3ListViewItem *parent, packageInfo* pinfo,
            const QPixmap& thePixmap,
            QString label1 = 0, QString label2  = 0,
            QString label3  = 0, QString label4  = 0,
            QString label5  = 0, QString label6  = 0,
            QString label7  = 0, QString label8 = 0);

  KpTreeListItem( K3ListView *parent, packageInfo* pinfo,
            const QPixmap& thePixmap,
            QString label1 = 0, QString label2  = 0,
            QString label3  = 0, QString label4  = 0,
            QString label5  = 0, QString label6  = 0,
            QString label7  = 0, QString label8 = 0);

   KpTreeListItem *firstChild();
   KpTreeListItem *nextSibling();

  void toggleMark();
  void setMark(bool mark);
  // flag for install/uninstall

  void hide();
  void show();

  virtual int compare( Q3ListViewItem *i, int col, bool ascending ) const;

  packageInfo *info;
  bool marked;
};

#endif // KPLVIEW_H
