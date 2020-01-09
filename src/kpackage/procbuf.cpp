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

#include "procbuf.h"
#include <k3process.h>
#include "kpackage.h"
#include <klocale.h>
#include <QtGui/QLabel>
#include <kdebug.h>

Modal::Modal(QString msg, QWidget *parent, const char * name )
  : KDialog( parent )
{
  QLabel *line1 = new QLabel(msg,this);
  line1->setAlignment(Qt::AlignCenter);
  //line1->setAutoResize(true);

 }

void Modal::terminate()
{
  done(0);
}

procbuf::procbuf()
{
  m = NULL;
  tm = new QTimer(this);
  connect(tm, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

procbuf::~procbuf()
{
}

void procbuf::setup(QString cmd)
{
  buf.truncate(0);
  proc = new K3Process();
  connect(proc, SIGNAL( receivedStdout(K3Process *, char *, int)), 
			this, SLOT(slotReadInfo(K3Process *, char *, int)));
  connect(proc, SIGNAL( receivedStderr(K3Process *, char *, int)), 
			this, SLOT(slotReadInfo(K3Process *, char *, int)));
  connect(proc, SIGNAL( processExited(K3Process *)), 
			this, SLOT(slotExited(K3Process *)));
  proc->clearArguments();
  *proc << cmd;
  command = cmd;
}

void procbuf::slotReadInfo(K3Process *, char *buffer, int buflen)
{
   char last;

   last = buffer[buflen - 1];
   buffer[buflen - 1] = 0; 

   buf += buffer;
   buf += last;

   if (timed) {
     timed =  false;
     tm->stop();
   }
}

void procbuf::slotExited(K3Process *)
{
  if (m) {
    m->terminate();
  }
  if (timed) {
    timed =  false;
    tm->stop();
  }
}

void procbuf::slotTimeout()
{
  if (m) {
    m->terminate();
  }
  //  kDebug() << "TTT\n";
}

int procbuf::start (QString  msg, bool errorDlg,
		    int timeout, QString timeMsg )
{
  if (timeout) {
    tm->start(timeout*1000);   
    timed = true;
  }

  if (!proc->start(!msg.isNull() ? K3Process::NotifyOnExit : K3Process::Block,
		   K3Process::All)) {
    if (errorDlg) {
      KpMsgE(i18n("Kprocess Failure"), true);
    }
    return 0;
  };

  if (!msg.isEmpty()) {
    m = new Modal(msg,kpkg, "wait");
    m->exec();
    delete m;
    m = 0;
  }

  kDebug() << command 
	    << " dialog=" << errorDlg 
	    << " normal=" << proc->normalExit()
	    << " exit=" << proc->exitStatus() << endl;
  if (timed) {
    kDebug() << "timeout..................\n";
    KpMsg("Error",i18n("Timeout: %1", timeMsg), true);
    delete proc; proc = 0;
    return 0;
  } else {
    if (!proc->normalExit() || proc->exitStatus()) {
      if (errorDlg) {
	KpMsg("Error",i18n("Kprocess error:%1", buf), true);
      }
      delete proc; proc = 0;
      return 0;
    }
  }
  delete proc;  proc = 0;
  return 1;
}











#include "procbuf.moc"
