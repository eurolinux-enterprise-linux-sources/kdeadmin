/***************************************************************************
    begin                : Sun Oct 3 1999
    copyright            : (C) 1999 by Peter Putzer
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

#include <ksv_service.h>
#include "leveldb.h"

KSVService::KSVService (const QString &name, const QString &basedir)
  : name_ (name),
    base_ (basedir)
{

}

KSVService* KSVService::newService (const QString &name, const QString &basedir)
{
  struct service service;

  int result = ::readServiceInfo (basedir.local8Bit(), name.local8Bit(), &service );

  if (!result)
    {
      KSVService* tmp = new KSVService(name, basedir);
      tmp->desc_ = service.desc;
      tmp->levels = service.levels;
      tmp->kPriority = service.kPriority;
      tmp->sPriority = service.sPriority;

      return tmp;
    } 
  else
    {
      return 0L;
    }
}

KSVService::~KSVService()
{
}

bool KSVService::isOn (int level) const
{
  return !::isOn (base_.local8Bit(), name_.local8Bit(), level );
}

bool KSVService::isConfigured (int level) const
{
  return !::isConfigured (base_.local8Bit(), name_.local8Bit(), level);
}

int KSVService::set (int level, bool on)
{
  struct service service;
  service.name = strdup (name_.local8Bit());
  service.desc = strdup (desc_.local8Bit());
  service.levels = levels;
  service.kPriority = kPriority;
  service.sPriority = sPriority;

  return ::doSetService ("/etc/rc.d", service, level, on);
}

QString KSVService::description () const
{
  return desc_.trimmed();
}
