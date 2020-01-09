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

// qt headers
#include <QtGui/QLabel>
//Added by qt3to4:
#include <QtGui/QShowEvent>
#include <QtCore/QList>
#include <QtGui/QBoxLayout>
#include <QtGui/QCloseEvent>
#include <QtGui/QListWidget>
#include <QtGui/QApplication>

#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kseparator.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <KStandardGuiItem>

#include "pkgOptions.h"
#include "managementWidget.h"
#include "kpackage.h"

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

pkgOptions::pkgOptions(pkgInterface *pki, QWidget *parent, const QString &caption)
  :  QDialog(parent)
{
  //  setFrameStyle(QFrame::Raised | QFrame::Panel);

//  kDebug() << "pkgOptions::pkgOptions\n";
  pkgInt = pki;

  hide();

  setCaption(caption);
}

// Destructor
pkgOptions::~pkgOptions()
{
  //  int i;
  //  for (i = 0; i < bnumber; i++) {
  //    delete(Boxes[i]);
  //  }
}

// Set up the sub-widgets
void pkgOptions::setupWidgets(QList<param *> &pars)
{
  int i;

  // Create widgets
  title = new QLabel("", this);
  QFont f( KGlobalSettings::generalFont());
  f.setBold(true);
  f.setPointSize(f.pointSize()+6);
  title->setFont(f);
  //  title->setAutoResize(true);
  //  title->update();

  installButton = new QPushButton(insType);
  cancelButton = new KPushButton(KStandardGuiItem::cancel());
  // count number of buttons
  bnumber = pars.count();

  Boxes = new QCheckBox *[bnumber];
  i = 0;
//  for ( p=pars.first(); p != 0; p=pars.next(), i++ ) {
  foreach (param *p, pars) {
    Boxes[i] = new QCheckBox(p->name);
    Boxes[i]->setChecked(p->init);
    i++;
  }

  Keep = new QCheckBox(i18n("Keep this window"));

  // Connections
  connect(installButton,SIGNAL(clicked()),SLOT(pkginstallButtonClicked()));
  connect(cancelButton,SIGNAL(clicked()),SLOT(cancelButtonClicked()));
  connect(Keep, SIGNAL(toggled(bool)), SLOT(keepToggle(bool)));

  // Do the layout
  vlayout = new QBoxLayout(this, QBoxLayout::TopToBottom);
  vlayout->addWidget(title,0);

  {
    hlayout = new QBoxLayout(vlayout,QBoxLayout::LeftToRight);

    {
      layout = new QBoxLayout(hlayout,QBoxLayout::TopToBottom);

      packages = new QListWidget();
      layout->addWidget(packages,20);
      
      QStringList heads;

      connect(packages, SIGNAL(itemClicked( QListWidgetItem *)),
	      this, SLOT(slotSearch( QListWidgetItem *)));


      layout->addStretch(1);
      for (i = 0; i < bnumber; i++) {
	layout->addWidget(Boxes[i],1);
      }
      layout->addWidget(new KSeparator(Qt::Horizontal), 2);

      QBoxLayout *slayout = new QBoxLayout(layout, QBoxLayout::LeftToRight);
      slayout->addStretch(1);
      slayout->addWidget(Keep, 1);
      slayout->addStretch(1);

      layout->addWidget(new KSeparator(Qt::Horizontal), 2);

      QBoxLayout *buttons = new QBoxLayout(QBoxLayout::LeftToRight);
      layout->addLayout(buttons);

      buttons->addWidget(installButton,2);
      buttons->addStretch(1);
      buttons->addWidget(cancelButton,2);
    }
    {
      term = new kpTerm(kpty);
      hlayout->addWidget(term, 1000);
    }
  }
  setLayout(vlayout);
  resize(800, 400);
}

bool pkgOptions::setup(packageInfo *p) {
//  QList<packageInfo *> *pl = new QList<packageInfo *>;
  QList<packageInfo *> pl;
  pl.append(p);
  return setup(pl);
}

void pkgOptions::resetPackages(bool init)
{
  //kDebug() << "resetPackages()\n";
  installButton->setEnabled(false);

  int packCount = packList.count();
  
  packListChecked.clear();
  QStringList checked;
  QSet<QString> isChecked;
  for (int i = 0; i < packCount; i++) {
    QListWidgetItem *w = packages->item(i);
      if (w->checkState() == Qt::Checked) {
	kDebug() << "CK=" << w->text() << "\n";
	checked << w->text();
	isChecked.insert(w->text());
      }
  }
  
  if (checked == checkedPackages && !init) 
    return;
  
  QApplication::setOverrideCursor( Qt::WaitCursor );
  checkedPackages = checked;
  
  packListChecked.clear();
  foreach (packageInfo *p, packList) {
//    kDebug() << "PC=" << p->fetchFilename() << "\n";
    if (isChecked.contains(p->fetchFilename())) {
      packListChecked << p;
    }
  }
  
  bool cancel;
  QStringList rlist;
  if (checked.count() > 0) {
    rlist = pkgInt->listInstalls(checked, installer, cancel);
  }
//  kDebug() << "rlist=" << rlist << "\n";
 
  QListWidgetItem *wd;
  while ((wd = packages->takeItem(packCount))) {
//    kDebug() << "packages del " << wd->text() << "\n";
    delete wd;
  }
  
  foreach (const QString &ritem, rlist) {
    if (!isChecked.contains(ritem)) {
       new QListWidgetItem(ritem, packages);
    }
 }
 
 if (packListChecked.count() > 0)
   installButton->setEnabled(true);

  QApplication::restoreOverrideCursor();
}

