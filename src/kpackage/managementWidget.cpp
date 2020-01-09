/*
** Copyright (C) 2007 Toivo Pedaste <toivo@ucs.uwa.edu.au>
**
// Author: Toivo Pedaste
//
// See managementWidget.h for more information
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

#include <QtGui/QSplitter>
#include <QtGui/QToolButton>
//Added by qt3to4:
#include <Qt3Support/Q3PtrList>
#include <QtGui/QFrame>
#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <Qt3Support/Q3ValueList>
#include <QtGui/QResizeEvent>
#include <QtGui/QApplication>
#include <QtGui/QComboBox>

#include <klocale.h>
#include <kdebug.h>
#include <k3listviewsearchline.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kglobal.h>

// kpackage.headers
#include "kpackage.h"
#include "kplview.h"
#include "managementWidget.h"
#include "pkgInterface.h"
#include "pkgOptions.h"
#include "packageDisplay.h"
#include "packageProperties.h"


KpListViewSearchLine::KpListViewSearchLine(QWidget *parent, KpTreeList *listView)
  :K3ListViewSearchLine(parent, listView)
{
  list = listView;
}

KpListViewSearchLine::~KpListViewSearchLine()
{
}

void KpListViewSearchLine::updateSearch(const QString &s)
{
    list->expand();
    K3ListViewSearchLine::updateSearch(s);
    K3ListViewSearchLine::updateSearch(s); // Yes both are needed
    list->sweep(false);
}


// constructor -- initialise variables
managementWidget::managementWidget(QWidget *parent)
  : QFrame(parent)
{
  install_action = 0;
  uninstall_action = 0;

  tType[0] = i18n("Installed Packages");
  tType[1] = i18n("New Packages");
  tType[2] = i18n("Updated Packages");
  tType[3] = i18n("Available Packages");
  tType[4] = i18n("All Packages");

  allPackages.clear();
  setupWidgets();

  connect(treeList,SIGNAL(updateMarked()),
	  this, SLOT( checkMarked()));


}

managementWidget::~managementWidget()
{
}

void managementWidget::resizeEvent(QResizeEvent *)
{
  arrangeWidgets();
}


void managementWidget::setupWidgets()
{

  top = new QBoxLayout(QBoxLayout::TopToBottom, this);
  vPan  = new QSplitter(Qt::Horizontal, this);
  top->addWidget(vPan);

  // the left panel
  leftpanel = new QFrame(vPan);
  leftbox = new QBoxLayout(QBoxLayout::TopToBottom, leftpanel);

  treeList = new KpTreeList(leftpanel);


  // Quick Search Bar
  searchToolBar = new QFrame( leftpanel);
  searchBox = new QBoxLayout(QBoxLayout::LeftToRight, searchToolBar);

  searchLabel = new QLabel(i18n("Search: "),searchToolBar);
  searchLine = new KpListViewSearchLine(searchToolBar, treeList);
  searchCombo = new QComboBox;
  for (int i = 0; i < 5; i++) {
    searchCombo->addItem(tType[i]);
  }
  treeList->treeType = 4;
  searchCombo->setCurrentIndex(treeList->treeType);

  searchBox->addWidget(searchLabel);
  searchBox->addWidget(searchLine);
  searchBox->addWidget(searchCombo);

  Q3ValueList<int> clist;  clist.append(0);  clist.append(1); clist.append(2);
  searchLine->setSearchColumns(clist);

 searchToolBar->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
// kDebug() << "searchToolBar->sizeHint() " << searchToolBar->sizeHint()  << "\n";

  connect( treeList, SIGNAL( cleared() ), searchLine, SLOT( clear() ));

  connect(searchCombo,SIGNAL(activated (int)),SLOT(filterChanged(int)));

  leftbox->addWidget(searchToolBar);
  leftbox->addWidget(treeList,1);

  leftbox->addStretch();

  lbuttons = new QBoxLayout(QBoxLayout::LeftToRight);

  luinstButton = new QPushButton(i18n("Uninstall Marked"),leftpanel);
  luinstButton->setEnabled(false);
  connect(luinstButton,SIGNAL(clicked()),
	  SLOT(uninstallMultClicked()));
  linstButton = new QPushButton(i18n("Install Marked"),leftpanel);
  linstButton->setEnabled(false);
  connect(linstButton,SIGNAL(clicked()),
	  SLOT(installMultClicked()));

  leftbox->addLayout(lbuttons,0); // top level layout as child

  // Setup the `buttons' layout
  lbuttons->addWidget(linstButton,1,Qt::AlignBottom);
  lbuttons->addWidget(luinstButton,1,Qt::AlignBottom);
  lbuttons->addStretch(1);

  connect(treeList, SIGNAL(selectionChanged(Q3ListViewItem *)),
         SLOT(packageHighlighted(Q3ListViewItem *)));

  // the right panel
  rightpanel = new QFrame(vPan);
  rightbox = new QBoxLayout(QBoxLayout::TopToBottom, rightpanel);

  packageDisplay = new packageDisplayWidget(rightpanel);
  //  connect(this, SIGNAL(changePackage(packageInfo *)),
  //  packageDisplay, SLOT(changePackage(packageInfo *)));

  rbuttons = new QBoxLayout(QBoxLayout::LeftToRight);

  uinstButton = new QPushButton(i18n("Uninstall"),rightpanel);
  uinstButton->setEnabled(false);
  connect(uinstButton,SIGNAL(clicked()),
	  SLOT(uninstallSingleClicked()));
  instButton = new QPushButton(i18n("Install"),rightpanel);
  instButton->setEnabled(false);
  connect(instButton,SIGNAL(clicked()),
	  SLOT(installSingleClicked()));


  // Setup the `right panel' layout
  rightbox->addWidget(packageDisplay,10);
  rightbox->addLayout(rbuttons,0); // top level layout as child

  // Setup the `buttons' layout
  rbuttons->addWidget(instButton,1);
  rbuttons->addWidget(uinstButton,1);
  rbuttons->addStretch(1);

}

////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////
void managementWidget::writePSeparator()
{
  kDebug() << "managementWidget::writePSeparator()\n";
  KConfigGroup config(KGlobal::config(), "Panel");

  config.writeEntry("panel1Width",vPan->sizes().first());
  config.writeEntry("panel2Width",vPan->sizes().last());
}

void managementWidget::readPSeparator()
{
  kDebug() << "managementWidget::readPSeparator()\n";
  KConfigGroup config(KGlobal::config(), "Panel");

  int w1 = config.readEntry("panel1Width",200);
  int w2 = config.readEntry("panel2Width",200);

  Q3ValueList<int> size;
  size << w1 << w2;
  vPan->setSizes(size);
}

///////////////////////////////////////////////////////////////////
void managementWidget::setupMultButton(int &cntInstall, int &cntUnInstall)
{
  if (cntInstall)
    linstButton->setEnabled(true);
  else
    linstButton->setEnabled(false);

  if (cntUnInstall)
    luinstButton->setEnabled(true);
  else
    luinstButton->setEnabled(false);
}

void managementWidget::setupInstButton()
{
  bool u,i;

  packageInfo *package = packageDisplay->package;

  if (!package) {
    i = false;
    u = false;
  } else {
    instButton->setText(i18n("Install"));
    if (package->isInstallable() ) {

      i = true;
      u = false;
    } else {
      i = false;
      u = true;
    }
  }
  instButton->setEnabled(i);
  if (install_action)
    install_action->setEnabled(i);

  uinstButton->setEnabled(u);
  if (uninstall_action)
    uninstall_action->setEnabled(u);
}

void managementWidget::arrangeWidgets()
{
  // this is done automatically by the layout managers
}

void managementWidget::filterChanged(int tab)
{
   treeList->treeType = tab;
   searchLine->updateSearch();
}


// Collect data from package.
void managementWidget::collectData(bool refresh)
{
 if (!refresh) {
    treeList->sweep(true);
    return; // if refresh not required already initialised
  }

QApplication::setOverrideCursor( Qt::WaitCursor );

// stop clear() sending selectionChanged signal
  disconnect(treeList, SIGNAL(selectionChanged(Q3ListViewItem *)),
          this, SLOT(packageHighlighted(Q3ListViewItem *)));

  treeList->hide();    // hide list tree
  treeList->clear();   // empty it

  connect(treeList, SIGNAL(selectionChanged(Q3ListViewItem *)),
          SLOT(packageHighlighted(Q3ListViewItem *)));

  packageDisplay->changePackage(0);

  // Delete old list if necessary

  QMutableListIterator<packageInfo *> p(allPackages);
  while (p.hasNext()) {
    delete p.next();
    p.remove();
  }

  dirInstPackages.clear();
  dirUninstPackages.clear();

  // List installed packages
  kpinterface[0]->listPackages(allPackages);

  // Rebuild the list tree
  rebuildListTree();

  kpackage->setStatus(kpackage->displayMsg);
  kpackage->displayMsg.clear();

  QApplication::restoreOverrideCursor();
}

// Rebuild the list tree
void managementWidget::rebuildListTree()
{
  int  n = 0;

  kpackage->setStatus(i18n("Building package tree"));
  kpackage->setPercent(0);

  treeList->setSorting(-1);

  // place all the packages found
  int count = allPackages.count();
  int incr = count/20;
  if (incr == 0)
    incr = 1;

  foreach (packageInfo *p, allPackages) {
//    kDebug() << "rebuildListTree()" << n << "\n";
    p->place(treeList,true);

    if (!(n % incr)) {
      kpackage->setPercent(int (n*100/count));
    }
    n++;
  }

  treeList->sweep(true);
  treeList->expandTree(treeList);

  treeList->setSorting(0);
  treeList->show();		// show the list tree

  kpackage->setPercent(100);	// set the progress
  kpackage->setStatus("");

  checkMarked();
}

// A package has been highlighted in the list tree
void managementWidget::packageHighlighted(Q3ListViewItem *item)
{
//  kDebug() << "packageHighlighted " << column << "\n";
//  if (column == 0) {
//    checkMarked();
//  }

  treeList->packageHighlighted(item, packageDisplay);
  setupInstButton();


  kpackage->setPercent(100);
}

void managementWidget::updatePackageState(QList<packageInfo *> packs,  bool wantInstall)
{
  QStringList pkList;
  QHash<QString, packageInfo *> infoPt;
  QApplication::setOverrideCursor( Qt::WaitCursor );

  foreach (packageInfo *p, packs) {
    QString name(p->getInfo("name"));
    if (p->hasInfo("version")) {
      name += '_';
      name += p->getInfo("version");
    }
    pkList << name;
    infoPt[name] = p;
  }

  QStringList instList = kpinterface[0]->getPackageState(pkList);

  QSet <QString> uset;
  foreach (const QString &u, instList) {
//            kDebug() << "instList =" << u << "\n";
    uset.insert(u);
  }

  foreach (const QString &s, pkList) {
    updatePackage(infoPt[s],wantInstall,uset.contains(s));

  QApplication::restoreOverrideCursor();

  }
}
/////////////////////////////////////////////////////////////////////////
// install has been clicked

void managementWidget::installSingleClicked()
{
  int result;
  QList<packageInfo *> plist;

  packageInfo *package = packageDisplay->package;

  if (package) {
      QString filename = package->getFilename();
      kDebug() << "File=" << filename  <<"\n";
      pkgInterface *interface = kpinterface[0];
      plist.append(package);
      if (!interface->installation->setup(plist)) {
        return;
      }
      result = interface->installation->exec();

      if (interface->installation->result() == QDialog::Accepted ||
	  interface->installation->modified) {
	// it was accepted, so the package has been installed

	updatePackageState(plist,  true);

	if (treeList->currentItem()) {
	  KpTreeListItem *p = treeList->currentItem();
	  packageDisplay->changePackage(p->info);
	} else {
	  packageDisplay->changePackage(0); // change package to no package
	}
	setupInstButton();
      }
  }
  kpackage->setPercent(100);

  checkMarked();
}

// install has been clicked
void managementWidget::installMultClicked()
{
  KpTreeListItem *it;
  QList<packageInfo *> lst;

  selList.clear();
  treeList->findMarked(treeList->firstChild(), selList);
  kDebug() << "installMultClicked() " << selList.count();

  for (it = selList.first(); it != 0; it = selList.next()) {
    if (
	    it->childCount() == 0 &&
	    (it->info->packageState == packageInfo::UPDATED ||
	     it->info->packageState == packageInfo::NEW))
    {
      lst.insert(0,it->info);
    }
  }
  selList.clear();

  if (lst.count() > 0) {
    if (kpinterface[0]->installation->setup(lst)) {
      if (kpinterface[0]->installation->exec() ||
	    kpinterface[0]->installation->modified) {
	   updatePackageState(lst,  true);
           if (treeList->currentItem()) {
               KpTreeListItem *p = treeList->currentItem();
               packageDisplay->changePackage(p->info);
           } else {
               packageDisplay->changePackage(0); // change package to no package
           }
      }
    }
  }

  checkMarked();
}

/////////////////////////////////////////////////////////////////////////////
// Uninstall has been clicked

void managementWidget::uninstallSingleClicked()
{
  int result;
  QList<packageInfo *> plist;

  packageInfo *package = packageDisplay->package;

  if (package) {			// check that there is a package to uninstall
    pkgInterface *interface = kpinterface[0];
    plist.append(package);
    if (!interface->uninstallation->setup(plist)) {
      return;
    }
    result = interface->uninstallation->exec();

    if(result == QDialog::Accepted ||
       interface->installation->modified) {
      updatePackageState(plist,  false);
      if (treeList->currentItem()) {
          KpTreeListItem *p = treeList->currentItem();
          packageDisplay->changePackage(p->info);
      } else {
          packageDisplay->changePackage(0); // change package to no package
      }
      setupInstButton();
    }
    //    kDebug() << "Result=" << result <<"\n";
  }
  kpackage->setPercent(100);

  checkMarked();
}

void managementWidget::uninstallMultClicked()
{
  KpTreeListItem *it;
  QList<packageInfo *> lst;

  selList.clear();
  treeList->findMarked(treeList->firstChild(), selList);
  for (it = selList.first(); it != 0; it = selList.next()) {
     if ( it->childCount() == 0 &&
	    (it->info->packageState == packageInfo::INSTALLED ||
	     it->info->packageState == packageInfo::BAD_INSTALL)) {
      lst.append(it->info);
    }
  }
  selList.clear();

  if (lst.count() > 0) {
    if (kpinterface[0]->uninstallation->setup(lst)) {
      if (kpinterface[0]->uninstallation->exec()||
	    kpinterface[0]->installation->modified ) {
	updatePackageState(lst,  false);
        if (treeList->currentItem()) {
            KpTreeListItem *p = treeList->currentItem();
            packageDisplay->changePackage(p->info);
	} else {
            packageDisplay->changePackage(0); // change package to no package
	}
      }
    }
  }

  checkMarked();
}


///////////////////////////////////////////////////////////////////////////

void managementWidget::doChangePackage(packageInfo *p)
{
   packageDisplay->changePackage(p);
}

///////////////////////////////////////////////////////////////////////////

KpTreeListItem *managementWidget::search(QString str, bool subStr, bool wrap,
			     bool start)
{
  return treeList->search(str, subStr, wrap, start);
}


///////////////////////////////////////////////////////////////////////////
KpTreeListItem *managementWidget::updatePackage(packageInfo *pki, bool, bool haveInstall)
{
  QString version;
  KpTreeListItem *q;

  QString name(pki->getInfo("name"));
  if (pki->hasInfo("version"))
    version = pki->getInfo("version");
  else
    version = "";

  packageInfo *ptree;
  QString pkgId =  name;
  kDebug() << "pkgId=" << pkgId << "\n";

  if (haveInstall) {
	ptree = dirInstPackages.value(pkgId); // remove installed entry
	dirInstPackages.remove(pkgId);
	if (ptree) {
	  if (ptree->getItem()) {
	    delete ptree->getItem();
	    ptree->item = 0;
	  }
	}

	ptree = dirUninstPackages.value(pkgId); // remove uninstalled entry
	if (ptree) {
	  ptree->packageState = packageInfo::HIDDEN;
	  if (ptree->getItem()) {
	    delete ptree->getItem();
	    ptree->item = 0;
	  }
	}

      pki->rmInfo("old-version");
      pki->packageState = packageInfo::INSTALLED;
      q = pki->place(treeList,true);

      if (!q) {
	kDebug() << "N=" << pki->getInfo("name") << "\n";
      } else {
	treeList->sweep(true);
	searchLine->updateSearch();
	return q;
      }
  } else { // uninstalling
      dirInstPackages.remove(pkgId);
      KpTreeListItem  *qt = pki->getItem();
      if (qt) {
	kDebug() << "qt pkgId=" << pkgId << "\n";
	treeList->stackRemove(qt);
        treeList->setSelected(qt,false);

 	kDebug() << "treeList->setSelected(qt,false);" << "\n";
	if (treeList->markPkg == qt)
	  treeList->markPkg = 0;
	pki->deleteItem();
      } else {
	kDebug() << "DEL=" << name;
      }

      packageInfo *pb = dirUninstPackages.value(pkgId);
      if (pb) { // available package matching the one just uninstalled
	kDebug() << "pb pkgId=" << pkgId << "\n";
	pb->packageState = packageInfo::NEW;
	dirUninstPackages.insert(pkgId, pb);
	dirInstPackages.remove(pkgId);
	q = pb->place(treeList,true);
	if (!q) {
	  kDebug() << "NOTP=" << qPrintable(pb->getInfo("name")) << "\n";
	} else {
	  treeList->sweep(true);
	  searchLine->updateSearch();
	  return q;
	}
      } else {
	pki->packageState = packageInfo::NEW;
	dirUninstPackages.insert(pkgId, pki);
	q = pki->place(treeList,true);
	if (!q) {
	  kDebug() << "NOTP=" << qPrintable(pb->getInfo("name")) << "\n";
	} else {
	  treeList->sweep(true);
	  searchLine->updateSearch();
	  return q;
	}

      }
  }

  treeList->sweep(true);
  searchLine->updateSearch();

  return 0;
}

///////////////////////////////////////////////////////////////////////////
void managementWidget::checkMarked()
{
int  cntInstall = 0;
int  cntUnInstall = 0;

  treeList->countMarked(treeList->firstChild(), cntInstall, cntUnInstall);
  kDebug() << "checkMarked() " << cntInstall << " " << cntUnInstall << "\n";
  setupMultButton(cntInstall, cntUnInstall);
}


#include "managementWidget.moc"
