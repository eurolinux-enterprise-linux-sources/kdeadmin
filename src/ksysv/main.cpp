 /***************************************************************************
    copyright            : (C) 1997-2000 by Peter Putzer
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

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kuniqueapplication.h>

#include "ksv_conf.h"
#include "ksvconfigwizard.h"
#include "TopWidget.h"
#include "version.h"

using namespace std;

#if (defined(DEBUG) && !(defined(NDEBUG) || defined(NO_DEBUG)))
#define KSYSV_DEBUG(x)  x
#else
#define KSYSV_DEBUG(x)  do { } while (0)
#endif

static void myMessageOutput( QtMsgType type, const char *msg )
{
  switch ( type ) {
  case QtDebugMsg:
    KSYSV_DEBUG(cerr << "Debug: " << msg << endl);
    break;
  case QtWarningMsg:
    KSYSV_DEBUG(cerr << "Warning: " << msg << endl);
    break;
  case QtFatalMsg:
    cerr << "Fatal: " << msg << endl;
    abort(); // dump core on purpose
  }
}

int main( int argc, char **argv ) {
  // install own message handler that ignores debug-msg when DEBUG is not defined
  qInstallMsgHandler(myMessageOutput);

  KAboutData about("ksysv", 0, ki18n("SysV-Init Editor"), KSYSV_VERSION_STRING,
				   ki18n ("Editor for Sys-V like init configurations"),
				   KAboutData::License_GPL,
				   ki18n("Copyright (c) 1997-2000, Peter Putzer."),
				   ki18n ("Similar to Red Hat's" \
                              "\"tksysv\", but SysV-Init Editor allows\n" \
			      "drag-and-drop, as well as keyboard use."));
  about.addAuthor (ki18n("Peter Putzer"), ki18n("Main developer"), "putzer@kde.org");
  ksv::about = &about;

  KCmdLineArgs::init(argc, argv, &about);
  KUniqueApplication::addCmdLineOptions ();

  if (!KUniqueApplication::start()) {
    cerr << "SysV-Init Editor is already running!" << endl;
    return -1;
  }

  KUniqueApplication app;

  // session-management
  if (kapp->isSessionRestored())
    RESTORE(KSVTopLevel)
  else
    {
      KSVConfig* conf = KSVConfig::self();
      if (!conf->isConfigured())
        {
          KSVConfigWizard* w = new KSVConfigWizard(0, "ConfigWizard", true);
          w->exec();

          conf->setConfigured(true);
          conf->setRunlevelPath (w->runlevelPath());
          conf->setScriptPath (w->servicesPath());
          conf->writeSettings();
        }

      KSVTopLevel* top = new KSVTopLevel();
      app.setMainWidget(top);
      top->show();
    }
  // end session-management

  return app.exec();
}

