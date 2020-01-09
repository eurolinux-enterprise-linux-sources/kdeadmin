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

#ifndef KSV_TRASH_H
#define KSV_TRASH_H

#include <q3frame.h>
#include <qpixmap.h>

//Added by qt3to4:
#include <QLabel>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QEvent>
#include <QDropEvent>

// forward declarations
class QLabel;
class KIconLoader;
class KSVItem;
class KSVAction;

class KSVTrash : public Q3Frame
{
  Q_OBJECT
  
public:
  KSVTrash (QWidget* parent = 0, const char* name = 0);
  virtual ~KSVTrash();
  
  virtual QSize sizeHint() const;

protected:
  /**
   * Overridden from @ref QDropSite
   */
  virtual void dragMoveEvent ( QDragMoveEvent* );
  
  /**
   * Overridden from @ref QDropSite
   */
  virtual void dragLeaveEvent ( QDragLeaveEvent* );

  /**
   * Overridden from @ref QDropSite
   */
  virtual void dropEvent ( QDropEvent* );

  virtual bool eventFilter ( QObject*, QEvent* );

private:
  KIconLoader* mKIL;
  QLabel* mLabel;
  bool mOpen;
  int mPixmapWidth;
  
signals:
  void undoAction (KSVAction*);
};

#endif
