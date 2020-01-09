/*
** Copyright (C) 2007 Toivo Pedaste <toivo@ucs.uwa.edu.au>
**
// Author:          Toivo Pedaste
//
// See kpackage.h for more information.
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

#include <kactioncollection.h>
#include <kapplication.h>
#include <QtCore/QDir>
#include <QtGui/QLabel>
#include <QtGui/QFrame>
//Added by qt3to4:
#include <QtGui/QDragEnterEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QCloseEvent>

#include <kdebug.h>
#include <kxmlguifactory.h>
#include <kfiledialog.h>
#include <QtGui/QProgressBar>
#include <kurl.h>
#include <krecentfilesaction.h>
#include <kaction.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstandardaction.h>
#include <kedittoolbar.h>
#include <kstandardshortcut.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include "kpkg.h"
#include "managementWidget.h"
#include "pkgOptions.h"
#include "findf.h"
#include "search.h"
#include "kpSettings.h"
#include "kpConfig.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

KPKG::KPKG(const KSharedConfigPtr &_config)
  : KXmlGuiWindow(0),
  config(KGlobal::config())
{
  kpackage = new KPACKAGE(_config, this);
  setCentralWidget(kpackage);

  kpackage->management->readPSeparator();

  // Get a nice default size
  resize(960,540);

  setupMenu();
  disableNext();
  disablePrevious();

  prop_restart = false;
  setAutoSaveSettings();
}

// Set up the menu

void KPKG::setupMenu()
{

  pack_open = KStandardAction::open(kpackage, SLOT(fileOpen()),
			       actionCollection());

  recent = KStandardAction::openRecent(this, SLOT(openRecent(const KUrl&)),
				  actionCollection());
  fgroup = new  KConfigGroup(config, "RecentFiles");
  recent->loadEntries( *fgroup );


  pack_hopen = actionCollection()->addAction( "pack_hopen");
  pack_hopen->setText(i18n("remote &Host..."));
  pack_hopen->setIcon(KIcon("remote-host"));
  connect(pack_hopen, SIGNAL(triggered()), this, SLOT(openHost()));

  recentHosts = actionCollection()->add<KRecentFilesAction>("remote_host");
  recentHosts->setText(i18n("Recent Remote Hosts"));
  recentHosts->setIcon(KIcon("remote-host"));
  connect(recentHosts, SIGNAL(urlSelected(const KUrl&)),SLOT(openRecentHost(const KUrl&)));

	  //  recentHosts = new KRecentFilesAction(KIcon("remote-host"), i18n("Recent remote Hosts"), actionCollection());
  
	  //  connect(recentHosts, SIGNAL(urlSelected(const KUrl&)),this, SLOT(openRecentHost(const KUrl&)));
				 
  hgroup = new  KConfigGroup(config, "RecentHosts");
  recentHosts->loadEntries( *hgroup );


  pack_find = actionCollection()->addAction( "pack_find");
  pack_find->setText(i18n("Find &Package..."));
  pack_find->setIcon(KIcon("edit-find"));
  qobject_cast<KAction*>( pack_find )->setShortcut(KStandardShortcut::shortcut(KStandardShortcut::Find));
  connect(pack_find, SIGNAL(triggered()), kpackage, SLOT(find()));

  pack_findf = actionCollection()->addAction( "pack_findf");
  pack_findf->setText(i18n("Find &File..."));
  pack_findf->setIcon(KIcon("file-find"));
  connect(pack_findf, SIGNAL(triggered()), kpackage, SLOT(findf()));

  kpack_reload = actionCollection()->addAction( "kpack_reload" );
  kpack_reload->setText(i18n("&Reload"));
  kpack_reload->setIcon(KIcon("view-refresh"));
  qobject_cast<KAction*>( kpack_reload )->setShortcut(KStandardShortcut::shortcut(KStandardShortcut::Reload));
  connect(kpack_reload, SIGNAL(triggered()), kpackage, SLOT(reload()));

  (void) KStandardAction::quit(kpackage, SLOT(fileQuit()),
			  actionCollection());

  pack_prev = KStandardAction::back(kpackage->management->treeList, SLOT(previous()),
			       actionCollection());
  actionCollection()->addAction( "pack_prev", pack_prev );

  pack_next = KStandardAction::forward(kpackage->management->treeList, SLOT(next()),
				  actionCollection());

  actionCollection()->addAction( "pack_next", pack_next );
  QAction *a = actionCollection()->addAction( "kpack_expand");

  a->setText(i18n("&Expand Tree"));
  a->setIcon(KIcon("ftout"));
  connect(a, SIGNAL(triggered()), kpackage, SLOT(expandTree()));

  a = actionCollection()->addAction("kpack_collapse");

  a->setText(i18n("&Collapse Tree"));
  a->setIcon(KIcon("ftin"));
  connect(a, SIGNAL(triggered()), kpackage, SLOT(collapseTree()));

  a = actionCollection()->addAction("kpack_clear");
  a->setText(i18n("Clear &Marked"));
  connect(a, SIGNAL(triggered()), kpackage, SLOT(clearMarked()));

  a = actionCollection()->addAction( "kpack_markall");
  a->setText(i18n("Mark &All"));
  connect(a, SIGNAL(triggered()), kpackage, SLOT(markAll()));

  pack_install = actionCollection()->addAction("install_single" );
  pack_install->setText(i18n("&Install"));
  connect(pack_install, SIGNAL(triggered()), kpackage->management, SLOT(installSingleClicked()));

  pack_install->setEnabled(false);
  kpackage->management->setInstallAction(pack_install);


  pack_uninstall = actionCollection()->addAction( "uninstall_single");
  pack_uninstall->setText(i18n("&Uninstall"));
  connect(pack_uninstall, SIGNAL(triggered()), kpackage->management, SLOT(uninstallSingleClicked()));

  pack_uninstall->setEnabled(false);
  kpackage->management->setUninstallAction(pack_uninstall);


  a = actionCollection()->addAction( "install_marked");
  a->setText(i18n("&Install Marked"));
  connect(a, SIGNAL(triggered()), kpackage->management, SLOT(installMultClicked()));

  a = actionCollection()->addAction( "unstall_marked");

  a->setText(i18n("&Uninstall Marked"));
  connect(a, SIGNAL(triggered()), kpackage->management, SLOT(uninstallMultClicked()));

  setStandardToolBarMenuEnabled(true);

  KStandardAction::configureToolbars( this, SLOT(configureToolBars()),
				 actionCollection());

  KStandardAction::saveOptions( this, SLOT(saveSettings()), actionCollection());

  KStandardAction::keyBindings( guiFactory(), SLOT(configureShortcuts()), actionCollection());

  a = actionCollection()->addAction( "kpack_options");

  a->setText(i18n("Configure &KPackage..."));
  a->setIcon(KIcon("configure"));
  connect(a, SIGNAL(triggered()), this, SLOT(setOptions()));

  kDebug() << "kpinterface[DEBt]->makeMenu\n";
  kpinterface[DEBt]->makeMenu(actionCollection());
 
  //  urlList.setAutoDelete(TRUE);
  createGUI();
  
  createSettings();
}

void KPKG::createSettings()
{ 
  conf = new kpSettings();
  conf->readSettings();
  kconf = new kpConfig(conf, this);  
  connect( kconf, SIGNAL(settingsChanged(const QString&)), 
           this, SLOT(updateConfiguration()) );
  
}

void KPKG::updateConfiguration()
{
  conf->readSettings();
  kDebug() << "kpConfig::updateConfiguration()\n";
}

void KPKG::disableMenu()
{
  pack_open->setEnabled(false);
  pack_find->setEnabled(false);
  pack_findf->setEnabled(false);
  kpack_reload->setEnabled(false);
  recent->setEnabled(false);
}

void KPKG::enableMenu()
{
  pack_open->setEnabled(true);
  pack_find->setEnabled(true);
  pack_findf->setEnabled(true);
  kpack_reload->setEnabled(true);
  recent->setEnabled(true);
}

void KPKG::disableNext() {
  pack_next->setEnabled(false);
}

void KPKG::enableNext() {
  pack_next->setEnabled(true);
}

void KPKG::disablePrevious() {
  pack_prev->setEnabled(false);
}

void KPKG::enablePrevious() {
  pack_prev->setEnabled(true);
}
void KPKG::openRecent(const KUrl& url){

  kpackage->openNetFile( url );
}

void KPKG::openHost() {
  kDebug("KPKG::openHost\n");

  QString host  = KInputDialog::getText( QString::null,
    i18n("Name of Remote Host:") );
 
  if (!host.isEmpty()) {
    QString url = "ssh:" + host;
    add_recent_host(url);

    QStringList s;
    s << "--remote" << host;
    int n = KProcess::startDetached("kpackage",s);
    kDebug() << " startDetached " << n << "\n";
  }
}

void KPKG::openRecentHost(const KUrl& url){
    kDebug() << "  KPKG::openRecentHost " << url<< "\n";

    QString host = url.url().remove("ssh://");
    host = host.remove("ssh:");
    QStringList s;

    s << "--remote" << host;
    int n = KProcess::startDetached("kpackage",s);
    kDebug() << " startDetached " << n << "\n";
}

void KPKG::add_recent_file(const QString &newfile){

  KUrl url = KUrl(newfile);

  recent->addUrl( url );
  recent->saveEntries(*fgroup);
}

void KPKG::add_recent_host(const QString &newhost){

  KUrl url(newhost);

  recentHosts->addUrl( url );
  recentHosts->saveEntries(*hgroup);
}

void KPKG::configureToolBars() {
  KEditToolBar dlg(actionCollection());
  connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(slotNewToolbarConfig()));
  dlg.exec();
}

void KPKG::slotNewToolbarConfig() {
  createGUI();
}

void KPKG::writeSettings(){

  kpackage->management->writePSeparator();

  KConfigGroup group(config, "Kpackage");
  recent->saveEntries( group );

  kpackage->management->treeList->writeTreePos();

  config->sync();
}

void KPKG::setOptions(){
  kconf->show();
}

void KPKG::saveSettings(){
  writeSettings();
}

void KPKG::saveProperties(KConfigGroup& config )
{
    config.writePathEntry("Name", kpackage->save_url.url());
}


void KPKG::readProperties(const KConfigGroup &config)
{
    QString entry = config.readPathEntry("Name", QString()); // no default
    if (entry.isNull())
	return;
    QStringList lst;
    lst <<entry;
    kpackage->openNetFiles(lst);
    
    prop_restart = true;
}

void KPKG::closeEvent ( QCloseEvent *) {
    kpackage->fileQuit();
}

