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

#ifndef KSV_CONF_H
#define KSV_CONF_H


#include <QPoint>
#include <QFont>
#include <QColor>
#include <QMap>

#include "ksv_core.h"

class KConfig;

class KSVConfig
{
public:
  
  inline ~KSVConfig() {}

  void readSettings();
  void readLegacySettings ();
  void writeSettings();

  void setPanningFactor (int val);

  inline void setShowLog( bool val = false )
  {
    mShowLog = val;
  }

  void setNewNormalColor (const QColor& color);
  void setNewSelectedColor (const QColor& color);
  void setChangedNormalColor (const QColor& color);
  void setChangedSelectedColor (const QColor& color);

  void setServiceFont (const QFont& font);
  void setNumberFont (const QFont& font);

  void setScriptPath (const QString& path);
  void setRunlevelPath (const QString& path);

  inline void setConfigured( bool val = true )
  {
    mConfigured = val;
  }

  inline void setShowDescription (bool val)
  {
	mShowDescription = val;
  }

  inline bool showLog() const
  {
    return mShowLog;
  }

  inline int panningFactor() const
  {
    return mPanningFactor;
  }

  inline const QFont& serviceFont () const { return mServiceFont; }
  inline const QFont& numberFont () const { return mNumberFont; }

  QPoint position() const;

  inline const QColor& newNormalColor () const { return mNewNormalColor; }
  inline const QColor& newSelectedColor () const { return mNewSelectedColor; }

  inline const QColor& changedNormalColor () const { return mChangedNormalColor; }
  inline const QColor& changedSelectedColor () const { return mChangedSelectedColor; }
  
  bool showRunlevel (int index) const;
  void setShowRunlevel (int index, bool state);
  void readRunlevels ();
  void writeRunlevels ();

  /**
   * Have the necessary config entries
   * been written?
   */
  inline bool isConfigured() const
  {
    return mConfigured;
  }

  inline const QString& scriptPath() const
  {
    return mScriptPath;
  }

  inline const QString& runlevelPath() const
  {
    return mRunlevelPath;
  }

  inline bool showDescription() const
  {
	return mShowDescription;
  }

  bool showMessage (ksv::Messages msg) const;
  void setShowMessage (ksv::Messages msg, bool on);

  static KSVConfig* self ();

private:
  KSVConfig ();

  QString mScriptPath;
  QString mRunlevelPath;
  bool mShowLog;
  bool mConfigured;
  KConfig* mConfig;
  int mPanningFactor;

  QColor mNewNormalColor, mNewSelectedColor;
  QColor mChangedNormalColor, mChangedSelectedColor;

  bool mShowDescription;

  QMap<int, bool> mShowRunlevel;

  QFont mServiceFont;
  QFont mNumberFont;
};

#endif // KSV_CONF_H

