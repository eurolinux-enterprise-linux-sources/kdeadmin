/***************************************************************************
    begin                : Tue Oct 5 1999
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

#include <qrect.h>
#include <qscrollbar.h>
#include <q3header.h>

#include <kdebug.h>

#include "ksvdraglist.h"
#include "kdltooltip.h"

KDLToolTip::KDLToolTip (KSVDragList *parent, QToolTipGroup* group)
  : QToolTip(parent, group),
	mParent (parent)
{
}

KDLToolTip::~KDLToolTip()
{
}

void KDLToolTip::maybeTip (const QPoint& p)
{
  if (!mParent->displayToolTips())
	return;

  QString text;
  QRect rect;

  const QRect vert = mParent->verticalScrollBar()->geometry();
  const QRect horiz = mParent->horizontalScrollBar()->geometry();

  if (vert.contains(p))
	{
	  rect = vert;
	  
	  if (!mParent->commonToolTips())
		text = mParent->verticalScrollBarTip();
	  else
		text = mParent->tooltip();
	}
  else if (horiz.contains(p))
	{
	  rect = horiz;
	  if (!mParent->commonToolTips())
		text = mParent->horizontalScrollBarTip();
	  else
		text = mParent->tooltip();
	  
	}
  else
	{ 
	  QPoint rp = mParent->viewport()->mapFromParent (p);
	  Q3ListViewItem* i = mParent->itemAt (rp);
	  KSVItem* item = static_cast<KSVItem*> (i);
	  	
	  rect = mParent->header()->geometry();
      if (rect.contains (p))
        {
		  text = mParent->tooltip();
        }
      else if (item)
		{
		  rect = mParent->itemRect (i);
		  rect.moveTopLeft (mParent->viewport()->mapToParent (rect.topLeft()));

		  text = item->tooltip();
		}
	  else
		{
          rect = mParent->rect();

          Q3ListViewItem* last = mParent->lastItem();          
          if (last)
            rect.setTop (mParent->viewport()->mapToParent (mParent->itemRect(last).bottomRight()).y());
          
		  text = mParent->tooltip();
		}
	}
  
  if (!text.isEmpty())
	tip (rect, text);
}
