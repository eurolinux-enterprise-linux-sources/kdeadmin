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

#include <QtGui/QLineEdit>
#include <QtGui/QPainter>
//Added by qt3to4:
#include <QtGui/QDragEnterEvent>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QResizeEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QApplication>

#include <klocale.h>
#include <kdebug.h>
#include <k3urldrag.h>
#include <kiconloader.h>

#include "kpackage.h"
#include "managementWidget.h"
#include "findf.h"
#include "pkgInterface.h"


FindF::FindF(QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("Find File") );
    setButtons(User1 | Close );
    setDefaultButton( User1 );
    setButtonGuiItem( User1, 	KGuiItem(i18n("&Find"),"file-find") );
    tick = UserIcon("ptick");

    QFrame *page = new QFrame(this);
    setMainWidget( page );
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout* vtop = new QVBoxLayout( page );
    vtop->setSpacing( KDialog::spacingHint() );
    vtop->setMargin( KDialog::marginHint() );
    vtop->setObjectName( "vtop" );
    Q3GroupBox *frame1 = new Q3GroupBox(i18n("Find Package"), page, "frame1");
    vtop->addWidget(frame1,1);

    QGridLayout* gtop = new QGridLayout( frame1 );
    //    gtop->setMargin( KDialog::marginHint() );
    gtop->setSpacing( KDialog::spacingHint() );

    value = new QLineEdit( frame1 );
    value->setObjectName( "value" );
    connect(value,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
    value->setFocus();

    QLabel *valueLabel = new QLabel(frame1);
    valueLabel->setBuddy(value);
    valueLabel->setText(i18n("Find:"));
    valueLabel->setAlignment( Qt::AlignRight );

    tab = new K3ListView( frame1 );
    tab->setObjectName( "tab" );
    connect(tab, SIGNAL(selectionChanged ( Q3ListViewItem * )),
           this, SLOT(search( Q3ListViewItem * )));
    tab->addColumn(i18n("Installed"),18);
    tab->addColumn("",0);   // Hidden column for package type
    tab->addColumn(i18n("Package"),180);
    tab->addColumn(i18n("File Name"),330);
    tab->setAllColumnsShowFocus(true);
    tab->setSorting(1);
    
    if (kpinterface[0]->ifExe("apt-file")) {
      searchAll = new QCheckBox(i18n("Also search uninstalled packages"), frame1);
      searchAll->setObjectName("searchAll");
    } else {
      searchAll = new QCheckBox(i18n("Also search uninstalled packages (apt-file needs to be installed)"), frame1);
      searchAll->setObjectName("searchAll");
      if (hostName.isEmpty()) {
	searchAll->setDisabled(true);
     } 
    }
    searchAll->setChecked(false);
    

    gtop->addWidget(valueLabel, 0, 0);
    gtop->addWidget(value, 0, 1);
    gtop->addWidget(tab, 1, 0, 1, 2);

    gtop->addWidget(searchAll, 2, 0);

    connect(this, SIGNAL(user1Clicked()), this, SLOT(ok_slot()));
    connect(this, SIGNAL(closeClicked()), this, SLOT(done_slot()));
    enableButton(User1 , false);
    show();

    setAcceptDrops(true);
}

FindF::~FindF()
{
}

void FindF::disableSearchAll()
{
    searchAll->setChecked(false);
    searchAll->setEnabled(false);
}

void FindF::textChanged ( const QString & text)
{
    enableButton(User1 , !text.isEmpty());
}

void FindF::ok_slot()
{
  doFind(value->text());
}

void FindF::doFind(const QString &str)
{
  QString t;
  int cnt = 0;

  bool all = searchAll->isChecked();

  QApplication::setOverrideCursor( Qt::WaitCursor );

  tab->clear();

  for (int i = 0; i < kpinterfaceN; i++) {
    const QStringList filelist = kpinterface[i]->FindFile(str, all);

//    kDebug() << "FindF::doFind" << filelist.count() << "\n";
    if (filelist.count() > 0) {
      cnt++;
    
    for ( QStringList::const_iterator it = filelist.begin(); it != filelist.end(); ++it ) {
      if ((*it).indexOf("diversion by") >= 0) {
        new Q3ListViewItem(tab, "", *it);
      }

      int t1 = (*it).indexOf('\t');
      QString s1 = (*it).left(t1);
      QString s2 = (*it).right((*it).length()-t1);
      s2 = s2.trimmed();

      Q3ListViewItem *ql = new Q3ListViewItem(tab, "", "", s1, s2);

      QString tx = s1;
      if (kpackage->management->dirInstPackages.value(tx)) {
	ql->setPixmap(0,tick);
      }
    }
   }
  }
 
  if (!cnt) {
    new Q3ListViewItem(tab, "", i18n("--Nothing found--"));
  }

  QApplication::restoreOverrideCursor();
}

void FindF::done_slot()
{
  hide();
}

void FindF::resizeEvent(QResizeEvent *){
}

void FindF::search(Q3ListViewItem *item)
{
  int p;

  QString s = item->text(3);
  s = s.trimmed();
  kDebug() << "searchF=" << s << "\n";

  p = s.indexOf(',');
  if (p > 0) {
    s.truncate(p);
  }

  KpTreeListItem *k =  kpackage->management->treeList->search(s ,item->text(1));
  if (k)
    kpackage->management->treeList->changePack(k);
}

void FindF::dragEnterEvent(QDragEnterEvent* e)
{
  e->setAccepted(K3URLDrag::canDecode(e));
}

void FindF::dropEvent(QDropEvent *de) // something has been dropped
{
  KUrl::List list;
  if (!K3URLDrag::decode(de, list) || list.isEmpty())
     return;

  const KUrl &url = list.first();

  if (url.isLocalFile()) {
    QString file = url.path(KUrl::RemoveTrailingSlash);
    value->setText(file);
    doFind(file);
  } else {
    KpMsgE(i18n("Incorrect URL type"),false);
  }
}

#include "findf.moc"
