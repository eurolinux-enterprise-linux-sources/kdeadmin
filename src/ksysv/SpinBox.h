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

#ifndef KSV_SPINBOX_H
#define KSV_SPINBOX_H

#include <QSpinBox>
//Added by qt3to4:
#include <QEvent>

#include <kcompletion.h>

class KSVSpinBox : public QSpinBox, public KCompletionBase
{
  Q_OBJECT

public:
  KSVSpinBox (QWidget* parent, const char* name = 0L);
  virtual ~KSVSpinBox ();
  
  virtual bool eventFilter (QObject*, QEvent*);
  virtual void setCompletedText (const QString&);
  virtual void setCompletedItems (const QStringList&, bool);

protected:
  virtual QString mapValueToText (int value);

private slots:
  void handleMatch (const QString&);

private:
  bool mClearedSelection;
};

#endif // KSV_SPINBOX_H
