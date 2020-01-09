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

#include "search.h"

#include "kpackage.h"
#include "managementWidget.h"
#include <klocale.h>
#include <QtGui/QLineEdit>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <Qt3Support/Q3GroupBox>
//Added by qt3to4:
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>

Search::Search(QWidget *parent, const char *)
    : KDialog(parent)
{
    setCaption( i18n("Find Package") );
    setButtons( User1 | Close );
    setDefaultButton( User1 );
    setButtonGuiItem( User1, KGuiItem( i18n("&Find"), "edit-find") );
    QFrame *page = new QFrame( this );
    setMainWidget( page );

    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout* vtop = new QVBoxLayout( page, 10, 10, "vtop");

    Q3GroupBox *frame1 = new Q3GroupBox(i18n("Find Package"), page, "frame1");
    vtop->addWidget(frame1,1);
    QVBoxLayout* vf = new QVBoxLayout( frame1, 20, 10, "vf");

    value = new QLineEdit( frame1, "v");
    vf->addWidget(value,0);
    value->setFocus();
    value->setFixedHeight(value->sizeHint().height());
    value->setMinimumWidth(250);
    connect(value, SIGNAL(textChanged(const QString &)),this, SLOT(textChanged(const QString &)));

    QHBoxLayout* hc = new QHBoxLayout( );
    vf->addLayout(hc,0);

    substr = new QCheckBox(i18n("Sub string"), frame1, "substr");
    substr->setChecked(true);
    hc->addWidget(substr,1,Qt::AlignLeft);
    substr->setFixedSize(substr->sizeHint());
    hc->addStretch(1);

    wrap = new QCheckBox(i18n("Wrap search"), frame1, "wrap");
    wrap->setChecked(true);
    hc->addWidget(wrap,1,Qt::AlignRight);
    wrap->setFixedSize(wrap->sizeHint());

    enableButton( User1, false );

    connect(this, SIGNAL(user1Clicked()), this, SLOT(ok_slot()));
    connect(this, SIGNAL(closeClicked()), this, SLOT(done_slot()));

    show();
}

Search::~Search()
{
}

void Search::textChanged(const QString &text)
{
    enableButton( User1, !text.isEmpty() );
}

void Search::ok_slot()
{
  Q3ListViewItem *pkg;

  QString to_find = value->text();
  to_find = to_find.trimmed();

  pkg = kpackage->management->search(to_find,
		 substr->isChecked(),false,false);
  if (pkg == 0 && wrap->isChecked()) {
    pkg = kpackage->management->search(to_find,
		 substr->isChecked(),true,false);
  }
  if (pkg == 0)
    KpMsg(i18n("Note"),
	  i18n("%1 was not found.", to_find),true);
}

void Search::done_slot()
{
  hide();
}

#include "search.moc"
