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

#ifndef KSCROLLER_H
#define KSCROLLER_H

#include <q3frame.h>
//Added by qt3to4:
#include <QResizeEvent>

class QScrollBar;

/**
 * A window that automatically enables scrollbars
 * if it's smaller than the content.
 *
 * @author Peter Putzer <putzer@kde.org>
 * @version $Id: kscroller.h 744825 2007-12-04 17:03:15Z toma $
 */
class KScroller : public Q3Frame
{
  Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param parent the parent of this widget (passed on as ususal).
   * @param name the name of this widget (as above).
   */
  KScroller (QWidget* parent = 0L, const char* name = 0L);

  /** 
   * Destructor.
   */
  virtual ~KScroller ();

  /**
   * @return the vertical scrollbar.
   */
  inline QScrollBar* verticalScrollBar () { return mVertical; }

  /**
   * @return the horizontal scrollbar.
   */
  inline QScrollBar* horizontalScrollBar () { return mHorizontal; }

  /**
   * @return the current content, or 0L if none set.
   */
  inline QWidget* content () { return mContent; }

  /**
   * @return the current corner widget, or 0L if none set.
   */
  QWidget* cornerWidget ();

public slots:
  /**
   * Sets the content. Ownership is transfered to the scroller, any
   * previously set content will be deleted!
   * 
   * @param content has to be a child of the KScroller.
   */
  void setContent (QWidget* content);

  /**
   * Sets the corner widget (the small widget that's located where the vertical and horizontal scrollbars
   * allmost meet).
   *
   * @param corner has to be a child of the KScroller.
   */
  void setCornerWidget (QWidget* corner);

  /**
   * Update the scrollbars. Call whenever you change the contents minimumSize.
   */
  void updateScrollBars ();

protected:
  /**
   * Reimplemented for internal reasons, the API is not affected.
   */
  virtual void resizeEvent (QResizeEvent*);

  /**
   * Reimplemented for internal reasons, the API is not affected.
   */
  virtual QSize sizeHint() const;

  /**
   * Reimplemented for internal reasons, the API is not affected.
   */
  virtual QSize minimumSizeHint() const;


private slots:
  /**
   * Scroll vertically.
   *
   * @param value is the new slider value.
   */
  void scrollVertical (int value);

  /**
   * Scroll horizontally.
   *
   * @param value is the new slider value.
   */
  void scrollHorizontal (int value);

private:
  /**
   * Set up the horizontal scrollbar.
   *
   * @param cw is the width of the content.
   * @param ch is the height of the content.
   * @param w is the width of the scroller.
   * @param h is the height of the scroller.
   */
  void setupHorizontal (int cw, int ch, int w, int h);

  /**
   * Set up the vertical scrollbar
   *
   * @param cw is the width of the content.
   * @param ch is the height of the content.
   * @param w is the width of the scroller.
   * @param h is the height of the scroller.
   */
  void setupVertical (int cw, int ch, int w, int h);

  /**
   * Set up the corner widget.
   *
   * @param w is the width of the scroller.
   * @param h is the height of the scroller.
   */
  void setupCornerWidget (int w, int h);

  class KScrollerPrivate;
  KScrollerPrivate* d;

  QScrollBar* mVertical;
  QScrollBar* mHorizontal;
  QWidget* mCornerWidget;

  QWidget* mContent;
  
  int mVerticalOld;
  int mHorizontalOld;
};

#endif // KSCROLLER_H
