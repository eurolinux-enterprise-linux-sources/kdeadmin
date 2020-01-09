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

#ifndef KPTERM_H
#define KPTERM_H

#include <Qt3Support/Q3TextEdit>
#include <QtCore/QStringList>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
//Added by qt3to4:
#include <QtGui/QKeyEvent>

#include <klocale.h>
#include <kdialog.h>
#include <kpackage.h>
#include <kpPty.h>

//////////////////////////////////////////////////////////////////////////////
class kpTerm: public Q3TextEdit
{ 
  Q_OBJECT

public:
  kpTerm(kpPty *pt, QWidget * parent=0, const char * name=0);
  void keyPressEvent ( QKeyEvent * e );
  bool run(const QString &cmd, QStringList &r);
  void doConnect();
  void doUnconnect();
  void insert ( const QString & str, bool mark=false );
  kpPty *pty;
  void cancel();
  void done();

public slots:
  void textIn(const QString &, bool);
  void slotResult(QStringList &, int);

signals:
  void keyOut(QString);
  void result(QStringList &, int);

};

//////////////////////////////////////////////////////////////////////////////

class kpRun: public KDialog
{
  Q_OBJECT

public:
  kpRun(QWidget *parent = 0);
  bool run(QString cmd, QString title);
  void addText(const QStringList &ret);

public slots:
  void slotResult(QStringList &, int);
  void slotCancel();

private:
  kpTerm *term;
  QLabel *title;
};	
//////////////////////////////////////////////////////////////////////////////
#endif // KPTERM_H
