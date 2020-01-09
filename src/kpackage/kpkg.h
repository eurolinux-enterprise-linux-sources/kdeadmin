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

#ifndef KPKG_H
#define KPKG_H

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

class QLabel;
class Options;
class KRecentFilesAction;
class KConfigGroup;
class kpSettings;
class kpConfig;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class KPKG : public KXmlGuiWindow
{
  Q_OBJECT

  enum { Tback = 1,
	 Tforward = 2,
	 Tfileopen = 3,
	 Tftout = 4,
	 Tftin = 5,
	 Tfind = 6,
	 Tfindf = 7,
	 Treload = 8
  };

public:

  KPKG(const KSharedConfigPtr &_config);
  // Constructor

  void add_recent_file(const QString &newfile);
  // keep list of files accessed

  void add_recent_host(const QString &newhost);
  
  void createSettings();
  // Edit settings dialog
     
  void writeSettings();
  // write config settings

  void saveProperties(KConfigGroup& config);
  void readProperties(const KConfigGroup& config);
  // save and read restart sstate

  void disableMenu();
  void enableMenu();
  // enable/deisable menu elements

  void disableNext();
  void enableNext();
  void disablePrevious();
  void enablePrevious();
  // Control next and previous commands
  
  
  bool prop_restart;
  // indicates a restart from saved state

  Options *optiondialog;
  // Options dialog

  KSharedConfigPtr config ;
  // Saved config information

  kpSettings *conf;
  kpConfig *kconf;

private:

  void setupMenu();
  // This sets up the menubar

  QAction *pack_hopen;
  QAction *pack_open;
  QAction *pack_find;
  QAction *pack_findf;
  QAction *kpack_reload;
  QAction *pack_prev;
  QAction *pack_next;
  QAction *pack_install;
  QAction *pack_uninstall;

  KConfigGroup *fgroup;
  KConfigGroup *hgroup;

  int  toolID, selectID;
  // refrences to  toolbar  and menu items

  bool hide_toolbar;
  // don't display toolbar

  KRecentFilesAction *recent;
  KRecentFilesAction *recentHosts;

public slots:

  void openHost();

  void openRecentHost(const KUrl& url);

  void openRecent(const KUrl& url);
  // opens file from list of recently opened ones

  void setOptions();
  // set options

  void saveSettings();
  // save config

  void configureToolBars();  
  
  void updateConfiguration();
 
  
  

 
private slots:
  void slotNewToolbarConfig();

protected:
  void closeEvent ( QCloseEvent *e);
};

extern KPKG *kpkg;

//////////////////////////////////////////////////////////////////////////////
#endif
