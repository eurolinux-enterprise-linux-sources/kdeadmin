/***************************************************************************
    begin                : Tue Oct 5 1999
    copyright            : (C) 1999 by Peter Putzer
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

#ifndef KDLTOOLTIP_H
#define KDLTOOLTIP_H



class KSVDragList;

/**
  * @short custom tooltip for use in @ref KDragList
  * @author Peter Putzer
  */
class KDLToolTip : public QToolTip
{
public:
  KDLToolTip (KSVDragList *parent, QToolTipGroup* group = 0L);
  virtual ~KDLToolTip();

protected:
  /**
   * Reimplemented from QToolTip for internal reasons.
   */
  virtual void maybeTip (const QPoint&);

private:
  KSVDragList* mParent;
};

#endif

