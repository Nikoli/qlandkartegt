/**********************************************************************************************
    Copyright (C) 2011 Jan Rheinländer jrheinlaender@users.sourceforge.net

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
#ifndef CPOWERLINE_H
#define CPOWERLINE_H


#include "IItem.h"

#include <QObject>
//#include <QPixmap>
#include <QString>
#include <QList>
/*#include <QFile>
#include <QDir>
#include <QSet>*/
#include <QDebug>

#include "CDlgEditPowerLine.h"

class IDB;

class CPowerLine : public IItem
{
    Q_OBJECT;
    public:
        CPowerLine(QObject * parent);
        virtual ~CPowerLine();

        QString getInfo();
        
        void setIcon(const QString&) {}; // Must be defined because it is virtual
        
        const QLine getLine() const;

        const double getDistance() const;
        
        /// set color by id
        void setColor(unsigned i);
        /// get QT color
        const QColor& getColor(){return color;}
        
        bool isHidden(){return m_hide;}
        
    private:
        //friend QDataStream& operator >>(QDataStream& s, CPowerLine& wpt);
        //friend QDataStream& operator <<(QDataStream& s, CPowerLine& wpt);
        //friend class CDlgEditWpt;
        //friend class CDlgWptEdit;

    public:
        quint32  selected;
        QString  keyNetwork;
        QString  keyFirst;
        QString  keySecond;

        void setCrossSection(const double D) { crossSection = D; recalc(); }
        void setPhases(const unsigned P) { phases = P; recalc(); }
        void setLength(const double L)   { length = L; recalc(); }
        void setConductivity(const double kappa) { conductivity = kappa; recalc(); }
        void setResistance(const double R) { resistance = R; }
        void setCurrent(const double I)  { current = I; recalc(); }
        void setDrop(const double delta_U) { drop = delta_U; }

        const double getCrossSection() const { return crossSection; }
        const double getPhases() const { return phases; }
        const double getLength() const { return length; }
        const double getConductivity() const { return conductivity; }
        const double getCurrent() const { return current; }
        const double getResistance() const { return resistance; }
        const double getDrop() const { return drop; }

        const double getPowerOnPhase(const int phase);

        const int getNumPhases() const; // 1, 2 or 3 phases (plus neutral)
        const bool hasPhase(const unsigned nump) const { return phases & (1>>(nump-1)); }
        const int firstPhase() const;
        const int secondPhase() const;

        unsigned highlighted; // 0: none, 1: network, 2: line, 3: both
        bool m_hide;
        
    private:
        float    crossSection;
        unsigned phases;
        float    length;
        float    conductivity;
        float    resistance;
        float    current;
        float    drop;

        /// the power line color
        QColor  color;
        /// the track line color by index
        unsigned colorIdx;

        void recalc();

};

//QDataStream& operator >>(QDataStream& s, CPowerLine& wpt);
//QDataStream& operator <<(QDataStream& s, CPowerLine& wpt);

//void operator >>(QFile& s, CPowerLine& wpt);
//void operator <<(QFile& s, CPowerLine& wpt);
#endif                           //CPowerLine_H

