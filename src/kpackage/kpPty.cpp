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


#include <QtCore/QTimer>
#include <QtCore/QRegExp>
//Added by qt3to4:
#include <QtCore/QTextCodec>

#include <k3processcontroller.h>
#include <kpty.h>
#include <kdebug.h>
#include <kpassworddialog.h>
#include <kapplication.h>

#include <kpPty.h>
#include <kpackage.h>
#include <kpTerm.h>
#include <kpSettings.h>
#include <kpackageSettings.h>
#include <utils.h>

#define SHPROMPT "# "
const int TIMEOUT = -33;
const int PASSWORD = -2000;
const int PROMPT = -1000;

//////////////////////////////////////////////////////////////////////////////

kpKProcIO::kpKProcIO ( QTextCodec *_codec)
  : K3ProcIO(_codec)
{
}

kpKProcIO::~kpKProcIO()
{
}

bool kpKProcIO::sstart (RunMode runmode)
{

  connect (this, SIGNAL (receivedStdout (K3Process *, char *, int)),
	   this, SLOT (received (K3Process *, char *, int)));


  connect (this, SIGNAL (wroteStdin(K3Process *)),
	   this, SLOT (sent (K3Process *)));

  return K3Process::start (runmode,( K3Process::Communication) ( K3Process::Stdin |  K3Process::Stdout));
}

//////////////////////////////////////////////////////////////////////////////

kpPty::kpPty() : QObject()
{
  QTextCodec *codec = QTextCodec::codecForName("UTF-8");
  pty = new kpKProcIO(codec);
  pty->setUsePty(K3Process::All, false);


  connect(pty, SIGNAL(readReady(K3ProcIO *)), this,
		   SLOT(readLines()));
  connect(pty, SIGNAL(processExited(K3Process *)), this,
		   SLOT(done()));
  pty->pty()->setWinSize(0,80);
  tm = new QTimer(this);
  connect(tm, SIGNAL(timeout()), this, SLOT(slotTimeout()));

  eventLoop = false;
  inSession = false;
  pUnterm = false;
  loginSession = false;
  
  codec = QTextCodec::codecForLocale();
}


kpPty::~kpPty()
{
}

void kpPty::startSu()
{
  kDebug() << "startSu()\n";
  pty->setEnvironment("PS1", SHPROMPT);
  (*pty) << "su" << "-s" << "/bin/sh";
}

void kpPty::startSudo()
{
  kDebug() << "startSudo()\n";
  pty->setEnvironment("PS1", SHPROMPT);
  (*pty) << "sudo" << "-p" << "Password: " << "/bin/sh";
}

void kpPty::startSsh()
{
  kDebug() << "startSsh()\n";
  (*pty) << "/usr/bin/ssh" << "-t" << "-l" << "root";
  if (hostName.isEmpty()) {
    (*pty) << "-o" << "StrictHostKeyChecking=no" << "localhost";
  } else {
    (*pty) << hostName;
  }
  (*pty) << "env PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin PS1='" SHPROMPT "' sh";
}

bool kpPty::needSession(bool needRoot) 
{
  return (!hostName.isEmpty() || needRoot);
}

