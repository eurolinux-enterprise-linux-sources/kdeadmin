/*
   Copyright 2000 Peter Putzer <putzer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <q3frame.h>
#include <QLabel>
#include <q3textview.h>
#include <QLayout>
#include <q3hbox.h>
#include <q3vbox.h>
#include <q3buttongroup.h>
#include <QPushButton>
//Added by qt3to4:
#include <QGridLayout>

#include <kdebug.h>
#include <klocale.h>
#include <klineedit.h>
#include <kiconloader.h>

#include "SpinBox.h"
#include "OldView.h"
#include "ksv_conf.h"
#include "ksv_core.h"
#include "Data.h"
#include "Properties.h"

KSVServicePropertiesDialog::KSVServicePropertiesDialog (KSVData& data, QWidget* parent)
  : KPropertiesDialog (KUrl(data.filenameAndPath()), parent),
  mData (data)
{
  KSVServicesPage* page = new KSVServicesPage (data, this);
  insertPlugin (page);
  
  showPage (page->pageIndex ());
}

KSVServicePropertiesDialog::~KSVServicePropertiesDialog ()
{
}

KSVServicesPage::KSVServicesPage (KSVData& data, KPropertiesDialog* props)
  : KPropsDlgPlugin (props),
    mData (data),
    mPage (props->addVBoxPage (i18n("&Service"))),
    mIndex (props->pageIndex (mPage))
{
  mPage->setSpacing (KDialog::spacingHint());

  Q3VBox* desc = new Q3VBox (mPage);
  desc->setSpacing (1);

  QLabel* label = new QLabel(i18n("Description:"), desc);
  label->setFixedHeight (label->sizeHint().height());

  QString text;
  ksv::getServiceDescription (data.filename(), text);
  mDesc = new Q3TextView (QString("<p>%1</p>").arg (text), QString(), desc);

  Q3ButtonGroup* buttons = new Q3ButtonGroup (1, Qt::Vertical, i18n ("Actions"), mPage);
  QPushButton* b = new QPushButton (i18n ("&Edit"), buttons);
  connect (b, SIGNAL (clicked()), props, SLOT (doEdit()));

  Q3Frame* spacer = new Q3Frame (buttons);
  spacer->setMinimumWidth (KDialog::spacingHint());

  b = new QPushButton (i18n ("&Start"), buttons);
  connect (b, SIGNAL (clicked()), props, SLOT (doStart()));

  b = new QPushButton (i18n ("S&top"), buttons);
  connect (b, SIGNAL (clicked()), props, SLOT (doStop()));

  b = new QPushButton (i18n ("&Restart"), buttons);
  connect (b, SIGNAL (clicked()), props, SLOT (doRestart()));
}

KSVServicesPage::~KSVServicesPage ()
{
}

void KSVServicesPage::applyChanges ()
{
}

void KSVServicePropertiesDialog::doEdit ()
{
  emit editService (mData.filenameAndPath ());
}

void KSVServicePropertiesDialog::doStart ()
{
  emit startService (mData.filenameAndPath ());
}

void KSVServicePropertiesDialog::doStop ()
{
  emit stopService (mData.filenameAndPath ());
}

void KSVServicePropertiesDialog::doRestart ()
{
  emit restartService (mData.filenameAndPath ());
}


KSVEntryPropertiesDialog::KSVEntryPropertiesDialog (KSVData& data, QWidget* parent)
  : KPropertiesDialog (data.label(), parent),
    mData (data)
{
  KSVEntryPage* page1 = new KSVEntryPage (data, this);
  insertPlugin (page1);

  KSVServicesPage* page2 = new KSVServicesPage (data, this);
  insertPlugin (page2);
}

KSVEntryPropertiesDialog::~KSVEntryPropertiesDialog ()
{
}

KSVEntryPage::KSVEntryPage (KSVData& data, KPropertiesDialog* props)
  : KPropsDlgPlugin (props),
    mData (data),
    mPage (props->addPage (i18n("&Entry"))),
    mIndex (props->pageIndex (mPage))
{
  QGridLayout* top = new QGridLayout (mPage, 4, 2, 0, KDialog::spacingHint());

  QLabel* labelLabel = new QLabel (i18n ("&Name:"), mPage);
  mLabelEdit = new KLineEdit (mPage);
  mLabelEdit->setText (mData.label());
  labelLabel->setBuddy (mLabelEdit);

  QLabel* serviceLabel = new QLabel (i18n ("&Points to service:"), mPage);
  mServiceEdit = new KLineEdit (mPage);
  mServiceEdit->setCompletionObject (ksv::serviceCompletion(), true);
  mServiceEdit->setText (mData.filename());
  serviceLabel->setBuddy (mServiceEdit);

  QLabel* numberLabel = new QLabel (i18n ("&Sorting number:"), mPage);
  mNumberEdit = new KSVSpinBox (mPage);
  mNumberEdit->setValue (mData.number());
  numberLabel->setBuddy (mNumberEdit);

  QLabel* iconLabel = new QLabel (mPage);
  iconLabel->setPixmap (DesktopIcon ("ksysv", 48));

  top->addWidget (labelLabel, 0, 0);
  top->addWidget (mLabelEdit, 0, 1);
  top->addWidget (serviceLabel, 1, 0);
  top->addWidget (mServiceEdit, 1, 1);
  top->addWidget (numberLabel, 2, 0);
  top->addWidget (mNumberEdit, 2, 1);
  top->addWidget (iconLabel, 3, 0);

  connect (mServiceEdit, SIGNAL (textChanged (const QString&)),
           this, SLOT (emitChanged()));
  connect (mLabelEdit, SIGNAL (textChanged (const QString&)),
           this, SLOT (emitChanged()));
  connect (mNumberEdit, SIGNAL (valueChanged (int)),
           this, SLOT (emitChanged()));
}

KSVEntryPage::~KSVEntryPage ()
{
}

void KSVEntryPage::applyChanges ()
{
  if (mNumberEdit->value() != mData.number())
    {
      mData.setNumber (mNumberEdit->value());
    }

  if (mLabelEdit->text() != mData.label())
    {
      mData.setLabel (mLabelEdit->text());
    }

  if (mServiceEdit->text() != mData.filename())
    {
      mData.setFilename (mServiceEdit->text());
      ksv::serviceCompletion ()->addItem (mData.filename());
    }
}

void KSVEntryPage::emitChanged ()
{
  emit changed();
}

void KSVEntryPropertiesDialog::doEdit ()
{
  emit editService (mData.filenameAndPath ());
}

void KSVEntryPropertiesDialog::doStart ()
{
  emit startService (mData.filenameAndPath ());
}

void KSVEntryPropertiesDialog::doStop ()
{
  emit stopService (mData.filenameAndPath ());
}

void KSVEntryPropertiesDialog::doRestart ()
{
  emit restartService (mData.filenameAndPath ());
}

#include "Properties.moc"
