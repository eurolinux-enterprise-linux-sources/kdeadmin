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

#ifndef KPPTY_H
#define KPPTY_H

#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/QTextCodec> 
#include <QtCore/QMap>
#include <QTimer>
//Added by qt3to4:
#include <k3procio.h>

//////////////////////////////////////////////////////////////////////////////

class kpKProcIO: public K3ProcIO
{
   Q_OBJECT

public:

   kpKProcIO ( QTextCodec *_codec = 0);
  ~kpKProcIO();

  bool sstart (RunMode runmode);
};


//////////////////////////////////////////////////////////////////////////////
class  kpPty: public QObject
{ Q_OBJECT
public:
  kpPty();
 ~kpPty();

  QStringList  run(const QString &cmd, bool inLoop = true,
		   bool needRoot= false, bool noReturn = false);
  bool startSession(bool needRoot);
  void close();

  QString remote;
  int Result;
  // True if have started a session
  bool inSession; 
  
  void listClear();
  
private slots:
  void readLines();
  void done();
  void slotTimeout();

public slots:
  void keyOut(QString);

signals:
  void textIn(const QString &, bool);
  void textLine(const QString );
  void result(QStringList &, int);

private:
  void finish(int ret);

  void startSsh();
  void startSu();
  void startSudo();
  void breakUpCmd(const QString &);
  bool needSession(bool needRoot);

  kpKProcIO* pty;
  QTimer *tm;
  QStringList retList;
  QRegExp terminator;
  bool pUnterm;
  QString uptext;
  //   True if in event loop
  bool eventLoop;
  // True if trying to login
  bool noRet;
  // Do not return lines in list
  bool loginSession;
  QTextCodec *codec;
  QMap<QString, QString> passwords;
};


#endif
