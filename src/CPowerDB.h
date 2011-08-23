/**********************************************************************************************
    Copyright (C) 2011 Jan Rheinländer jrheinlaender@users.sourceforge.net

    Based on CGeoDB.h by Oliver Eichler.

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
#ifndef CPOWERDB_H
#define CPOWERDB_H

#include "IDB.h"

#include <QObject>
#include <QSqlDatabase>

class CPowerDB : public QObject
{
    Q_OBJECT;
    public:
        virtual ~CPowerDB();

        static CPowerDB& self(){return *m_self;}

        void clear();

        void load(const QString& filename);

        void save(const QString& filename);

    signals:
        void sigModified();
        void sigChanged();

    private slots:
        void slotWptDBChanged();
        void slotWptDBKeyModified(const QString& key);

    private:
        friend class CPowerDBInternalEditLock;
        friend class CMainWindow;

        CPowerDB(QObject * parent);

        /// initialize database from scratch
        void initDB();
        /// move database from one version to most recent version
        void migrateDB(int version);

        static CPowerDB * m_self;

        /// a counter that is used by CPowerDBInternalEditLock to block slotItemChanged() on internal item edits
        quint32 isInternalEdit;
        /// the database object
        QSqlDatabase *db; // If this is not a pointer then at program end we get warning: "removeDatabase: connection 'qlpowerdb' is still in use"
};

#endif //CPOWERDB_H

