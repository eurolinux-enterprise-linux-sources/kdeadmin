/*
   Copyright 2000 Peter Putzer <putzer@kde.org>
   Copyright 2002 Waldo Bastian <bastian@kde.org> 

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) version 3.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <qscrollbar.h>
#include <QLayout>
#include <qstyle.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QResizeEvent>

#include <kdebug.h>
#include <kdialog.h>

#include "kscroller.h"

class KScroller::KScrollerPrivate
{
public:
  KScrollerPrivate ()
    : setCornerWidget (0L)
  {
  }

  ~KScrollerPrivate () {}

  QWidget* setCornerWidget;
};

KScroller::KScroller (QWidget* parent, const char* name)
  : Q3Frame (parent, name),
    d (new KScrollerPrivate()),
    mVertical (new QScrollBar (Qt::Vertical, this)),
    mHorizontal (new QScrollBar (Qt::Horizontal, this)),
    mCornerWidget (new QWidget (this)),
    mContent (0L),
    mVerticalOld (0),
    mHorizontalOld (0)
{
  mCornerWidget->hide();
  mVertical->hide();
  mHorizontal->hide();

  connect (mVertical, SIGNAL (valueChanged (int)),
           this, SLOT (scrollVertical (int)));
  connect (mHorizontal, SIGNAL (valueChanged (int)),
           this, SLOT (scrollHorizontal (int)));
}

KScroller::~KScroller ()
{
  delete d;
}

void KScroller::setContent (QWidget* content)
{
  delete mContent;

  mContent = content;
  updateScrollBars();
}

void KScroller::setCornerWidget (QWidget* corner)
{
  delete mCornerWidget;

  mCornerWidget = d->setCornerWidget = corner;

  updateScrollBars();
}

QWidget* KScroller::cornerWidget ()
{
  return d->setCornerWidget;
}

void KScroller::resizeEvent (QResizeEvent* e)
{
  Q3Frame::resizeEvent (e);
 
  updateScrollBars();
}

QSize KScroller::minimumSizeHint() const
{
  QSize size = sizeHint();
  if (size.width() > 300)
     size.setWidth(300);
  return size;
}

QSize KScroller::sizeHint() const
{
  QSize size = mContent->minimumSize();
  int extra = kapp->style().pixelMetric( QStyle::PM_ScrollBarExtent, 0 );
  size += QSize( extra, extra );
  return size;
}

void KScroller::updateScrollBars ()
{
  int w = width();
  int h = height();
  QSize cs = mContent->minimumSize();

  setupVertical ( cs.width(), cs.height(), w, h);
  setupHorizontal ( cs.width(), cs.height(), w, h);
  mContent->resize (cs);

  setupCornerWidget (w, h);
}

void KScroller::setupHorizontal (int cw, int, int w, int h)
{
  mHorizontal->setValue (0);
  mHorizontalOld = 0;

  if (cw > w)
    {
      int extra = kapp->style().pixelMetric( QStyle::PM_ScrollBarExtent, 0 );

      if (!mVertical->isHidden())
        w -= extra;

      mHorizontal->setRange (0, cw - w);
      mHorizontal->setPageStep (w);
      mHorizontal->setLineStep (25);
      mHorizontal->setGeometry (0, h - extra, w, extra);

      mHorizontal->raise();
      mHorizontal->show();
    }
  else
    {
      mHorizontal->hide();
    }
}

void KScroller::setupVertical (int, int ch, int w, int h)
{
  mVertical->setValue (0);
  mVerticalOld = 0;

  if (ch > h)
    {
      int extra = kapp->style().pixelMetric( QStyle::PM_ScrollBarExtent, 0 );
      mVertical->setRange (0, ch - h);
      mVertical->setPageStep (h);
      mVertical->setLineStep (25);

      mVertical->setGeometry (w - extra, 0, extra, h);

      mVertical->raise();
      mVertical->show();
    }
  else
    {
      mVertical->hide();
    }
}

void KScroller::scrollVertical (int value)
{  
  int amount = value - mVerticalOld;
  mVerticalOld = value;

  mContent->move (mContent->x(), mContent->y() - amount);
}

void KScroller::scrollHorizontal (int value)
{
  int amount = value - mHorizontalOld;
  mHorizontalOld = value;
  
  mContent->move (mContent->x() - amount, mContent->y());
}

void KScroller::setupCornerWidget (int w, int h)
{
  if (!mVertical->isHidden() && !mHorizontal->isHidden())
    {
      int extra = kapp->style().pixelMetric( QStyle::PM_ScrollBarExtent, 0 );

      mCornerWidget->raise();
      mCornerWidget->setGeometry (w - extra, h - extra, extra, extra);

      mCornerWidget->show();
    }
  else
    {
      mCornerWidget->hide();
    }
}

#include "kscroller.moc"
