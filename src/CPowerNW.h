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
#ifndef CPOWERNETWORK_H
#define CPOWERNETWORK_H

#include "IItem.h"

class CPowerNW : public IItem
{
    Q_OBJECT;
    public:
        CPowerNW(QObject * parent);
        virtual ~CPowerNW();

        //static QDir& getWptPath(){return path;}

        void setIcon(const QString& str);
        QString getInfo();

        QPixmap getIcon();
        
        QRectF getBoundingRectF();
        

    public:
        quint32 selected;
        QString ph;
        float   voltage;
        float   minVoltage;
        float   watts; 
        float   conductivity;
        float   powerfactor;
        float   ratedVoltage;
        
};

//QDataStream& operator >>(QDataStream& s, CPowerNetwork& wpt);
//QDataStream& operator <<(QDataStream& s, CPowerNetwork& wpt);

#endif                           //CPowerNetwork_H
  