bool pkgOptions::setup(QList<packageInfo *> pl)
{
 QString s;
 modified = false;

 packList = pl;
 
  QStringList plist, rlist, clist;
  QSet<QString> dict;
//  kDebug() << "pl=" << pl << "\n";
  
  foreach (packageInfo *p, pl) {
    plist += p->fetchFilename();
    dict.insert(p->getInfo("name"));
  }
//  kDebug() << "clist=" << clist << "\n";
 
 s = i18ncp("Used when listing how many packages are to be installed, and how many are to be uninstalled.  The argument on the left is a translation of Install or Uninstall respectively.", "%2: 1 Package","%2: %1 Packages",plist.count(),insType);
 title->setText(s);
// kDebug() << "plist=" << plist << "\n";

 packages->clear();
 int i = 0;
  foreach (const QString &pit, plist) {
    QListWidgetItem *pw = new QListWidgetItem(pit, packages);
    if (i < packList.count()) {
      pw->setCheckState(Qt::Checked);
    }
    i++;
  }
  
  cancelButton->setGuiItem(KStandardGuiItem::cancel());
  resetPackages(true);
  return true;
}

// install button has been clicked....so install the package
void pkgOptions::pkginstallButtonClicked()
{
  int i;
  QStringList r;
  modified = true;

  // Collect data from check boxes
  int installFlags = 0;

  for (i = 0; i < bnumber; i++) {
    installFlags |= (Boxes[i]->isChecked()) << i;
  }

  test = false;
  QString s = doPackages(installFlags, packListChecked, test);
  // A "0=" or "1=" indicates it was actually (un)installed by the doPackages
  // routine instead of just returning a command to execute

//            kDebug() <<  "S=" << s << "\n";
  if (s == "0=") {
    cancelButtonClicked();
  } else if (s.left(2) == "1=") {
    term->textIn(s.mid(2), true);
  } else {
    connect(term,SIGNAL(result(QStringList &, int)),
	 this,SLOT(slotResult(QStringList &, int)));

    installButton->setEnabled(false);

    if (term->run(s, r)) {
      running = true;
      cancelButton->setGuiItem(KStandardGuiItem::cancel());
    } else {
      reset();
    }
  }
}

void pkgOptions::slotSearch(QListWidgetItem *item)
{
  if (item) {
    QString s = item->text();
    // kDebug() << "searchI=" << s << "\n";

    foreach (packageInfo *p, packList) {
      if (s == p->getInfo("name")) {
        kpackage->management->doChangePackage(p);
        break;
      }
    }
    resetPackages(false);
  }
}

void pkgOptions::reset() {
  installButton->setEnabled(true);
  cancelButton->setGuiItem(KGuiItem(i18n("Done")));  //clear icon
  disconnect(term,SIGNAL(result(QStringList &, int)),
	 this,SLOT(slotResult(QStringList &, int)));
  running = false;
}

void pkgOptions::slotResult(QStringList &, int ret)
{
  reset();
  if (ret == 0 && !test && !keep) {
    term->done();
    accept();
  }
}

void pkgOptions::terminate() {
  if (running) {
    term->cancel();
    reset();
  }
}

void pkgOptions::cancelButtonClicked()
{
  terminate();
  term->done();

  if (!modified || test)
    reject();
  else
    accept();
}

void pkgOptions::closeEvent ( QCloseEvent * e ) {
//  kDebug() << "pkgOptions::QCloseEvent\n";
  terminate();

  QWidget::closeEvent (e);
}

void pkgOptions::showEvent ( QShowEvent *e ) {
  //  kDebug() << "pkgOptions::showEvent\n";
  getKeep();

  modified = false;
  running = false;

  QWidget::showEvent(e);
}

void pkgOptions::keepToggle(bool kp)
{
  //  kDebug() << "KEEP " << kp << "\n";

  KConfigGroup config = KGlobal::config()->group("Kpackage");

  config.writeEntry("keepIWin", kp);

  keep = kp;
}

void pkgOptions::getKeep()
{
    KConfigGroup config(KGlobal::config(), "Kpackage");
    keep =  config.readEntry("keepIWin", true);
//            kDebug() << "getKeeP " << keep << "\n";
    Keep->setChecked(keep);

}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
pkgOptionsI::pkgOptionsI(pkgInterface *pkg, QWidget *parent):
  pkgOptions(pkg, parent, i18n("Install"))
{
//  kDebug() << "pkgOptionsI::pkgOptionsI\n";
  insType = i18n("Install");
  installer = true;
  setupWidgets(pkg->paramsInst);
}

QString pkgOptionsI::doPackages(int installFlags, QList<packageInfo *> p, bool &test)
{
  return pkgInt->install(installFlags, p, test);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
pkgOptionsU::pkgOptionsU(pkgInterface *pkg, QWidget *parent):
  pkgOptions(pkg, parent, i18n("Uninstall"))
{
//  kDebug() << "pkgOptionsU::pkgOptionsU\n";
  insType = i18n("Uninstall");
  installer = false;
  setupWidgets(pkg->paramsUninst);
}

QString pkgOptionsU::doPackages(int installFlags, QList<packageInfo *> p, bool &test)
{
  return pkgInt->uninstall(installFlags, p, test);
}
#include "pkgOptions.moc"
