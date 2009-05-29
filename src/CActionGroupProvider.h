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
class QWidget;

class CActionGroupProvider: public QObject
{
  Q_OBJECT
  Q_ENUMS(ActionGroupName)
public:
  CActionGroupProvider();
  virtual ~CActionGroupProvider();
  static CActionGroupProvider* getInstance();
  enum ActionGroupName
  {
    TrackMenu,
    MapMenu,
    MainMenu
  };

  void addAction(ActionGroupName group, QAction *action, bool force = false);
  QAction *addAction(ActionGroupName group, QString shortCut, QString icon, QString text, QString tooltip, bool force = false);

  void removeAction(ActionGroupName group, QAction *action);
  void removeAction(QAction *action);
  void switchToActionGroup(ActionGroupName group);

  QList<QAction *> *getActiveActions() { return actionGroupHash.value(activeGroup,new QList<QAction* >());};
private:
  ActionGroupName activeGroup;
  QHash<ActionGroupName, QList<QAction *> *> actionGroupHash;
};

#endif /* CActionGroupProvider_H_ */
