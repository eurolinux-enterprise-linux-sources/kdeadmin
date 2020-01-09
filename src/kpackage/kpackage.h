/*
** Copyright (C) 2007 Toivo Pedaste <toivo@ucs.uwa.edu.au>
**
** This is the main widget for kpackage
** The whole widget is a DND drop zone where users can drop packages to
** be installed.
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

#ifndef KPACKAGE_H
#define KPACKAGE_H

// KDE headers
#include <kurl.h>
#include <kxmlguiwindow.h>
#include <kpPty.h>
#include <ksharedconfig.h>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QLabel>
#include <QtGui/QFrame>
#include <QtGui/QResizeEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QCloseEvent>

#include <kpkg.h>

class KFileDialog;
class QProgressBar;
class QLabel;
class Search;
class FindF;
class pkgInterface;
class managementWidget;
class kpRun;

//////////////////////////////////////////////////////////////////////////////

 #define kpinterfaceN 3

 enum {UPACKAGE = 0, DEBt, RPMt, SLACKt};

  //////////////////////////////////////////////////////////////////////////////
class KPACKAGE : public QWidget
{
  Q_OBJECT

  ///////////// METHODS ------------------------------------------------------
public:
  KPACKAGE(const KSharedConfigPtr &_config, QWidget *parent);
  // Constructor

  ~KPACKAGE();
  // Destructor

  void setStatus(const QString &s);
  // this sets the status bar's string to s

  void setPercent(int x);
  // this set the status bar's progress to x

  QString getStatus();
  // this gets the current status string on the status bar

  //  void setMode(int newmode, pkgInterface *type, int refresh);
  // This sets the mode to newmode and updates the display accordingly.

  void setup();

  pkgInterface *pkType(const QString &fname);
  // find type of package

  void openNetFiles(const QStringList &urls, bool install=true);
  void openNetFile(const KUrl & url, bool install=true);
  // open a file given a URL

  QString fetchNetFile(const KUrl & url);
  // fetch a file given a URL

  static QString getFileName(const KUrl & url, QString &cacheName);
  // return file name, if not local file cachename is name for cache entry

  static bool isFileLocal( const KUrl & url );
  // true if URL refers to local or cached file

protected:
  void resizeEvent(QResizeEvent *re);
  // This is called when the widget is resized

  void dropEvent(QDropEvent *);
  // This is called when a URL has been dropped in the drop zone

  void dragEnterEvent(QDragEnterEvent* e);

private:

  void setupModeWidgets();
  // This sets up the mode widgets (ie management/installation widgets)

  void setupStatusBar();
  // This sets up the status bar

  void arrangeWidgets();
  // This arranges the widgets in the window (should be called after a
  // resize event)

  KFileDialog* getFileDialog(const QString &captiontext);

  ///////////// SLOTS --------------------------------------------------------
public slots:

//  void modeFinished(int mode, pkgInterface *interface, int refresh);
  // This is called when the mode `mode' has finished.  KPACKAGE should
  // then change modes appropriately

  void fileOpen();
  // This is called when File->Open is selected from the menu

  void clearMarked();
  // clear package Marks

  void markAll();
  // mark all packages in the selected view

  void expandTree();
  void collapseTree();
  // expand and collapse file tree

  void fileOpenUrl();
  // menu item FIle->OpenUrl

  void find();
  // search for package

  void findf();
  // search for file in package

  void fileQuit();
  // This is called when File->Quit is selected from the menu

  void reload();
  // reload file package information

 ///////////// SIGNALS ------------------------------------------------------

  ///////////// DATA ---------------------------------------------------------
public:

  enum { Management, Installation } ;
  // Widget modes

  KSharedConfigPtr config;
  // pointer to kconfig object

  managementWidget *management;
  // management widget

  KUrl save_url;
  // save the URL entered

  FindF *findialog;
  // find file dialog
  
  QString displayMsg;

private:
  int mode;
  // Widget mode

  // Menu item identifiers

  QFrame *statusbar;
  // the status bar

  QProgressBar *processProgress;
  // Progress bar for showing progress

  QLabel *status;
  // The actual status

  KFileDialog *file_dialog;
  /// If we load a file from the net this is the corresponding URL

  Search *srchdialog;
  // find package dialog

};


//////////////////////////////////////////////////////////////////////////////
extern KPKG *kpkg;

extern KPACKAGE *kpackage;
extern kpPty *kpty;
extern kpRun *kprun;
extern kpRun *kpstart;

extern QString hostName;

extern pkgInterface *kpinterface[];

void KpMsg(const QString &lab, const QString &msg, bool stop);
void KpMsgE(const QString &msg, bool stop = false);
#endif

