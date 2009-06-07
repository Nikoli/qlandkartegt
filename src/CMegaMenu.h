/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#ifndef CMEGAMENU_H
#define CMEGAMENU_H

#include <QLabel>
#include <QPointer>
#include <QVector>
#include "CActionGroupProvider.h"
class CCanvas;
class QLabel;
class QGridLayout;
class CActions;
class QVBoxLayout;
class QScrollArea;
/// the left hand context sensitive menu
class CMegaMenu : public QLabel
{
    Q_OBJECT;
    public:
        virtual ~CMegaMenu();

        static CMegaMenu& self(){return *m_self;}

        void keyPressEvent(QKeyEvent * e);

        void switchByKeyWord(const QString& key);

    protected slots:
        void slotEnable(){setEnabled(true);}

    protected:
        void mousePressEvent(QMouseEvent * e);

    private:
        friend class CMainWindow;
        CMegaMenu(CCanvas * canvas);
        CActionGroupProvider *actionGroup;
        CActions *actions;
        struct func_key_state_t
        {
            func_key_state_t() {
                icon = 0;
                name = tr("-");
                func = 0;
                tooltip = tr("-");
            }

            func_key_state_t(const char * icon,const QString& name, void (CMegaMenu::*func)(), const QString& tooltip)
                : icon(icon), name(name), func(func), tooltip(tooltip){}
            const char * icon;
            QString name;
            void (CMegaMenu::*func)();
            QString tooltip;
        };

    private slots:
        void switchState();

     private:
        static CMegaMenu * m_self;
        QWidget *menuWidget;
        QVBoxLayout *mainLayout;
        QPointer<CCanvas>  canvas;
        QMenu *menu;
        QScrollArea *scrollArea;
        QLabel * menuTitle;

        QGridLayout * layout;
        QLabel * keyEsc;
        QLabel * keyF1;
        QLabel * keyF2;
        QLabel * keyF3;
        QLabel * keyF4;
        QLabel * keyF5;
        QLabel * keyF6;
        QLabel * keyF7;
        QLabel * keyF8;
        QLabel * keyF9;
        QLabel * keyF10;

        QLabel * icons[11];

        QLabel * names[11];

        QVector<func_key_state_t>* current;

        QVector<func_key_state_t> fsMain;
        QVector<func_key_state_t> fsMap;
        QVector<func_key_state_t> fsMap3D;
        QVector<func_key_state_t> fsWpt;
        QVector<func_key_state_t> fsTrack;
        QVector<func_key_state_t> fsLiveLog;
        QVector<func_key_state_t> fsOverlay;
        QVector<func_key_state_t> fsMainMore;
        QVector<func_key_state_t> fsRoute;

};
#endif                           //CMEGAMENU_H
