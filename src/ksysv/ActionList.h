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

#ifndef KSV_ACTIONLIST_H
#define KSV_ACTIONLIST_H

#include <q3ptrstack.h>
#include <qobject.h>

class KSVData;
class KSVDragList;

class KSVAction
{
public:
  KSVAction ();
  virtual ~KSVAction () = 0;

  virtual void undo () = 0;
  virtual void redo() = 0;

private:
  const KSVAction& operator= (const KSVAction&); // undefined
  KSVAction (const KSVAction&); // undefined
};

class SimpleAction : public KSVAction
{
public:
  SimpleAction (KSVDragList* source, const KSVData* item);
  virtual ~SimpleAction();

  inline KSVDragList* source() { return mSource; }
  inline KSVData* item() { return mItem; }

  virtual void undo() = 0;
  virtual void redo() = 0;

private:
  KSVDragList* mSource;
  KSVData* mItem;
};

class RemoveAction : public SimpleAction
{
public:
  RemoveAction (KSVDragList* s, const KSVData* i);
  virtual ~RemoveAction();

  virtual void undo();
  virtual void redo();
};

class AddAction : public SimpleAction
{
public:
  AddAction (KSVDragList* s, const KSVData* i);
  virtual ~AddAction();

  virtual void undo();
  virtual void redo();
};

class ChangeAction : public SimpleAction
{
public:
  ChangeAction (KSVDragList* s, const KSVData* oldState, const KSVData* newState);
  virtual ~ChangeAction();

  virtual void undo();
  virtual void redo();

  inline KSVData* newState() { return mNew; }
  inline KSVData* oldState() { return item(); }
private:
  KSVData* mNew;
};

class CompoundAction : public KSVAction
{
public:
  CompoundAction (KSVAction*[], int nr);
  virtual ~CompoundAction();
  
  virtual void undo();
  virtual void redo();

private:
  KSVAction** mActions;
  int mCount;
};



class ActionList : public QObject, private Q3PtrStack<KSVAction>
{
  Q_OBJECT
  
public:
  ActionList (QObject* parent, const char* name);
  virtual ~ActionList ();

  KSVAction* top () const { return Q3PtrStack<KSVAction>::top(); }

public slots:
  void undoLast ();
  void undoAll ();
  void redoLast ();
  void redoAll ();
  void push (KSVAction*);
  void clear ();

signals:
  void undone ();
  void empty ();
  void filled ();
};

#endif

