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

#include <qapplication.h>
#include "ksvdraglist.h"
#include "ActionList.h"

//////////////////////////////////
// Action                       //
//////////////////////////////////

KSVAction::KSVAction()
{
}

KSVAction::~KSVAction()
{
  // default - shouldn't be used normally (pure virtual declaration)
}

SimpleAction::SimpleAction (KSVDragList* s, const KSVData* i)
  : KSVAction(),
    mSource (s),
    mItem (new KSVData (*i))
{
}

SimpleAction::~SimpleAction()
{
  delete mItem;
}

ChangeAction::ChangeAction (KSVDragList* s, const KSVData* oldS, const KSVData* newS)
  : SimpleAction (s, oldS),
    mNew (new KSVData (*newS))
{
  source()->addToRMList (*oldState());
}

ChangeAction::~ChangeAction()
{
  delete mNew;
}

void ChangeAction::redo ()
{
  source()->addToRMList(*oldState());
  source()->match (*oldState())->copy (*newState());
  source()->sort();
}

void ChangeAction::undo()
{
  source()->removeFromRMList(*oldState());
  source()->match(*newState())->copy (*oldState());
  source()->sort();
}

AddAction::AddAction (KSVDragList* s, const KSVData* i)
  : SimpleAction(s, i)
{
}

AddAction::~AddAction()
{
}

void AddAction::redo ()
{
  new KSVItem (source(), *item());
}

void AddAction::undo()
{
  delete source()->match(*item());
}

RemoveAction::RemoveAction (KSVDragList* s, const KSVData* i)
  : SimpleAction(s, i)
{
  source()->addToRMList (*item());
}

RemoveAction::~RemoveAction()
{
}

void RemoveAction::redo()
{
  source()->addToRMList(*item());

  delete source()->match (*item());
}

void RemoveAction::undo()
{
  source()->removeFromRMList(*item());

  new KSVItem (source(), *item());
}

CompoundAction::CompoundAction (KSVAction** a, int nr)
  : mActions (new KSVAction* [nr]),
    mCount (nr)
{
  memcpy (mActions, a, sizeof(KSVAction*) * nr);
}

CompoundAction::~CompoundAction()
{
  for (int i=0; i < mCount; ++i)
    {
      delete mActions[i];
    }

  delete[] mActions;
}

void CompoundAction::redo ()
{
  for (int i=0; i < mCount; ++i)
    {
      mActions[i]->redo();
    }
}

void CompoundAction::undo()
{
  for (int i=0; i < mCount; ++i)
    {
      mActions[i]->undo();
    }
}

//////////////////////////////////
// ActionList                   //
//////////////////////////////////

ActionList::ActionList (QObject* parent, const char* name)
  : QObject(parent, name)
{
  setAutoDelete(false);
}

ActionList::~ActionList()
{
}

void ActionList::undoLast()
{
  if (!count())
    return;

  KSVAction* a = Q3PtrStack<KSVAction>::pop();
  a->undo();

  emit undone();

  if (!count())
    emit empty();
}

void ActionList::undoAll()
{
  while (count())
    undoLast();
}

void ActionList::redoLast()
{
  if (!count())
    return;

  KSVAction* a = Q3PtrStack<KSVAction>::pop();
  a->redo();

  emit undone();

  if (!count())
    emit empty();
}

void ActionList::redoAll()
{
  while (count())
    redoLast();
}

void ActionList::push (KSVAction* a)
{
  Q3PtrStack<KSVAction>::push(a);

  if (count() == 1)
    emit filled();
}

void ActionList::clear()
{
  setAutoDelete (true);
  Q3PtrStack<KSVAction>::clear();
  setAutoDelete (false);

  emit empty();
}

#include "ActionList.moc"
