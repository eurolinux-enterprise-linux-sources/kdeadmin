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

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <kpropertiesdialog.h>
//Added by qt3to4:
#include <QFrame>

class Q3HBox;
class Q3VBox;
class Q3TextView;
class QSpinBox;

class KLineEdit;

class KSVData;

class KSVServicePropertiesDialog : public KPropertiesDialog
{
  Q_OBJECT

public:
  KSVServicePropertiesDialog (KSVData& data, QWidget* parent);
  virtual ~KSVServicePropertiesDialog ();

signals:
  void startService (const QString&);
  void stopService (const QString&);
  void restartService (const QString&);
  void editService (const QString&);

private slots:
  void doEdit ();
  void doStart ();
  void doStop ();
  void doRestart ();

private:
  KSVData& mData;
};


class KSVEntryPropertiesDialog : public KPropertiesDialog
{
  Q_OBJECT

public:
  KSVEntryPropertiesDialog (KSVData& data, QWidget* parent);
  virtual ~KSVEntryPropertiesDialog ();

signals:
  void startService (const QString&);
  void stopService (const QString&);
  void restartService (const QString&);
  void editService (const QString&);

private slots:
  void doEdit ();
  void doStart ();
  void doStop ();
  void doRestart ();

private:
  KSVData& mData;
};

class KSVEntryPage : public KPropertiesDialogPlugin
{
  Q_OBJECT

public:
  KSVEntryPage (KSVData& data, KPropertiesDialog* props);
  virtual ~KSVEntryPage ();

  virtual void applyChanges ();

  inline int pageIndex () const { return mIndex; }

private slots:
  void emitChanged ();

private:
  KSVData& mData;
  QFrame* mPage;
  int mIndex;

  KLineEdit* mServiceEdit;
  KLineEdit* mLabelEdit;
  QSpinBox* mNumberEdit;
};

class KSVServicesPage : public KPropertiesDialogPlugin
{
  Q_OBJECT

public:
  KSVServicesPage (KSVData& data, KPropertiesDialog* props);
  virtual ~KSVServicesPage ();

  virtual void applyChanges ();

  inline int pageIndex () const { return mIndex; }

private:
  KSVData& mData;
  QFrame* mPage;
  Q3TextView* mDesc;
  int mIndex;
};

#endif // PROPERTIES_H
