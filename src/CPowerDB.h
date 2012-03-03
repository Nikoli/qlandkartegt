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

//#include "IDB.h"
#include "CWpt.h"
#include "CPowerLine.h"
//#include "CPowerNW.h"

#include <QObject>
#include <QSqlDatabase>

class CPowerNW;
class CPowerToolWidget;
class QUndoStack;

class CPowerDB : public QObject
{
    Q_OBJECT;
    public:
        CPowerDB(QTabWidget * tb, QObject * parent);
        
        virtual ~CPowerDB();

        static CPowerDB& self(){return *m_self;}

        CPowerToolWidget * getToolWidget();

        void clear();

        void load(const QString& filename);
        void save(const QString& filename);

        void addWpt(CWpt* wpt);
        void delWpt(const QString& key);
        
        CPowerLine* newPowerLine(const QString& start, const QString& end);
        void delPowerLine(const QString& key);
        void delPowerLines(const QStringList& keys);
        const bool hasPowerLines();
        
        CPowerNW* newPowerNW(const QString& ph_key);
        void delPowerNW(const QString& key);
        void delPowerNWs(const QStringList& keys);
        const bool hasPowerNWs();
        
        struct wpt_eElectric {
            wpt_eElectric(const double c = 0, const double l = 0, const double v = 230,
                          const double pf = 0.75)
                : consumers(c), consumers1(0), consumers2(0), consumers3(0),
                  load(l), totalload(0),
                  loadphase1(0.0), loadphase2(0.0), loadphase3(0.0),
                  voltage(v), powerfactor(pf),
                  resistance(0), totalresistance(0), is_new(true) {};
          double consumers;
          double consumers1;
          double consumers2;
          double consumers3;
          double load;
          double totalload;
          double loadphase1;
          double loadphase2;
          double loadphase3;
          double voltage;
          double powerfactor;
          double resistance;
          double totalresistance;
          bool is_new;
        };
        
        void setElectricData(const QString& key, const wpt_eElectric& e);
        const wpt_eElectric getElectricData(const QString& key) const;
        const QString getElectricInfo(const QString& key) const;
        const bool hasElectricData();

        void setPowerLineData(CPowerLine& l);
        void setPowerNWData(CPowerNW& nw);

        CPowerLine* getPowerLineByKey(const QString& key);
        const QStringList getPowerLines() const;
        
        CPowerNW* getPowerNWByKey(const QString& key);
        CPowerNW* getPowerNWByName(const QString& name);
        const QStringList getPowerNWs() const;

        const bool containsPowerLine(const QString& key) const;
        const bool containsPowerNW(const QString& key) const;
        
        void highlightPowerLine(const QString& key);
        void highlightPowerNW(const QString& key);
        
        /// unset the highlight flag
        void unHighlightPowerLine(const QString& key);
        //void setHighlightPowerNW(const QString& key, bool yes);
        /// get the value of the highlight flag
        unsigned isHighlightedPowerLine(const QString& key);
        bool isHighlightedPowerNW(const QString& key);
        /**
            <b>WARNING</b> The object referenced by the returned
            pointer might be subject to destruction at any time.
            Thus you must use it temporarily or store it by a
            QPointer object.

            @return A pointer to the current highlighted power line or 0.
        */
        CPowerLine* highlightedPowerLine();
        CPowerNW* highlightedPowerNW();

        QSqlQuery getCustomQuery();

        void gainFocus();
        void draw(QPainter& p, const QRect& rect, bool& needsRedraw);

    signals:
        void sigModified();
        void sigChanged(const bool highlightedOnly = false);

    private slots:
        void slotWptDBChanged();
        void slotWptDBKeyModified(const QString& key);

    private:
        friend class CPowerDBInternalEditLock;
        friend class CMainWindow;

        CPowerDB(QObject * parent);

        /// Open an empty database and create the table structure in it
        QSqlDatabase* createDB(const QString& dbname);

        /// move database from one version to most recent version
        void migrateDB(QSqlDatabase* thedb, int version);

        static CPowerDB * m_self;
        
        void addPowerLine(CPowerLine * l, CPowerNW * nw, bool silent);
        void addPowerNW(CPowerNW * nw, bool silent);
        
        void drawLine(const QLine& qline, const QRect& extViewport, QPainter& p);

        /// left hand tool widget tabbar
        QTabWidget * tabbar;

        QWidget *  toolview;
        
        QUndoStack *undoStack;

        /// a counter that is used by CPowerDBInternalEditLock to block slotItemChanged() on internal item edits
        quint32 isInternalEdit;
        /// the database object
        QSqlDatabase *db; // If this is not a pointer then at program end we get warning: "removeDatabase: connection 'qlpowerdb' is still in use"
        
        quint32 cnt;
        
        bool showBullets;
};

#endif //CPOWERDB_H

