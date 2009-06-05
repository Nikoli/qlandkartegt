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

#ifndef CACTIONGROUPPROVIDER_H_
#define CACTIONGROUPPROVIDER_H_
#include <QList>
#include <QAction>
#include <QPointer>
#include <QHash>
class CActions;
class QWidget;

class CActionGroupProvider: public QObject
{
  Q_OBJECT
  Q_ENUMS(ActionGroupName)
public:
  CActionGroupProvider(QObject * parent);
  virtual ~CActionGroupProvider();
  enum ActionGroupName
  {
    Map3DMenu,
    WptMenu,
    LiveLogMenu,
    OverlayMenu,
    MainMoreMenu,
    TrackMenu,
    RouteMenu,
    MapMenu,
    MainMenu
  };

  void addAction(ActionGroupName group, QAction *action, bool force = false);
  void addAction(ActionGroupName group, const QString& actionName, bool force = false);

  void removeAction(ActionGroupName group, QAction *action);
  void removeAction(QAction *action);
  void switchToActionGroup(ActionGroupName group);
  CActions* getActions() {return actions;};
  QList<QAction *> *getActiveActions() { return actionGroupHash.value(activeGroup,new QList<QAction* >());};
private:
  ActionGroupName activeGroup;
  QHash<ActionGroupName, QList<QAction *> *> actionGroupHash;
  CActions* actions;
};

#endif /* CActionGroupProvider_H_ */
