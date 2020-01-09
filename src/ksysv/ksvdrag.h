/*
   Copyright 2000 Peter Putzer <putzer@kde.org>

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

#ifndef KSVDRAG_H
#define KSVDRAG_H

#include <q3dragobject.h>

class KSVData;
class KSVItem;
class QWidget;

class KSVDrag : public Q3DragObject
{
  Q_OBJECT

public:
  KSVDrag (const KSVData& item, QWidget* dragSource = 0L, const char* name = 0L);
  KSVDrag (const KSVItem& item, QWidget* dragSource = 0L, const char* name = 0L);
  virtual ~KSVDrag();

  virtual const char* format (int i) const;
  QByteArray encodedData (const char*) const;
  
  static bool decodeNative (const QMimeSource*, KSVData&);
  
private:
  enum
  {
	Native, Text, URL
  };

  class Private;
  Private* d;
};

#endif // KSVDRAG_H
