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

#include "kpackage.h"
#include "managementWidget.h"
#include "pkgOptions.h"
#include "findf.h"
#include "search.h"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

KPACKAGE::KPACKAGE(const KSharedConfigPtr &_config, QWidget *parent)
  : QWidget(parent)
{

  // Save copy of config
  config = _config;

  setAcceptDrops(true);
  setupModeWidgets();

  setupStatusBar();

  file_dialog = NULL;
  findialog = NULL;
  srchdialog = NULL;

}

// Destructor
KPACKAGE::~KPACKAGE()
{
}

// resize event -- arrange the widgets
void KPACKAGE::resizeEvent(QResizeEvent *re)
{
  re = re;			// prevent warning
  arrangeWidgets();
}

// Set up the mode widgets
void KPACKAGE::setupModeWidgets()
{
  management = new managementWidget(this);

      kpinterface[0]->uninstallation = new pkgOptionsU(kpinterface[0]);
      kpinterface[0]->installation = new pkgOptionsI(kpinterface[0]);
}

// Set up the status bar
void KPACKAGE::setupStatusBar()
{
  statusbar = new QFrame(this);
  statusbar->setFrameStyle(QFrame::Raised | QFrame::Panel);
  processProgress = new QProgressBar(statusbar);
  processProgress->setRange(0, 100);
  processProgress->setValue(0);
  processProgress->setTextVisible(false);

  status = new QLabel(i18n("Management Mode"), statusbar);
}

// Arrange the widgets nicely
void KPACKAGE::arrangeWidgets()
{
  statusbar->resize(width(),20);
  statusbar->move(0,height()-20);
  status->resize((statusbar->width() / 4) * 3, 16);
  status->move(2,2);
  processProgress->resize(statusbar->width() / 4 - 4, 16);
  processProgress->move((statusbar->width() / 4) * 3 + 3, 2);

  management->resize(width(),height() - 20);

  kpinterface[0]->installation->resize(width(),height() - 20);
}

void KPACKAGE::setup()
{
  management->collectData(1);
}

void KPACKAGE::fileQuit()		// file->quit selected from menu
{
  kpkg->writeSettings();
 
  KApplication::exit(0);	// exit the application
}

void KPACKAGE::reload()
{
  kpackage->management->collectData(true);
}

void KPACKAGE::fileOpen()		
{
    KFileDialog *box;

    box = getFileDialog(i18n("Select Package"));

    if( box->exec()) {
        if(!box->selectedUrl().isEmpty()) {
            openNetFile( box->selectedUrl() );
        }
    }
}

void KPACKAGE::clearMarked()
{
  management->treeList->clearMarked(management->treeList->firstChild());
}

void KPACKAGE::markAll()
{
  management->treeList->markAll(management->treeList->firstChild());
}

void KPACKAGE::expandTree()
{
  management->treeList->expandTree(management->treeList);
}

void KPACKAGE::collapseTree()
{
  management->treeList->collapseTree(management->treeList);
}


/////////////////////////////////////////////////////////////////////////

void KPACKAGE::openNetFiles (const QStringList &urls, bool install )
{
  QStringList files, names;
  
  QList<packageInfo *> lst;
  packageInfo *pk = 0;

  kDebug() << "openNetFiles\n";

  foreach (const QString &it, urls) {
    QString f = fetchNetFile(it);
    if (!f.isEmpty())
      files.append(f);
    kpkg->add_recent_file(it);
  }

  if (files.count() > 0) {
   foreach (const QString &t, files) {
     kDebug() << "T=" << t << "\n";
      pk = kpinterface[0]->collectPath(t);
      if (pk) {
       kDebug() << "PK OK\n";
       pk->pkgFileIns(t);
       lst.append(pk);
       names.append(pk->getInfo("name"));
      }
    }
  
    if (install) {
      kpinterface[0]->installation->setup(lst);
      if (kpinterface[0]->installation->exec()) {
        kpackage->management->updatePackageState(lst, true);
      }
    } else {
     if (pk) {
        KpTreeListItem *pt = pk->item;
	// NOT the best place for this CODE
       kpackage->management->filterChanged(managementWidget::ALL);
        if (pt)
  	kpackage->management->packageHighlighted(pt);
      }
    }
  }
}

void KPACKAGE::openNetFile(const KUrl &url, bool install )
{
  QStringList lst;
  lst<<url.url();
  openNetFiles(lst, install);
}

//    KMimeMagic *magic = KMimeMagic::self();
//    KMimeMagicResult *r = magic->findFileType(s);
    //    printf("r=%s\n",(r->mimeType()).data());



QString KPACKAGE::getFileName(const KUrl & url, QString &cacheName )
{
  QString none  = "";
  QString fname = "";

  if ( !url.isValid() )  {
    KpMsgE(i18n("Malformed URL: %1", url.url()), true);
  } else {

    // Just a usual file ?
    if ( url.isLocalFile() ) {
      cacheName = url.path();
      fname = url.path();
    } else {
    }
  }
  return fname;
}

bool KPACKAGE::isFileLocal( const KUrl & url )
{
  QString cf;

  QString f = getFileName(url, cf);

  if (cf.isEmpty()) {
    return false;
  } else {
    if (!f.isEmpty()) {
      return true;
    } else {
      return false;
    }
  }
}

QString KPACKAGE::fetchNetFile( const KUrl & url )
{
  QString tmpFile = KStandardDirs::locateLocal("tmp",url.fileName());
  if ( !KIO::NetAccess::download( url, tmpFile, this) ) {
      KMessageBox::error(this, KIO::NetAccess::lastErrorString() );
       return "";
  }
  return tmpFile;
}

/////////////////////////////////////////////////////////////////////////
void KPACKAGE::fileOpenUrl(){

  bool ok;

  QString url = KInputDialog::getText( QString::null,	//krazy:exclude=nullstrassign for old broken gcc
      i18n( "Open location:" ), save_url.prettyUrl(), &ok, this );

  if ( ok )
    {
      kpkg->add_recent_file( url );
      openNetFile( url );
    }
}

void KPACKAGE::find(){
  if (srchdialog)
    srchdialog->show();
  else
    srchdialog = new Search(this, "find package");
}

void KPACKAGE::findf(){
  if (findialog)
    findialog->show();
  else
    findialog = new FindF(this);
}

KFileDialog* KPACKAGE::getFileDialog(const QString &captiontext)
{

  if(!file_dialog) {
    file_dialog = new KFileDialog(QDir::currentPath(), "",
				  this);
  }

  QString pat = "*.deb *.rpm *.tgz";
  
  file_dialog->setFilter(pat);
  file_dialog->setCaption(captiontext);
  //  file_dialog->rereadDir();

  return file_dialog;
}

void KPACKAGE::dragEnterEvent(QDragEnterEvent* e)
{
  e->setAccepted(KUrl::List::canDecode(e->mimeData()));
}

void KPACKAGE::dropEvent(QDropEvent *de) // something has been dropped
{

  KUrl::List list = KUrl::List::fromMimeData( de->mimeData() );
 if ( list.isEmpty() )
     return;

  openNetFiles(list.toStringList());
}

void KPACKAGE::setStatus(const QString &s)	// set the text in the status bar
{
  status->setText(s);
 qApp->processEvents();	// refresh the screen
}

QString KPACKAGE::getStatus()	// get the text in the status bar
{
  if(status)
    return status->text();
  else
    return "";
}

void KPACKAGE::setPercent(int x)	// set the progress in the status bar
{
 processProgress->setValue(x);
 qApp->processEvents();	// refresh it
}

//////////////////////////////////////////////////////////////////////////////



#include "kpackage.moc"
