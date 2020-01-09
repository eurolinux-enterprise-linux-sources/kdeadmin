/***************************************************************************
                          ksv::IO.h  -  description
                             -------------------
    begin                : Sun Oct 3 1999
    copyright            : (C) 1997-99 by Peter Putzer
    email                : putzer@kde.org
 ***************************************************************************/

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

#ifndef KSV_IOCORE_H
#define KSV_IOCORE_H

#include <QDir>

//Added by qt3to4:
#include <Q3ValueList>

#include <kservice.h>

template<class T> class Q3ValueList;
class QFileInfo;
class QDataStream;

class KSVDragList;
class KSVData;

namespace ksv
{
  namespace IO
  {
    void removeFile (const QFileInfo& info, QDir& dir,
                     QString& rich, QString& plain);

    QString relToAbs (const QString& dir, const QString& rel);

    void makeSymlink (const KSVData& data, int runlevel, bool start,
                      QString& rich, QString& plain);

    void dissectFilename (const QString& file, QString& name, int& nr);

    QString makeRelativePath (const QString& from, const QString& to);

    bool saveConfiguration (QDataStream&,
                            KSVDragList** start,
                            KSVDragList** stop);

    bool loadSavedConfiguration (QDataStream&,
                                 Q3ValueList<KSVData>* start,
                                 Q3ValueList<KSVData>* stop);

    KMimetypeTrader::OfferList servicesForFile (const QString& filename);
    KService::Ptr preferredServiceForFile (const QString& filename);

  } // namespace IO
} // namespace ksv

#endif

