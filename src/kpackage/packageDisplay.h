/*
** Copyright (C) 1999,2000 Toivo Pedaste <toivo@ucs.uwa.edu.au>
**
// This widget is used to display information and the file list of a
// package.
//
// Package information will be displayed using (another) sub-widget
// that is inherited from a QTableView.
//
// The file list will be displayed using a tree list.
//
// The widget is mainly a QTabDialog with two tabs: Info and FileList.
// The Info tab is the default one.
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

#ifndef PACKAGEDISPLAY_H
#define PACKAGEDISPLAY_H

// Qt Headers
#include <QtGui/QTabBar>
#include <QtGui/QTabWidget>
#include <kvbox.h>
//Added by qt3to4:
#include <QtGui/QPixmap>
#include <kmenu.h>  
#include <k3listview.h>

class packagePropertiesWidget;
class packageDepends;
class packageInfo;
class Q3ListViewItem;
class QTextEdit;
class packageDisplayWidget;
class kpFileList : public K3ListView
{
  Q_OBJECT

public:
  
  kpFileList(QWidget* parent, packageDisplayWidget* parent2);
  virtual ~kpFileList() {} 
  QString item2Path(Q3ListViewItem *it);
  

public slots:
 
  void openContext(K3ListView *, Q3ListViewItem *, const QPoint &);
  
  virtual void clear();
  
  Q3ListViewItem* insert(const QString &cur, const QPixmap &pixmap);

  
 private:
   KMenu *FileListMenu;
   packageDisplayWidget* pkDisplay;
   
   int openwith;

};

class packageDisplayWidget : public QTabWidget
{
  Q_OBJECT

  friend class kpFileList;
  ///////////// METHODS ------------------------------------------------------
public:
  packageDisplayWidget(QWidget *parent=0);
  // Constructor

  ~packageDisplayWidget();
  // Destructor

  void noPackage();
  // clear package display in right panel

  void changePackage(packageInfo *p);
  // Set currently selected package

private:
  void setupWidgets();
  // This sets up the sub-widgets

  void updateFileList();
  // This updates the file list to match that found with the currently
  // selected package
  
  void updateDepends();
  // Update dependency list
  
  void updateChangeLog();
  // This updates the change log to match that found with the currently
  // selected package
  
  void tabSet(QWidget *);
  // Set display for corresponding tab

  ///////////// SLOTS --------------------------------------------------------
public slots:

  void tabSelected(QWidget *);

  void openBinding(Q3ListViewItem *);

  void openBindingWith(Q3ListViewItem *);  
  
void __openBindingWith();
  
  
  ///////////// SIGNALS ------------------------------------------------------

  ///////////// DATA ---------------------------------------------------------
public:
  packageInfo *package;
  // the currently selected package

private:

  QTabWidget *tabbar;
  // The tab bar

  KVBox *proptab, *deptab, *fltab, *cltab;

  QWidget *curTab;
  // current active tab

  kpFileList *fileList;
  // This holds the file list (and is used as a page on the tab dialog)

  QTextEdit *changeLog;
  // Holds changelog

  QPixmap tick, cross, question, *blank;
  // The pixmaps for the filelist

  packagePropertiesWidget *packageProperties;
  // This displays the package properties (and is used as a page on the
  // tab dialog)

  packageDepends *depends;
  // Dependency display in tab dialog
  
  bool initList;
  // True is file list has been initialised
};
#endif
