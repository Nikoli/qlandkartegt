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

#include "CMenus.h"
#include "CMainWindow.h"
#include "CActions.h"
#include "CCanvas.h"

#include <QtGui>
#define SIZE_OF_MEGAMENU 11

//#define lqdebug(x) qDebug() << x
#define lqdebug(x)

CMenus::CMenus(QObject *parent) : QObject(parent)
{
    activeGroup = MainMenu;
    actions = new CActions(this);

    QStringList list;
    list << "aSwitchToMain" << "aMoveArea" << "aZoomArea" << "aCenterMap";

    foreach(QString s, list)
    {
        QAction *a = actions->getAction(s);
        if (a)
        {
            excludedActionForMenuBarMenu << a;
        }
    }

}


CMenus::~CMenus()
{

}


void CMenus::addAction(ActionGroupName groupName, const QString& actionName, bool force /*= false*/)
{
    QAction *action = actions->getAction(actionName);

    if (action)
        addAction(groupName,action,force);
}


void CMenus::addAction(ActionGroupName groupName, QAction *action, bool force /*= false*/)
{
    QList<QAction *> *actionGroup;
    if(!actionGroupHash.contains(groupName)) {
        actionGroup = new QList<QAction *>();
        actionGroupHash.insert(groupName, actionGroup);
    }
    else
        actionGroup = actionGroupHash.value(groupName);

    actionGroup->append(action);
    controlledActions << action;
}


void CMenus::removeAction(ActionGroupName group, QAction *action)
{

}


void CMenus::removeAction(QAction *action)
{

}


void CMenus::switchToActionGroup(ActionGroupName group)
{

    if (!actionGroupHash.value(group)) {
        qDebug() << tr("ActionGroup %1 not defined. Please fix.").arg(group);
        return;
    }

    activeGroup = group;

    foreach(QAction* a, ((QWidget *)theMainWindow)->actions()) {
        if (controlledActions.contains(a)) {
            lqdebug(QString("Action with '%1' as text is controlled -> removed").arg(a->text()));
            theMainWindow->removeAction(a);
            lqdebug(a->shortcuts());
            if (!actionsShortcuts.contains(a))
                actionsShortcuts.insert(a, a->shortcuts());
            a->setShortcuts(QList<QKeySequence> ());
            lqdebug(a->shortcuts());
        }
        else {
            lqdebug(QString("Action with '%1' as text is not controlled -> don't touch").arg(a->text()));
        }
    }

    foreach(QAction* a, *actionGroupHash.value(group)) {
        lqdebug(QString("Controlled Action with '%1' added").arg(a->text()));
        theMainWindow->addAction(a);
        if (actionsShortcuts.contains(a))
            a->setShortcuts(actionsShortcuts.value(a,QList<QKeySequence> ()));
        lqdebug(a->shortcuts());
    }

    emit (stateChanged());
}


bool CMenus::addActionsToMenu(QMenu *menu, MenuContextNames contex, ActionGroupName groupName)
{
    menu->setTitle(actions->getMenuTitle());
    menu->addActions(getActiveActionsList(menu,contex,groupName));

}


bool CMenus::addActionsToWidget(QLabel *menu)
{
    menu->setObjectName(actions->getMenuTitle());
    menu->addActions(getActiveActionsList(menu,LeftSideMenu,activeGroup));
    menu->setPixmap(actions->getMenuPixmap());
}


QList<QAction *> CMenus::getActiveActionsList(QObject *menu, MenuContextNames names, ActionGroupName groupName)
{
    QList<QAction *> list;
    int i=0;

    foreach(QAction *a, *getActiveActions(groupName)) {
        lqdebug(QString("enter menu: %1 ").arg(a->shortcut().toString()));
        if ( names.testFlag(LeftSideMenu) )
        {
            QRegExp re ("^F(\\d+)$");
            if (i==0)
            {
                if (a->shortcut().toString() != "Esc")
                {
                    lqdebug("add action Esc");
                    QAction *dummyAction = new QAction(menu);
                    dummyAction->setText(tr("-"));
                    dummyAction->setShortcut(tr("Esc"));
                    list << dummyAction;
                }
                i++;
            }
            if (re.exactMatch(a->shortcut().toString()) ) {
                int nextNumber = re.cap(1).toInt();
                lqdebug(QString("match: i=%1, nextNumber=%2").arg(i).arg(nextNumber) << a->shortcut().toString());
                while(i < nextNumber ) {
                    lqdebug("add action" << nextNumber << i);
                    QAction *dummyAction = new QAction(menu);
                    dummyAction->setText(tr("-"));
                    dummyAction->setShortcut(tr("F%1").arg(i));
                    list << dummyAction;
                    i++;
                }
                i++;
            }
            else
            {
                lqdebug("not matched" << a->shortcut().toString());
            }

            if (i> SIZE_OF_MEGAMENU) {
                return list;
            }
        }
        else
        {
            i++;
        }
        if (names.testFlag(MenuBarMenu) && !excludedActionForMenuBarMenu.contains(a))
        {
            if (!actionsShortcuts.contains(a))
                actionsShortcuts.insert(a, a->shortcuts());
            a->setShortcuts(QList<QKeySequence> ());
            list << a;
        }
        else if (names.testFlag(ContextMenu) || names.testFlag(LeftSideMenu))
        {
            list << a;
        }
    }

    return list;
}
QList<QAction *> *CMenus::getActiveActions(ActionGroupName group)
{
    ActionGroupName g = group;
    if (g == NoMenu)
    {
        g = activeGroup;
    }
    return actionGroupHash.value(g,new QList<QAction* >());
};
