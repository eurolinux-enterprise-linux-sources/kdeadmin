/***************************************************************************
                          ksv::IO.cpp  -  description
                             -------------------
    begin                : Sun Oct 3 1999
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

#include <errno.h>
#include <unistd.h>

#include <qfileinfo.h>
#include <q3ptrlist.h>
#include <q3valuelist.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3CString>

#include <kdebug.h>
#include <klocale.h>
#include <kmimemagic.h>
#include <kservice.h>
#include <kuserprofile.h>
#include <QDateTime>
#include "Data.h"
#include "IOCore.h"
#include "ksvdraglist.h"
#include "ksv_core.h"
#include "ksv_conf.h"

QString ksv::IO::relToAbs (const QString& dir, const QString& rel)
{
  if (rel.left(1) != "/")
    return QDir::cleanDirPath (dir + "/" + rel);
  else
    return QDir::cleanDirPath (rel);
}

void ksv::IO::removeFile (const QFileInfo& info, QDir& dir, QString& rich, QString& plain)
{
  if (!dir.remove(QDir::homePath () + info.fileName()))
	{
	  rich = (i18n ("<error>FAILED</error> to remove <cmd>%1</cmd> from <cmd>%2</cmd>: \"%3\"<br/>",
			   info.fileName(),
			   dir.path(),
			   strerror(errno)));
	  plain = (i18n ("FAILED to remove %1 from %2: \"%3\"\n",
                info.fileName(),
                dir.path(),
                strerror(errno)));
	}
  else
	{
	  rich = i18n("removed <cmd>%1</cmd> from <cmd>%2</cmd><br/>",
		 info.fileName(),
		 dir.path());
	  
	  plain = i18n("removed %1 from %2\n",
		 info.fileName(),
		 dir.path());
	}
}

void ksv::IO::dissectFilename (const QString& file, QString& base, int& nr)
{
  QString tmp = file.mid(1, file.length());

  nr = tmp.left(2).toInt();
  base = tmp.mid(2, tmp.length());
}

void ksv::IO::makeSymlink (const KSVData& data, int runlevel, bool start,
                           QString& rich, QString& plain)
{
  const QString symName = QString("%1%2%3").arg(start ? "S" : "K").arg(data.numberString()).arg(data.label());
  const QString symPath = QString("%1/rc%2.d/").arg(KSVConfig::self()->runlevelPath()).arg(runlevel);

  const QString symbol = symPath + symName;
  QString target = data.filename();
  
  if (QDir::isRelativePath(target))
    target = ksv::IO::makeRelativePath(QDir::cleanDirPath(symPath),
                                       QDir::cleanDirPath(data.path())) + data.filename();

  if (symlink(target.local8Bit(), symbol.local8Bit()) == 0)
	{
	  rich = i18n("created <cmd>%1</cmd> in <cmd>%2</cmd><br/>", symName, symPath);
	  plain = i18n("created %1 in %2\n", symName, symPath);
	}
  else
	{
	  rich = i18n("<error>FAILED</error> to create <cmd>%1</cmd> in <cmd>%2</cmd>: \"%3\"<br/>",
		 symName,
		 symPath,
		 strerror(errno));
      
	  plain = i18n("FAILED to create %1 in %2: \"%3\"\n",
		 symName,
		 symPath,
		 strerror(errno));
	}
}

QString ksv::IO::makeRelativePath (const QString& from, const QString& to)
{
  if (QDir::isRelativePath(from) || QDir::isRelativePath(to))
    return QString();
  
  int pos = 0;
  const int f_length = from.length();
  
  QStringList from_list;
  while (pos > -1)
    {
      const int old = pos + 1;
      const int res = from.find('/', old);
      
      int length = 0;

      if (res > -1)
        length = res - old + 1;
      else
        length = f_length - old;
      
      from_list.append (from.mid(old, length));
      
      pos = res;
    }

  const int t_length = to.length();
  
  QStringList to_list;
  pos = 0;
  
  while (pos > -1)
    {
      const int old = pos + 1;
      const int res = to.find('/', old);
      
      int length = 0;
      
      if (res > -1)
        length = res - old + 1;
      else
        length = t_length - old;
      
      to_list.append (to.mid(old, length));
      
      pos = res;
    }
  
  int lcp = 0; // longest common prefix
  const int f_c = from_list.count();
  const int t_c = to_list.count();
  
  while (lcp < f_c && lcp < t_c
         && from_list.at(lcp) == to_list.at(lcp))
    lcp++;
  
  QString result;
  for (int i = f_c - lcp; i > 0; --i)
    result += "../";

  for (int i = lcp; i < t_c; ++i)
    result += to_list.at(i) + "/";
  
  return result;
}

bool ksv::IO::loadSavedConfiguration (QDataStream& s,
                                      Q3ValueList<KSVData>* start,
                                      Q3ValueList<KSVData>* stop)
{
  Q3CString magic;
  s >> magic;
  if (magic != "KSysV")
    return false;
  
  qint32 version = 0;
  s >> version;

  if (version != 3)
    return false; // too old

  QDateTime saveTime;
  s >> saveTime;

  for (int i = 0; i < ksv::runlevelNumber; ++i)
    {
      QString rlMagic;
      QString section;
      
      s >> rlMagic;
      s >> section;

      if (rlMagic != QString::fromLatin1("RUNLEVEL %1").arg(i))
        return false;
      
      if (section != "START")
        return false;

      qint32 numberOfItems;
      s >> numberOfItems;
      
      KSVData data;
      for (int j = 0; j < numberOfItems; ++j)
        {
          s >> data;
          start[i].append (data);
        }
      
      s >> section;
      if (section != "STOP")
        return false;

      s >> numberOfItems;
      for (int j = 0; j < numberOfItems; ++j)
        {
          s >> data;
          stop[i].append(data);
        }
    }
  
  return true;
}

bool ksv::IO::saveConfiguration (QDataStream& s,
                                 KSVDragList** start,
                                 KSVDragList** stop)
{
  qint32 version = 3;

  s << Q3CString("KSysV")
    << version
    << QDateTime::currentDateTime(); // save date

  for (int i = 0; i < ksv::runlevelNumber; ++i)
    {
      qint32 numberOfItems = start[i]->childCount();

      s << QString::fromLatin1 ("RUNLEVEL %1").arg (i)
        << QString::fromLatin1 ("START")
        << numberOfItems;
      
      for (Q3ListViewItemIterator it (start[i]); 
           it.current();
           ++it)
        {
          s << *static_cast<KSVItem*> (it.current())->data();
        }

      numberOfItems = stop[i]->childCount();

      s << QString::fromLatin1 ("STOP")
        << numberOfItems;

      for (Q3ListViewItemIterator it (stop[i]); 
           it.current();
           ++it)
        {
          s << *static_cast<KSVItem*> (it.current())->data();
        }
    }

  return true;
}

KMimeTypeTrader::OfferList ksv::IO::servicesForFile (const QString& filename)
{
  static KTrader* trader = KTrader::self();
  static KMimeMagic* magic = KMimeMagic::self();
  const QString mimetype = magic->findFileType(filename)->mimeType();

  return trader->query (mimetype, "Type == 'Application'");
}

KService::Ptr ksv::IO::preferredServiceForFile (const QString& filename)
{
  static KMimeMagic* magic = KMimeMagic::self();
  const QString mimetype = magic->findFileType(filename)->mimeType();
  
  return KServiceTypeProfile::preferredService (mimetype, "Application");
}
