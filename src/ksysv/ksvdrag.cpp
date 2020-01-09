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

#include <q3cstring.h>

#include <kurl.h>

#include "Data.h"
#include "ksvdraglist.h"
#include "ksvdrag.h"

class KSVDrag::Private
{
public:
  QByteArray mNative;
  QString mText;
  KUrl mURL;
};

KSVDrag::KSVDrag (const KSVData& item, QWidget* source, const char* name)
  : Q3DragObject (source, name),
    d (new Private())
{
  QDataStream ds (&d->mNative, QIODevice::ReadWrite);
  ds << item;

  d->mText = item.filenameAndPath ();
  d->mURL.setPath (item.path() + "/" + item.filename());
}

KSVDrag::KSVDrag (const KSVItem& item, QWidget* source, const char* name)
  : Q3DragObject (source, name),
    d (new Private())
{
  QDataStream ds (&d->mNative, QIODevice::ReadWrite);
  ds << *item.data();

  d->mText = item.toString();
  d->mURL.setPath (item.path() + "/" + item.filename());
}

KSVDrag::~KSVDrag ()
{
  delete d;
}

const char* KSVDrag::format (int i) const
{
  switch (i)
	{
	case Native:
	  return "application/x-ksysv";
	  break;
	  
	case Text:
	  return "text/plain";
	  break;
	  
	case URL:
	  return "text/uri-list";
	  break;
	  
	default:
	  return 0L;
	}
}

QByteArray KSVDrag::encodedData (const char* format) const
{
  QByteArray res;

  if (!strcmp (format, "application/x-ksysv"))
    {
      res = d->mNative;
    }
  else if (!strcmp (format, "text/plain"))
    {
      QDataStream ds (&res, QIODevice::ReadWrite);
	  ds << d->mText;
    }
  else if (!strcmp (format, "text/uri-list"))
    {
      res = Q3CString(d->mURL.url().latin1()).copy();
    }

  return res;
}

bool KSVDrag::decodeNative (const QMimeSource* mime, KSVData& data)
{
  if (mime && mime->provides ("application/x-ksysv"))
	{
	  QDataStream ds (&mime->encodedData ("application/x-ksysv"), QIODevice::ReadOnly);
	  ds >> data;

	  return true;
	}

  return false;
}

#include "ksvdrag.moc"
