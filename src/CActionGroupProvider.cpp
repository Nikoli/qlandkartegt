//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- Copyright (c) 2009 Marc Feld
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C-  ------------------------------------------------------------------

#include "CActionGroupProvider.h"
#include <QPointer>
#include <QWidget>

#include "CMainWindow.h"
#include "CActions.h"

CActionGroupProvider::CActionGroupProvider(QObject *parent) : QObject(parent)
{
  activeGroup = MainMenu;
}

CActionGroupProvider::~CActionGroupProvider()
{

}

void CActionGroupProvider::addAction(ActionGroupName groupName, const QString& actionName, bool force /*= false*/)
{
  QAction *action = theMainWindow->getActions()->getAction(actionName);

  if (action)
    addAction(groupName,action,force);
}

void CActionGroupProvider::addAction(ActionGroupName groupName, QAction *action, bool force /*= false*/)
{
  QList<QAction *> *actionGroup;
  if(!actionGroupHash.contains(groupName))
  {
    actionGroup = new QList<QAction *>();
    actionGroupHash.insert(groupName, actionGroup);
  }
  else
    actionGroup = actionGroupHash.value(groupName);

  actionGroup->append(action);
}

void CActionGroupProvider::removeAction(ActionGroupName group, QAction *action)
{

}

void CActionGroupProvider::removeAction(QAction *action)
{

}

void CActionGroupProvider::switchToActionGroup(ActionGroupName group)
{
  activeGroup = group;
  foreach(ActionGroupName i, actionGroupHash.keys())
  {
    foreach(QAction* a, *actionGroupHash.value(i))
    {
      a->setEnabled(false);
    }
  }

  foreach(QAction* a, *actionGroupHash.value(group))
  {
    a->setEnabled(true);
  }
}