bool kpPty::startSession(bool needRoot)
{
  bool interact = false; // Have interacted with user, prevents loops
  bool passwordTried = false; // Have tried the current save password, so need to put up dialog
  pUnterm = false;
  kDebug() << "kpPty::startSession\n";
  if (!inSession && needSession(needRoot)) {
    // Assume !needRoot actions are simple executables
    kDebug() << "kpPty::startSession TRUE\n";
    loginSession = true;
    int ret;
    QString s = "echo START=$?\n";

  FULL_RESTART:
    interact = false;
    retList.clear();
    pty->resetAll();

    QString passMsg;
    kDebug() << "privCmd=" << kpkg->conf->privCommand << " " << Settings::EnumPrivCommand::sudo << " " << Settings::EnumPrivCommand::su << "\n";
    if (kpkg->conf->privCommand == Settings::EnumPrivCommand::ssh || !hostName.isEmpty()) {
      passMsg = i18n("The action you requested uses ssh. Please enter the password or pass phrase.\n");
      startSsh();
    } else if (kpkg->conf->privCommand == Settings::EnumPrivCommand::su) {
      passMsg = i18n("The action you requested needs root privileges. Please enter root's password.\n");
      startSu();
    } else if (kpkg->conf->privCommand == Settings::EnumPrivCommand::sudo) {
      passMsg = i18n("The action you requested needs root privileges. Please enter your password.\n");
      startSudo();
    } else {
      kDebug() << "privCommand Invalid\n";
    }
    pty->sstart(K3Process::NotifyOnExit);

  RESTART:
    tm->start(6*1000);
    eventLoop = true;
    kDebug() << "Loopst\n";
    kapp->enter_loop();
    kDebug() << "Loopfn Result=" << Result  <<  "\n";
    tm->stop();
    if (Result == TIMEOUT) { // timeout
      interact = true;
      //      kDebug() << "Line=" << retList.last() << "\n";
      kpstart->addText(retList);
      kpstart->run("", i18n("Login Problem: Please login manually"));

      ret = kpstart->exec();
      kDebug() << "Sret=" << ret << "\n";
      if (ret) {
	inSession = false;
      } else {
	inSession = true;
      }
    } else if (Result == PASSWORD) {  // We got a password prompt
      QString pass;
      int res;
      interact = true;
      kDebug() << "H=" << hostName << " PT=" << passwordTried <<"\n";

	  if (passwords[hostName].count()!=0 && !passwordTried) {
        pass = passwords[hostName];
        res = 1;
      } else {
        kDebug() << "Passwd=" << retList.last() << "\n";
        QString msg = passMsg;
	if (kpkg->conf->privCommand == Settings::EnumPrivCommand::ssh || !hostName.isEmpty()) {
          msg += retList.last();
        }
        KPasswordDialog dlg(0 , KPasswordDialog::ShowKeepPassword);
        dlg.setPrompt( msg );
        res = dlg.exec();
        pass = dlg.password();
//            kDebug() << "Pass=" << pass <<  " Res=" << res << "\n";
        if (res && dlg.keepPassword()) {
          passwords[hostName] = pass;
        } else if(res) {
          passwords.remove(hostName);
        }
      }
      pty->writeStdin(pass.append("\n"), false);
      passwordTried = true;
      if (res) {
        retList.clear();
        goto RESTART;
      } else {
        inSession = false;
      }
    } else if (Result == PROMPT) { // Got Prompt
      inSession = true;
      kDebug() << "kpPty::startSession TRUE\n";
    } else {  // process return code
      pty->writeStdin(QByteArray("\04"), false);  // SU doesn't listen to ^C
      if (interact) {
	goto FULL_RESTART;
      } else {
	QString errMsg = retList.join(" ");
	KpMsgE(errMsg, true);
        inSession = false;
      }
    }
  } else {
    kDebug() << "kpPty::startSession Not needed\n";
  }
  
  loginSession = false;
  if (!inSession)
    close();
  
  return inSession;
}

void kpPty::breakUpCmd(const QString &cmd)
{
  kDebug() << " kpPty::run CMD=\""<< cmd <<"\" pty = " << pty << "\n";

  bool quote = false;
  QString s;
  QStringList cl = cmd.split(" ", QString::SkipEmptyParts);

  for ( QStringList::Iterator it = cl.begin(); it != cl.end(); ++it ) {
//    kDebug() << "S=" << *it << "\n";
    int lastPt = (*it).length() - 1;
    if ((*it)[0] == '\'') { // Start of quoted string
      s = *it;
      if ((*it)[lastPt] == '\'') { // Also End of quoted string
	s.replace("'","");
	(*pty) << s;
	quote = false;
      } else {
	s += ' ';
	quote = true;
      }
    } else if ((*it)[lastPt] == '\'') { // End of quoted string
      s += *it;
      s.replace("'","");
      (*pty) << s;
      quote = false;
    }	else if (quote) {
      s += *it;
      s += ' ';
    } else {
      (*pty) << (*it);
    }
  }
}

void kpPty::listClear() 
{
  retList.clear();
}

