/***************************************************************************
    begin                : Sun Oct 3 1999
    copyright            : (C) 1997-2000 by Peter Putzer
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

// Trash Can


#include <QLabel>
#include <qpainter.h>
//Added by qt3to4:
#include <QDragLeaveEvent>
#include <Q3Frame>
#include <QDragMoveEvent>
#include <QEvent>
#include <QDropEvent>

#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "ksvdraglist.h"
#include "ksvdrag.h"
#include "ActionList.h"
#include "trash.h"

KSVTrash::KSVTrash (QWidget* parent, const char* name)
  : Q3Frame (parent, name),
    mKIL (KGlobal::iconLoader()),
    mLabel (new QLabel(this)),
    mOpen (false)
{
  setLineWidth(1);
  setMidLineWidth(0);

  setFrameStyle (Q3Frame::StyledPanel | Q3Frame::Sunken);

  mLabel->setPixmap(mKIL->loadIcon("user-trash", KIconLoader::Desktop));
  mPixmapWidth = mLabel->pixmap()->width();
  mLabel->setGeometry(5, 7, mPixmapWidth, mPixmapWidth);

  mLabel->setToolTip( i18n("Drag here to remove services"));
  this->setToolTip( i18n("Drag here to remove services"));
    
  setMinimumSize(sizeHint());
  setAcceptDrops(true);

  mLabel->installEventFilter(this);
  mLabel->setAcceptDrops(true);
}

KSVTrash::~KSVTrash()
{
}

void KSVTrash::dropEvent (QDropEvent* e)
{
  KSVData data;
  KSVDragList* list = static_cast<KSVDragList*> (e->source());

  if (list && strcmp (list->name(), "Scripts")  && KSVDrag::decodeNative (e, data))
	{
	  e->accept();
	  
	  emit undoAction (new RemoveAction (list, &data));
	  delete list->match (data);
	}
  else
	e->ignore();

  if (mOpen)
    {
      mLabel->repaint(); 
      mOpen = false;
    }
}

void KSVTrash::dragMoveEvent ( QDragMoveEvent* e )
{
  if (e->provides ("application/x-ksysv") &&
      e->source() && strcmp (e->source()->name(), "Scripts"))
    {
      QPainter p;
	  
      p.begin(mLabel);
      p.drawPixmap( 0, 0, mKIL->loadIcon("user-trash-full", KIconLoader::Desktop) );
      p.end();
	  
      mOpen = true;
      e->accept();
    }
  else
	e->ignore();
}

void KSVTrash::dragLeaveEvent ( QDragLeaveEvent* )
{
  if (mOpen)
    {
      mLabel->repaint(); 
      mOpen = false;
    }
}

bool KSVTrash::eventFilter( QObject *, QEvent *e )
{
  switch (e->type())
    {
    case QEvent::DragMove:
      dragMoveEvent ( static_cast<QDragMoveEvent*> (e) );
      return true;
      break;
      
    case QEvent::DragLeave:
      dragLeaveEvent ( static_cast<QDragLeaveEvent*> (e) );
      return true;
      break;
      
    case QEvent::Drop:
      dropEvent ( static_cast<QDropEvent*> (e) );
      return true;
      break;

    default:
      return false;
    }
}

QSize KSVTrash::sizeHint() const
{
  static QSize size = QSize (mPixmapWidth + 2 * 5, mPixmapWidth + 2 * 7);

  return size;
}

#include "trash.moc"
