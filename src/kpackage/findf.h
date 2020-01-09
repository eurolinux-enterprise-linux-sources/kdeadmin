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

#ifndef FINDF_H
#define FINDF_H

// Standard Headers
#include <stdio.h>

// Qt Headers
#include <QtCore/QDir>
#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <Qt3Support/Q3FileDialog>
#include <Qt3Support/Q3GroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QLayout>
//Added by qt3to4:
#include <QtGui/QDragEnterEvent>
#include <QtGui/QPixmap>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QResizeEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QPushButton>

// KDE headers
#include <kmenubar.h>
#include <k3listview.h>
#include <kdialog.h>

class FindF : public KDialog
{
  Q_OBJECT

public:

  FindF ( QWidget *parent = 0);
  ~FindF();
  void resizeEvent(QResizeEvent *);
  void dropEvent(QDropEvent *);
  void dragEnterEvent(QDragEnterEvent* e);
  void disableSearchAll();

  
private:
  void doFind(const QString &str);
  // Do the actual search

  QLineEdit *value;
  Q3ListView *tab;
  QVBoxLayout* vl;
  QVBoxLayout* vtop, vf;

  QHBoxLayout* hb;
  QCheckBox *searchAll;
  QPushButton *updateFiles;
  QPixmap tick;

signals:
    void findf_signal();
    void findf_done_signal();

public slots:
  void done_slot();
  void ok_slot();
  void search(Q3ListViewItem *);
  void textChanged ( const QString & text);

};

#endif
