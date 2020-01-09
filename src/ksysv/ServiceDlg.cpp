/***************************************************************************
    begin                : Sun Oct 3 1999
    copyright            : (C) 1999 by Peter Putzer
    email                : putzer@kde.org
 ***************************************************************************/

/*
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

#include <QLayout>
#include <QLabel>
#include <qfileinfo.h>
#include <QComboBox>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>

#include <klocale.h>
#include <kbuttonbox.h>

#include "ksvdraglist.h"
#include "ServiceDlg.h"

ServiceDlg::ServiceDlg (const QString& action, const QString& label,
						QWidget* parent, const char* name)
  : KDialogBase (parent, name, false, action, Apply|Close, Apply, true)
{
  QWidget* page = new QWidget (this);

  QBoxLayout* top = new QVBoxLayout (page, 0, spacingHint());

  mServices = new QComboBox (false, page);
  QLabel* desc = new QLabel(label, page);
  desc->setMinimumSize(desc->sizeHint());
  desc->setBuddy(mServices);
  mServices->setMinimumSize(mServices->sizeHint());
  mServices->setMinimumWidth(mServices->minimumSize().width() * 2);

  QBoxLayout* serv_layout = new QHBoxLayout();
  top->addLayout (serv_layout);
  serv_layout->addWidget(desc);
  serv_layout->addWidget(mServices);
  
  setFixedSize (sizeHint());
}

ServiceDlg::~ServiceDlg()
{
}

void ServiceDlg::slotApply()
{
  emit doAction (mMapServices[mServices->currentText()]->filenameAndPath());
}

int ServiceDlg::count() const
{
  return mServices->count();
}

void ServiceDlg::resetChooser(KSVDragList* list, bool edit)
{
  mServices->clear();
  mMapServices.clear();

  if (!list)
    return;

  // initialize the combobox
  for (Q3ListViewItemIterator it (list); 
	   it.current();
	   ++it)
    {
      const KSVItem* item = static_cast<KSVItem*> (it.current());
	  
      QFileInfo info (item->filenameAndPath());
      
      if (edit)
		{
		  if (info.isReadable())
			mServices->insertItem(item->label());
		  
		  mMapServices[item->label()] = item;
		}
      else
		{
		  if (info.isExecutable())
			mServices->insertItem(item->label());

		  mMapServices[item->label()] = item;
		}
    }
}

void ServiceDlg::show ()
{
  QDialog::show ();

  emit display (true);
}

void ServiceDlg::hide ()
{
  QDialog::hide ();

  emit display (false);
}

void ServiceDlg::toggle ()
{
  if (isHidden())
    show();
  else
    hide();
}

#include "ServiceDlg.moc"
