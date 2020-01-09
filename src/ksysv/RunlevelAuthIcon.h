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

#ifndef RUNLEVEL_AUTH_ICON_H
#define RUNLEVEL_AUTH_ICON_H

#include "kauthicon.h"

class QTimer;
class QFileInfo;

class RunlevelAuthIcon : public KAuthIcon
{
  Q_OBJECT
  Q_PROPERTY (int refreshInterval READ refreshInterval WRITE setRefreshInterval)
    
public:
  RunlevelAuthIcon (const QString& scriptPath, const QString& runlevelPath,
					QWidget* parent = 0L, const char* name = 0L);

  virtual ~RunlevelAuthIcon ();

  virtual bool status () const;

  inline int refreshInterval () const { return mInterval; }
  inline bool isCheckEnabled () const { return mCheckEnabled; }

public slots:
  virtual void updateStatus ();

  void setServicesPath (const QString& servicesPath);
  void setRunlevelPath (const QString& runlevelPath);

  void setRefreshInterval (int);

  void setCheckEnabled(bool);

private slots:
  void timerEvent ();

private:
  QTimer* mTimer;
  QFileInfo* mServicesInfo;
  QFileInfo** mRLInfo;
  bool mOld;
  int mInterval;
  
  bool mCheckEnabled;
};

#endif // RUNLEVEL_AUTH_ICON_H
