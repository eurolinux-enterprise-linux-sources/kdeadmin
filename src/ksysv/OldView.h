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

#ifndef KSV_VIEW_H
#define KSV_VIEW_H

#include <qsplitter.h>
#include <q3valuelist.h>
#include <qsize.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3CString>
#include <QMoveEvent>
#include <Q3Frame>
#include <QLabel>
#include <QShowEvent>
#include <QResizeEvent>
#include <Q3PopupMenu>

#include <kmimetypetrader.h>

class Q3PopupMenu;
class QFileInfo;
class QLabel;
class Q3ListViewItem;
class Q3TextEdit;
class Q3Frame;
class Q3VBox;
class QPixmap;
class QLayout;

class KScroller;
class K3Process;
class K3ListView;
class QSplitter;
class KMenu;
class KSVTrash;
class KSVDragList;
class KSVConfig;
class KSVData;
class KSVItem;
class KSVAction;
class KSVTopLevel;

class KSVContent : public QSplitter
{
  Q_OBJECT

public:
  KSVContent (KMenu* openWithMenu, KSVTopLevel* parent = 0, const char* name = 0);
  ~KSVContent();

  KSVDragList* getOrigin();

  const QString& log() const;
  const QString& xmlLog() const;

  void setColors (const QColor& newNormal,
                  const QColor& newSelected,
                  const QColor& changedNormal,
                  const QColor& changedSelected);

  void mergeLoadedPackage (Q3ValueList<KSVData>* start,
                           Q3ValueList<KSVData>* stop);

public slots:
  void slotWriteSysV();

  void infoOnData(KSVItem* data);
  void setDisplayScriptOutput(bool val);
  void slotScriptProperties (Q3ListViewItem*);
  void multiplexEnabled (bool);

  void hideRunlevel (int index);
  void showRunlevel (int index);

protected:
  virtual void resizeEvent (QResizeEvent* e);
  virtual void moveEvent (QMoveEvent* e);
  virtual void showEvent (QShowEvent*);

private slots:
  void calcMinSize ();
  void fwdOrigin (KSVDragList*);
  void startService();
  void startService (const QString& path);
  void stopService();
  void stopService (const QString& path);
  void restartService();
  void restartService (const QString& path);
  void editService();
  void editService (const QString& path);
  void slotOutputOrError( K3Process* _p, char* _buffer, int _buflen );
  void slotExitedProcess(K3Process* proc);
  void slotScriptsNotRemovable();
  void slotDoubleClick (Q3ListViewItem*);
  void slotExitDuringRestart(K3Process* proc);
  void appendLog(const QString& rich, const QString& plain);
  void appendLog(const Q3CString& _buffer);
  void fwdCannotGenerateNumber();
  void fwdOrigin();
  void reSortRL();
  void pasteAppend();
  void fwdUndoAction(KSVAction*);
  void updatePanningFactor();

  void popupRunlevelMenu (K3ListView*, Q3ListViewItem*, const QPoint&);
  void popupServicesMenu (K3ListView*, Q3ListViewItem*, const QPoint&);

  void updateServicesAfterChange (const QString&);
  void updateRunlevelsAfterChange ();

  void repaintRunlevels ();

  void openWith ();
  void openWith (int index);

signals:
  void sigUpdateParent();
  void sigRun (const QString&);
  void sigStop();
  void sigNotRemovable();
  void cannotGenerateNumber();
  void selected (KSVItem*);
  void selectedScripts (KSVItem*);
  void sizeChanged();

  void undoAction (KSVAction*);
  void logChanged();

  void newOrigin();

private:
  static int splitterToPanningFactor (const Q3ValueList<int>&);
  static const Q3ValueList<int>& panningFactorToSplitter (int);

  void merge (Q3ValueList<KSVData>& list, KSVDragList* widget);
  void initLList();
  void initScripts();
  void initRunlevels();
  void info2Widget (QFileInfo* info, int index);
  void writeToDisk (const KSVData& _w, int _rl, bool _start);
  void clearRL( int _rl );

  friend class KSVTopLevel;

  Q3Frame* mContent;
  KScroller* mScroller;

  KSVDragList** startRL;
  KSVDragList** stopRL;
  KSVDragList* scripts;

  KMenu* mItemMenu;
  KMenu* mContextMenu;
  KMenu* mScriptMenu;
  KSVTrash* trash;
  Q3TextEdit* textDisplay;
  KSVConfig* conf;

  Q3VBox* mScriptBox;
  Q3VBox** mRunlevels;
  QWidget* mBuffer;

  KSVDragList* mOrigin;

  QString mLogText, mXMLLogText;

  QSize mMinSize;

  KMenu* mOpenWithMenu;
  KServiceOfferList mOpenWithOffers;

  Q3CString m_buffer;
};

#endif
