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
#include "CMainWindow.h"
#include "CActions.h"
#include "CCanvas.h"

#include <QtGui>
#define SIZE_OF_MEGAMENU 10

//#define lqdebug(x) qDebug() << x
#define lqdebug(x)

CActionGroupProvider::CActionGroupProvider(QObject *parent) : QObject(parent)
{
    activeGroup = MainMenu;
    actions = new CActions(this);
}


CActionGroupProvider::~CActionGroupProvider()
{

}


void CActionGroupProvider::addAction(ActionGroupName groupName, const QString& actionName, bool force /*= false*/)
{
    QAction *action = actions->getAction(actionName);

    if (action)
        addAction(groupName,action,force);
}


void CActionGroupProvider::addAction(ActionGroupName groupName, QAction *action, bool force /*= false*/)
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


void CActionGroupProvider::removeAction(ActionGroupName group, QAction *action)
{

}


void CActionGroupProvider::removeAction(QAction *action)
{

}


void CActionGroupProvider::switchToActionGroup(ActionGroupName group)
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
            a->setEnabled(false);
        }
        else {
            lqdebug(QString("Action with '%1' as text is not controlled -> don't touch").arg(a->text()));
        }
    }

    foreach(QAction* a, *actionGroupHash.value(group)) {
        lqdebug(QString("Controlled Action with '%1' added").arg(a->text()));
        theMainWindow->addAction(a);
        a->setEnabled(true);
    }

    emit (stateChanged());
}


bool CActionGroupProvider::addActionsToMenu(QMenu *menu, bool isMegaMenu /*= false*/)
{
    int i=0;
    menu->setTitle(actions->getMenuTitle());
    foreach(QAction *a, *getActiveActions()) {
        if (isMegaMenu) {
            lqdebug("isMegamenu");
            QRegExp re ("^F(\\d+)$");
            if (re.exactMatch(a->shortcut().toString())) {
                int nextNumber = re.cap(1).toInt();
                lqdebug("match" << nextNumber << i);
                while(i < nextNumber) {
                    lqdebug("add action" << nextNumber << i);
                    QAction *dummyAction = new QAction(menu);
                    dummyAction->setText(tr("-"));
                    dummyAction->setShortcut(tr("F%1").arg(i));
                    menu->addAction(dummyAction);
                    i++;
                }
            }
            if (i> SIZE_OF_MEGAMENU)
                return true;
        }
        menu->addAction(a);
        i++;
    }
    // Howto set the pixmap as background of the menu?
    // menu->render(&actions->getMenuPixmap());
}


bool CActionGroupProvider::addActionsToWidget(QLabel *menu)
{
    int i=0;
    menu->setObjectName(actions->getMenuTitle());
    foreach(QAction *a, *getActiveActions()) {
        lqdebug("isMegamenu");
        QRegExp re ("^F(\\d+)$");
        if (re.exactMatch(a->shortcut().toString())) {
            int nextNumber = re.cap(1).toInt();
            lqdebug("match" << nextNumber << i);
            while(i < nextNumber) {
                lqdebug("add action" << nextNumber << i);
                QAction *dummyAction = new QAction(menu);
                dummyAction->setText(tr("-"));
                dummyAction->setShortcut(tr("F%1").arg(i));
                menu->addAction(dummyAction);
                i++;
            }
        }
        if (i> SIZE_OF_MEGAMENU) {
            return true;
        }

        menu->addAction(a);
        i++;
    }

    menu->setPixmap(actions->getMenuPixmap());
    // Howto set the pixmap as background of the menu?
    // menu->render(&actions->getMenuPixmap());
}