QStringList kpPty::run(const QString &cmd, bool inLoop, bool needRoot, bool noReturn)
{
  Result = 0;

  pUnterm = false;
  noRet = noReturn;

  if (!inSession && !needSession(needRoot)) {
    // Assume !needRoot actions are simple executables
    pty->resetAll();
    breakUpCmd(cmd);
    pty->setEnvironment("TERM", "dumb");
    if (!pty->sstart(K3Process::NotifyOnExit)) {
      kDebug() << " kpPty::run execute=0\n";
      return QStringList();
    }
  } else {
    if (startSession(needRoot)) {
      kDebug() << "CMDroot='"<< cmd <<"'\n";
      QString s = cmd + ";echo RESULT=$?";
      pty->writeStdin(s);
      kDebug() << " kpPty::run session\n";
    } else {
      kDebug() << " kpPty::run other=0\n";
      return QStringList();
    }
  }
  retList.clear();

  if (inLoop) {
    eventLoop = true;
    kapp->enter_loop();
    kDebug() << "return Result=" << Result << " " << retList.count() << "\n";
    return retList;
  } else {
    return QStringList();
  }
}

void kpPty::close() {
  //  kDebug() << "kpPty::close\n";

  pty->closeAll();
  while(pty->isRunning()) {
    K3ProcessController::instance()->waitForProcessExit(1);
  }
  inSession = false;
}

void kpPty::finish(int ret)
{
//   kDebug() << "kpPty::finish " << ret << " " << retList.count() << "\n";

  QStringList::Iterator l;

  if (ret == PROMPT) {  // Called program executed in session
   if (!retList.empty()) {
      l = retList.fromLast();
      if ((*l).right(2) == SHPROMPT) {
	retList.erase(l);                  // Remove prompt
      }
    }

    if (!retList.empty()) {
      int p;
      l = retList.fromLast();
//      kDebug() << "kpPty::finish RESULT=" << *l << "\n";
      if ((p = (*l).indexOf("RESULT=")) >= 0) {
	ret = (*l).mid(p+7).toInt(0,10);
	retList.erase(l);                  // Remove return code
      } 
    }

    if (!retList.empty()) {
      l = retList.begin();
      if ( l !=  retList.end()) {
	if ((*l).indexOf("RESULT=") >= 0) {
	  retList.erase(l);                  // Remove command at start
	}
      }
    }
  } 
  emit result(retList,ret);
 
//  kDebug() << "Result=" << Result << " ret=" << ret <<"\n";
  Result = ret;   

  if (eventLoop) {
    eventLoop = false;
    kapp->exit_loop();
  }
}

void kpPty::readLines()
{
  bool unterm = false;

  QString stext;
  while(pty->readln(stext, false, &unterm) >= 0)
  {
   //         kDebug() << "[=" << pUnterm << "-" << unterm << "=" << stext.length() << "-" << stext << ">\n";
    
    emit textIn(stext, !unterm);
   
    if (pUnterm) {
      QStringList::Iterator lst;
      lst = retList.fromLast();
      if (lst != retList.end())
      {
        stext = *lst + stext;
        retList.erase(lst);
      }
    } 
    
    if (!unterm) 
	emit textLine(stext);
    
    int i;
    if (!unterm) {
      while (stext.endsWith("\r")) {
          stext.truncate(stext.length()-1);
      }
       
      i = stext.lastIndexOf('\r');
      if (i > -1) {
        stext = stext.mid(i+1);
      }
    }

    pUnterm = unterm;

    if (!noRet || unterm)
      retList << stext;
    
 //   kDebug() << "++" << stext << "\n";
    if (stext.right(2) == SHPROMPT) {  // Shell prompt
      emit textIn("\r \n", false);
      finish(PROMPT);
    } else if (loginSession)  {
      QRegExp rx( "^[^:]+:[\\s]*$");  // Password prompt
      if (rx.indexIn(retList.last()) >= 0) {
        kDebug() << loginSession << " " <<retList.last()<< " Match password p\n";
	finish(PASSWORD);
      }
    }
  }
  pty->ackRead();
}

void kpPty::keyOut(QString s)
{
 pty->writeStdin(s, false);
}

void kpPty::done()
{
  int ret = pty->exitStatus();
  QString stext;

  finish(ret);
}

void kpPty::slotTimeout()
{
  kDebug() << "Timeout..............\n";
  finish(TIMEOUT);
}
#include "kpPty.moc"
