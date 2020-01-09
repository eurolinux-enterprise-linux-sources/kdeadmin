/*
** Copyright (C) 1999,2007 Toivo Pedaste <toivo@ucs.uwa.edu.au>
**
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
#include <stdio.h>

// Qt headers
#include <QtGui/QApplication>
#include <QtCore/QFileInfo>
#include <QtGui/QTextEdit>
//Added by qt3to4:
#include <QtGui/QPixmap>

#include <kdebug.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <krun.h>

// kpackage.headers
#include "kpackage.h"
#include "packageDisplay.h"
#include "packageDepends.h"
#include "packageProperties.h"
#include "pkgInterface.h"
#include "kpSettings.h"
#include "utils.h"
#include <klocale.h>


// constructor
packageDisplayWidget::packageDisplayWidget(QWidget *parent)
  : QTabWidget(parent)
{
  // Initially we're not dealing with any package
  package=NULL;

  // Set up the widgets
  setupWidgets();

  // Load the pixmaps
  tick = UserIcon("ptick");
  cross = UserIcon("cross");
  question = UserIcon("question");
  blank = new QPixmap();

}

packageDisplayWidget::~packageDisplayWidget()
{
    delete blank;
}

void packageDisplayWidget::setupWidgets()
{
  proptab = new KVBox( this);
  curTab = proptab;
  deptab = new KVBox( this);
  fltab = new KVBox( this);
  cltab = new KVBox( this);

  packageProperties = new packagePropertiesWidget(proptab);
  depends = new packageDepends(deptab);
  
  fileList = new kpFileList(fltab, this);
  connect(fileList, SIGNAL(executed(Q3ListViewItem *)),
            this, SLOT( openBinding(Q3ListViewItem *)) );
  connect(fileList, SIGNAL(returnPressed(Q3ListViewItem *)),
            this, SLOT( openBinding(Q3ListViewItem *)) );

  changeLog = new QTextEdit(cltab);

  addTab(proptab, i18n("Properties"));
  addTab(deptab, i18n("Dependencies List"));
  addTab(fltab, i18n("File List"));
  addTab(cltab, i18n("Change Log"));

  if (isTabEnabled(indexOf (cltab)))
      setTabEnabled(indexOf (cltab),false);
  if (isTabEnabled(indexOf (deptab)))
      setTabEnabled(indexOf (deptab),false);
  if (isTabEnabled(indexOf (fltab)))
      setTabEnabled(indexOf (fltab),false);
  if (isTabEnabled(indexOf (proptab)))
      setTabEnabled(indexOf (proptab),false);

  connect(this,SIGNAL(currentChanged(QWidget *)), this, SLOT(tabSelected(QWidget *)));
}

void packageDisplayWidget::tabSelected(QWidget *tab)
{
  curTab = tab;
  tabSet(tab);
}

void packageDisplayWidget::tabSet(QWidget *tab)
{
  disconnect(this,SIGNAL(currentChanged(QWidget *)), this, SLOT(tabSelected(QWidget *)));
  if(tab == proptab) {
    packageProperties->show();
    depends->hide();
    fileList->hide();
    changeLog->hide();
    setCurrentIndex(0);
  } else if (tab == deptab) {
    packageProperties->hide();
    depends->show();
    fileList->hide();
    changeLog->hide();
    updateDepends();
    setCurrentIndex(1);
  } else if (tab == fltab) {
    packageProperties->hide();
    depends->hide();
    fileList->hide();
    changeLog->hide();
    if (isTabEnabled(indexOf (fltab))) {
      if (!initList) {
	updateFileList();
	initList = 1;
      } 
      fileList->show();
    } else {
      fileList->hide();
    }
    setCurrentIndex(2);
  } else {
   packageProperties->hide();
    depends->hide();
    fileList->hide();
    if (isTabEnabled(indexOf (cltab))) {
      updateChangeLog();
      changeLog->show();
    } else {
      changeLog->hide();
    }
    setCurrentIndex(3);
  }
  connect(this,SIGNAL(currentChanged(QWidget *)), this, SLOT(tabSelected(QWidget *)));
}

void packageDisplayWidget::noPackage()
{
  disconnect(this,SIGNAL(currentChanged(QWidget *)), this, SLOT(tabSelected(QWidget *)));
//         kDebug() <<  "packageDisplayWidget::noPackage\n";
  setTabEnabled(indexOf (proptab),false);
  setTabEnabled(indexOf (deptab),false);
  setTabEnabled(indexOf (fltab),false);
  setTabEnabled(indexOf (cltab),false);

  packageProperties->changePackage(NULL);

  packageProperties->setText("");
  fileList->clear();
  fileList->setColumnText(0,"");
  changeLog->setText("");
  depends->setText("");
  
  connect(this,SIGNAL(currentChanged(QWidget *)), this, SLOT(tabSelected(QWidget *)));
}

// Change packages
void packageDisplayWidget::changePackage(packageInfo *p)
{

// This is to stop selectionChanged firing off here

    disconnect(fileList, SIGNAL(executed(Q3ListViewItem *)),
            this, SLOT( openBinding(Q3ListViewItem *)) );
  disconnect(fileList, SIGNAL(returnPressed(Q3ListViewItem *)),
            this, SLOT( openBinding(Q3ListViewItem *)) );
  disconnect(fileList, SIGNAL(contextMenu(K3ListView *, Q3ListViewItem *, const QPoint &)),
            fileList, SLOT( openContext(K3ListView *, Q3ListViewItem *, const QPoint &)) );


  if (package && package != p) {
    if (!package->getItem() && !kpackage->management->allPackages.contains(package)) {
      delete package;
      package = 0;
    }
  }

  package = p;
  if (!p) {			// change to no package
    noPackage();
  } else {
    QString u = package->getFilename();
    if (!package->updated &&  !u.isEmpty()) {
      packageInfo *np = package->interface->getFPackageInfo(u);

      if (np) { 
	QMapIterator<QString, QString> it(np->info);
	while (it.hasNext()) {
	  it.next();
	  package->info.insert(it.key(),it.value());
	}
	package->interface = np->interface;
	delete np;
	package->updated = true;
	package->updateItem();
      }
    }

    initList = 0;
    
    packageProperties->changePackage(package);
    if (package->interface->filesTab(package))
      setTabEnabled(indexOf (fltab),true);
    else
      setTabEnabled(indexOf (fltab),false);

    if (package->interface->depTab(package))
      setTabEnabled(indexOf (deptab),true);
    else
      setTabEnabled(indexOf (deptab),false);


    if (package->interface->changeTab(package))
      setTabEnabled(indexOf (cltab),true);
    else
      setTabEnabled(indexOf (cltab),false);

    setTabEnabled(indexOf (proptab),true);
 
    tabSet(curTab);


  }
    connect(fileList, SIGNAL(executed(Q3ListViewItem *)),
            this, SLOT( openBinding(Q3ListViewItem *)) );
  connect(fileList, SIGNAL(returnPressed(Q3ListViewItem *)),
            this, SLOT( openBinding(Q3ListViewItem *)) );
  connect(fileList, SIGNAL(contextMenu(K3ListView *, Q3ListViewItem *, const QPoint &)),
            fileList, SLOT( openContext(K3ListView *, Q3ListViewItem *, const QPoint &)) );

 }

void packageDisplayWidget::updateDepends()
{
  if (!package)
    return;
  
  depends->clear();
  if (package->depends == 0)
    package->interface->getPackageDepends(package);
  depends->changePackage(package);
}

void packageDisplayWidget::updateChangeLog()
{
  if (!package)
    return;

  QStringList lines;
  QString stmp;
  lines = package->interface->getChangeLog(package);

  changeLog->setTextFormat(Qt::LogText);
  changeLog->hide();
  if (lines.count() > 1) {
    changeLog->setText("");
    for (QStringList::ConstIterator it = lines.constBegin();
	 (it != lines.constEnd());
	 it++) {
      if (! (*it).isEmpty())
	changeLog->append(*it);
      else
	changeLog->append(" ");
    }
  } else {
    changeLog->setText(i18n(" - No change log -"));
  }
  changeLog->show();
  changeLog->scroll(-99999,-9999);
}

void packageDisplayWidget::updateFileList()
{
  if (!package)
    return;

  // Get a list of files in the package
  QStringList errorfiles;

  // set the status
  kpackage->setStatus(i18n("Updating File List"));

  // clear the file list
  fileList->clear();
  fileList->setColumnText(0, "");

  if (package->files == 0) 
    package->interface->getFileList(package);
  
//        kDebug() << "files.count=" << package->files->count() << "\n";
  if (package->files->count() == 0)
    return;

  // Get a list of files that failed verification
  if (kpkg->conf->verify) {
    errorfiles = package->interface->verify(package, *package->files);
  }

  kpackage->setStatus(i18n("Updating File List"));

  uint c=0, p=0;
  uint step = (package->files->count() / 100) + 1;

  QString ftmp;
  ftmp.setNum(package->files->count());
  ftmp += i18n(" Files");

  fileList->setColumnText(0, ftmp);
  fileList->hide();
  fileList->setSorting(-1);

  Q3ListViewItem *q;

  // Go through all the files
  for (QStringList::ConstIterator it = package->files->constBegin(); (it != package->files->constEnd());  ) {
    // Update the status progress
    c++;
    if(c > step) {
      c=0;
      p++;
      kpackage->setPercent(p);
    }

    int error=0;

      QString cur = *it;
      it++;
      QPixmap pixmap;
     
      if ( errorfiles.count() > 0) {
	for( QStringList::ConstIterator itError = errorfiles.constBegin();
	     (itError != errorfiles.constEnd());
	     (itError++) ) {
	  if (cur == *itError) {
	    error = 1;
	  }
	}
      }
       if(error) 
	 pixmap=cross;
       else 
         pixmap=tick;
    
     q = fileList->insert(cur, pixmap);   
 }

  fileList->setSorting(0);
  fileList->show();
  kpackage->setPercent(100);
}
  
 kpFileList::kpFileList(QWidget* parent, packageDisplayWidget* parent2) : K3ListView(parent)
 {
   hide();
   addColumn("name");
   setRootIsDecorated(true);
   connect(this, SIGNAL(contextMenu(K3ListView *, Q3ListViewItem *, const QPoint &)),
             this, SLOT( openContext(K3ListView *, Q3ListViewItem *, const QPoint &)) );
            
   FileListMenu = new KMenu();
   openwith = FileListMenu->insertItem(i18n("&Open With..."),parent2,SLOT(__openBindingWith()));	// what to do with this insertItem()?
   
   pkDisplay = parent2;
 }

 void packageDisplayWidget::__openBindingWith()
 {
        openBindingWith(fileList->selectedItem());
 }
 
 void packageDisplayWidget::openBindingWith(Q3ListViewItem *index)
 {      
     if ( !index ) return; 
     KUrl url;
     if (package && package->packageState == packageInfo::INSTALLED) {
       url.setPath( fileList->item2Path(index) ); // from local file to URL
       KRun::displayOpenWithDialog(KUrl::List(url),this );
      }      
 }

 void kpFileList::openContext(K3ListView *, Q3ListViewItem *, const QPoint &p)
 {
        FileListMenu->setItemEnabled(openwith,
        (selectedItem() && pkDisplay->package && pkDisplay->package->getFilename().isEmpty()) ? true : false);
        FileListMenu->exec(p);
 }

 void kpFileList::clear()
 {
        K3ListView::clear();
 }

 QString kpFileList::item2Path(Q3ListViewItem *it)
 {
        QString res;
        res = it ? it->text(0) : NULL;  
        return res;
 }

 Q3ListViewItem* kpFileList::insert(const QString &cur, const QPixmap &pixmap)
 {
   Q3ListViewItem* q;

   q = new Q3ListViewItem(this, cur);
   if (q) 
     q->setPixmap(0,pixmap);
   return q;       
 }
 
 void packageDisplayWidget::openBinding(Q3ListViewItem *index)
 {
   if ( !index ) return; 
   KUrl url;
   if (package && package->packageState == packageInfo::INSTALLED) {
     url.setPath( fileList->item2Path(index) ); // from local file to URL
     (void) new KRun ( url,this ); // run the URL
   }
 }

#include "packageDisplay.moc"
