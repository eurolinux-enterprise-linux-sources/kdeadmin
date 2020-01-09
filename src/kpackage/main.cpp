/*
** Copyright (C) 1999,2000 Toivo Pedaste <toivo@ucs.uwa.edu.au>
**
// Author: Toivo Pedaste
//
// This is the entry point to the program
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

#include <stdlib.h>

#include "kpackage.h"

#include <QtCore/QFile>

#include <kapplication.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <kaboutdata.h>
#include <kdebug.h>

#include <kpTerm.h>
#include "debInterface.h"
#include "rpmInterface.h"

static const char description[] =
	I18N_NOOP("KDE Package installer");

// Globals
KPKG *kpkg;
KPACKAGE *kpackage;
kpPty *kpty;
kpRun *kprun;
kpRun *kpstart;

QString hostName;

pkgInterface *kpinterface[3];
 
int main(int argc, char **argv)
{
  KAboutData aboutData( "kpackage", 0, ki18n("KPackage"), 
			KDE_VERSION_STRING, ki18n(description), KAboutData::License_GPL, 
			ki18n(//    KDE_VERSION_STRING, description, 0, 
			"(c) 1999-2001, Toivo Pedaste"));
  KCmdLineArgs::init( argc, argv, &aboutData );
  aboutData.addAuthor( ki18n("Toivo Pedaste"),KLocalizedString(), "toivo@ucs.uwa.edu.au");

  KCmdLineOptions options;
  options.add("remote ", ki18n("Remote host for Debian APT, via SSH"));
  options.add("r ");
  options.add("+[Package]", ki18n("Package to install"));
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KApplication app;
  
  kpkg = 0;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString remoteHost = args->getOption("remote");

  if (remoteHost.isEmpty() || remoteHost == "localhost") {
    hostName = "";
  } else {
    hostName = remoteHost;
  }

  kpty = new kpPty();
  kprun = new kpRun();
  kpstart = new kpRun();

  kpinterface[0] = new pkgInterface;
  kpinterface[DEBt] = new DEB;
  kpinterface[RPMt] = new RPM;
  
  if ( app.isSessionRestored() ) {
    if (KPKG::canBeRestored(1)) {
      kpkg =  new KPKG(KGlobal::config());  
      kpkg->restore(1);
    }
  } else {
    // Create the main widget and show it
    kpkg = new KPKG(KGlobal::config());
    kpkg->show();
  }
  kpkg->setCaption(hostName);

  if (args->count()) {		// an argument has been given
    QStringList files;
    for(int i = 0; i < args->count(); i++) {
      files.append(args->url(i).url());
    }
    kpackage->openNetFiles(files, false); 
  }  else {	
    if (!kpkg->prop_restart)
      kpackage->setup();
  }
  
  args->clear();

  int r = app.exec();		// execute the application

  return r;			// return the result
}

