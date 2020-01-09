/***************************************************************************
    begin                : Sun Oct 3 1999
    copyright            : (C) 1997-99 by Peter Putzer
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

#include <QPushButton>
#include <q3stylesheet.h>

#include <kglobalsettings.h>
#include <kcompletion.h>
#include <klocale.h>
#include <kglobal.h>

#include "ksv_conf.h"
#include "ksvdraglist.h"
#include "ksv_service.h"
#include "ksv_core.h"

const int ksv::runlevelNumber = 7;
KAboutData* ksv::about = 0L;

bool ksv::getServiceDescription (const QString& path, QString& res)
{
  KSVService* tmp = KSVService::newService (path, KSVConfig::self()->runlevelPath());
  res = i18n ("No description available.");
  bool result = false;

  if (tmp)
    {
      res = tmp->description();
      result = true;
    }

  delete tmp;

  return result;
}

QString ksv::breakWords (const QString& s, int amount)
{
  QString res;

  int left = s.length();
  int maximum = amount;
  QString str = s;

  while (left > maximum)
    {
      int pos = -1;
      for (int i = 1; i < amount; ++i)
        {
          if ((pos = str.find(' ', amount - i, false)) < 0 || (pos > maximum + i))
            continue;
          else
            break;
        }
      
      if (pos == -1) break;  //handles case when no spaces
      
      maximum = qMax (maximum, pos);
      res += str.left(pos) + "\n";
      left += -pos - 1;
      str = str.right(left);
    }
  
  res += str;
  
  return res;
}

const QString& ksv::copyrightSymbol ()
{
  static QString c = QString::fromUtf8 ("©");

  return c;
}

Q3StyleSheet* ksv::styleSheet ()
{
  static Q3StyleSheet style;
  static bool initialized = false;

  if (!initialized)
	{
	  Q3StyleSheetItem* item = new Q3StyleSheetItem (&style, "vip"); // very important
	  item->setLogicalFontSize (5);
	  item->setFontWeight (QFont::Bold);
      item->setDisplayMode (Q3StyleSheetItem::DisplayBlock);

	  item = new Q3StyleSheetItem (&style, "rl"); // runlevel
	  item->setLogicalFontSize (4);
	  item->setFontWeight (QFont::Bold);
//       item->setDisplayMode (QStyleSheetItem::DisplayBlock);

	  item = new Q3StyleSheetItem (&style, "start"); // start section
	  item->setColor (Qt::green);
//       item->setContexts ("rl");

	  item = new Q3StyleSheetItem (&style, "stop"); // stop section
	  item->setColor (Qt::red);
//       item->setContexts ("rl");

	  item = new Q3StyleSheetItem (&style, "error"); // signal an error
	  item->setColor (Qt::red);
	  item->setLogicalFontSizeStep (1);

	  item = new Q3StyleSheetItem (&style, "cmd"); // command line
	  item->setFontFamily (KGlobalSettings::fixedFont().family());
	}

  return &style;
}

KCompletion* ksv::serviceCompletion ()
{
  static KCompletion comp;  

  return &comp;
}

KCompletion* ksv::numberCompletion ()
{
  static KCompletion comp;
  static bool initialized = false;
  
  if (!initialized)
    {
      for (int value = 0; value < 100; value += 10)
        {
          QString result = QString::number (value);

          if (value < 10)
            result.sprintf("%.2i", value);
          
          comp.addItem (result);
        }
    }

  return &comp;
}

const QString& ksv::logFileFilter ()
{
  static QString filter = "*" + ksv::logFileExtension() + "|" +
    QString(KGlobal::caption() + i18n(" log files") + " (*" + ksv::logFileExtension() + ")");
  
  return filter;
}

const QString& ksv::nativeFileFilter ()
{
  static QString filter = "*" + ksv::nativeFileExtension() + "|"
    + i18n("Saved Init Configurations") + " (*" + ksv::nativeFileExtension() + ")";
  
  return filter;
}

const QString& ksv::logFileExtension ()
{
  static QString ext = ".ksysv_log";
  
  return ext;
}

const QString& ksv::nativeFileExtension ()
{
  static QString ext = ".ksysv";
  
  return ext;
}

const char* ksv::notifications[] =
{
  "Show Runlevels ReadOnly",
  "Show Could Not Generate Sorting Number"
};
