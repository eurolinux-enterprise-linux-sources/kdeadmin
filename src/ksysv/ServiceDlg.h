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

#ifndef KSV_SERVICE_DIALOG_H
#define KSV_SERVICE_DIALOG_H

#include <kdialogbase.h>
#include <QMap>

class KSVDragList;
class KSVItem;
class QString;
class QComboBox;

class ServiceDlg : public KDialogBase
{
  Q_OBJECT

public:
  ServiceDlg (const QString& action, const QString& label,
	      QWidget* parent = 0, const char* name = 0);
  virtual ~ServiceDlg();

  int count() const;

  void resetChooser (KSVDragList* data, bool edit);

public slots:
  virtual void show ();
  virtual void hide ();
  void toggle ();

private:
  QComboBox* mServices;
  QMap<QString,const KSVItem*> mMapServices;

protected slots:
  virtual void slotApply();

signals:
  void doAction (const QString& on);
  void display (bool);
};

#endif
