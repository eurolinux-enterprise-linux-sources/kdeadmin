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

#ifndef KSV_CORE_H
#define KSV_CORE_H

class QPushButton;
class Q3StyleSheet;
class KAboutData;
class KCompletion;
class KSVItem;

namespace ksv
{
  bool getServiceDescription (const QString& path, QString& res);

  QString breakWords (const QString& s, int amount);

  /**
   * \return the Unicode string for the (c) symbol.
   */
  const QString& copyrightSymbol ();

  Q3StyleSheet* styleSheet ();

  const QString& logFileFilter ();
  const QString& nativeFileFilter ();

  const QString& logFileExtension ();
  const QString& nativeFileExtension ();

  KCompletion* serviceCompletion ();
  KCompletion* numberCompletion ();

  extern const int runlevelNumber;
  extern KAboutData* about;

  enum Messages {
    RunlevelsReadOnly = 0,
    CouldNotGenerateSortingNumber
  };
  
  extern const char* notifications[];
}

#endif // KSV_CORE_H

