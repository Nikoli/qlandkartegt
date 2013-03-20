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

#include "CPowerLine.h"
#include "CMapDB.h"
#include "CWpt.h"
#include "CWptDB.h"
#include "IMap.h"
#include "CTrack.h"
#include "GeoMath.h"

//#include "IUnit.h"
//#include "config.h"
//#include "GeoMath.h"

#include <QDateTime>
#include <QDebug>

CPowerLine::CPowerLine(QObject * parent)
: IItem(parent)
, keyNetwork("")
, keyFirst("")
, keySecond("")
, highlighted(0)
, m_hide(false)
, crossSection(4.0)
, phases(7)
, length(0)
, conductivity(30.0)
, resistance(0.0)
, current(0.0)
, drop(0.0)
{
}


CPowerLine::~CPowerLine()
{
//    qDebug() << "CPowerLine::~CPowerLine()";
}

QString CPowerLine::getInfo()
{
    QString str = getName() + "\n";
    if (str == "New Power line\n") str = ""; //

    /*if(timestamp != 0x00000000 && timestamp != 0xFFFFFFFF)
    {
        if(str.count()) str += "\n";
        QDateTime time = QDateTime::fromTime_t(timestamp);
        time.setTimeSpec(Qt::LocalTime);
        str += time.toString();
    }*/
    
    CWpt* first = CWptDB::self().getWptByKey(keyFirst);
    CWpt* second = CWptDB::self().getWptByKey(keySecond);
    str += (((first != NULL) && (second != NULL)) ? tr("From ") + first->getName() + tr(" to ") + second->getName() + "\n": "");
    str += trUtf8("%1 m, %2 mm²").arg(length,0,'f',0).arg(crossSection,0,'f',1);
    str += tr(", phases: ") + (phases & 1 ? "1" : "") + (phases & 2 ? "2" : "") + (phases & 4 ? "3" : "");
    str += "\n" + tr("current %1 A, drop %2 V").arg(current,0,'f',1).arg(drop,0,'f',1);

    if(description.count())
    {
        if(str.count()) str += "\n";

        if(description.count() < 200)
        {
            str += description;
        }
        else
        {
            str += description.left(197) + "...";
        }
    }

    // Debugging
    //str += tr("\nkappa=%1, R=%2").arg(conductivity,0,'f',0).arg(resistance,0,'f',1);

    return str;
}

const QLine CPowerLine::getLine(QPoint& middle, double& angle) const {
    IMap& map = CMapDB::self().getMap();
    
    CWpt* wpt1 = CWptDB::self().getWptByKey(keyFirst);
    CWpt* wpt2 = CWptDB::self().getWptByKey(keySecond);
    //if ((wpt1 == NULL) || (wpt2 == NULL))
    
    double u1 = wpt1->lon * DEG_TO_RAD;
    double v1 = wpt1->lat * DEG_TO_RAD;
    map.convertRad2Pt(u1,v1);
    
    double u2 = wpt2->lon * DEG_TO_RAD;
    double v2 = wpt2->lat * DEG_TO_RAD;
    map.convertRad2Pt(u2,v2);

    // Find middle point of line (e.g. for displaying info)
    middle = (QPoint(u1, v1) + QPoint(u2, v2))/2;

    // Find angle of line
    angle = atan2(v2 - v1, u2 - u1) * 180.0 / M_PI;
    
    return QLine(u1, v1, u2, v2);
} // getLine()

const double CPowerLine::getDistance() const {
  double a1, a2;
  projXY pt1, pt2;
  double dist;

  CWpt* wpt1 = CWptDB::self().getWptByKey(keyFirst);
  CWpt* wpt2 = CWptDB::self().getWptByKey(keySecond);

  pt1.u = wpt1->lon * DEG_TO_RAD;
  pt1.v = wpt1->lat * DEG_TO_RAD;
  pt2.u = wpt2->lon * DEG_TO_RAD;
  pt2.v = wpt2->lat * DEG_TO_RAD;
  dist = distance(pt1,pt2,a1,a2);
  //qDebug() << "Elevations: " << wpt1->ele << ", " << wpt2->ele;
  return sqrt(pow(dist, 2.0) + pow(double(wpt1->ele - wpt2->ele),2.0));
  // TODO: Is this geomathematically correct?
}

void CPowerLine::setColor(unsigned i)
{
    if(i>16) i = 4;
    colorIdx    = i;
    color       = CTrack::lineColors[i];
    //bullet      = QPixmap(bulletColors[i]);

    setIcon(color.name());
}

const int CPowerLine::getNumPhases() const
{
    switch (phases) {
        case 1:
        case 2:
        case 4: return 1;
        case 3:
        case 5:
        case 6: return 2;
        case 7: return 3;
    }

    return 0;
}


const int CPowerLine::firstPhase() const {
    switch (phases) {
        case 1:
        case 3:
        case 5:
        case 7: return 1;
        case 2:
        case 6: return 2;
        case 4: return 3;
    }

    return 0;
}

const int CPowerLine::secondPhase() const {
    switch (phases) {
        case 1:
        case 2:
        case 4: return 0; // invalid: single-phase system
        case 3:
        case 7: return 2;
        case 5:
        case 6: return 3;
    }

    return 0;
}

void CPowerLine::recalc()
{
    double rphase = length / (conductivity * crossSection); // resistance of one phase
    double rneutral = length / (conductivity * crossSection); // resistance of neutral line
    // TODO: Neutral cross-section might be different for
    // two-phase case and unbalanced three-phase case

    if (getNumPhases() == 1)
        resistance = rphase + rneutral; // because current goes there and back
    else if (getNumPhases() == 2)
        resistance =  rphase/2  + rneutral; // two single phases, current divided one way there but united on way back in neutral
    else
        resistance  = rphase/3; // three-phase system, if balanced then no current on neutral

    drop = resistance * current;
}

const double CPowerLine::getPowerOnPhase(const int phase) {
    switch (phase) {
        case 1:
            switch (phases) {
                case 1: return drop * current;
                case 2: return 0;
                case 4: return 0;
                case 3: return drop * current/2;
                case 5: return drop * current/2;
                case 6: return 0;
                case 7: return drop * current/3;
            }
        case 2:
            switch (phases) {
                case 1: return 0;
                case 2: return drop * current;
                case 4: return 0;
                case 3: return drop * current/2;
                case 5: return 0;
                case 6: return drop * current/2;
                case 7: return drop * current/3;
            }
        case 3:
            switch (phases) {
                case 1: return 0;
                case 2: return 0;
                case 4: return drop * current;
                case 3: return 0;
                case 5: return drop * current/2;
                case 6: return drop * current/2;
                case 7: return drop * current/3;
            }
    }
    return 0;
}
