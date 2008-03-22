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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/
#ifndef IDB_H
#define IDB_H

#include <QObject>

class QTabWidget;
class QWidget;
class QPainter;
class CGpx;
class CQlb;

/// base class for all database objects
class IDB : public QObject
{
    Q_OBJECT;
    public:
        IDB(QTabWidget * tb, QObject * parent);
        virtual ~IDB();

        virtual void gainFocus();

        virtual void loadGPX(CGpx& gpx) = 0;
        virtual void saveGPX(CGpx& gpx) = 0;

        virtual void loadQLB(CQlb& qlb) = 0;
        virtual void saveQLB(CQlb& qlb) = 0;

        virtual void upload() = 0;
        virtual void download() = 0;

        virtual void clear() = 0;

        signals:
        void sigChanged();

    protected:
        QTabWidget *  tabbar;
        QWidget *  toolview;
};
#endif                           //IDB_H
