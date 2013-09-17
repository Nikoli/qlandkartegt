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
#include "CRoute.h"
#include "GeoMath.h"

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
  projXY pt1, pt2;

  CWpt* wpt1 = CWptDB::self().getWptByKey(keyFirst);
  CWpt* wpt2 = CWptDB::self().getWptByKey(keySecond);
  if ((wpt1 == NULL) || (wpt2 == NULL))
      return -1.0;

  pt1.u = wpt1->lon * DEG_TO_RAD;
  pt1.v = wpt1->lat * DEG_TO_RAD;
  pt2.u = wpt2->lon * DEG_TO_RAD;
  pt2.v = wpt2->lat * DEG_TO_RAD;

  // create route then track and find distance for spacing poles at 30m (which is a good average, changing it to 25m or 50m doesn't
  // make a lot of difference in the distance calculation anyway)
  CRoute* route = new CRoute(NULL);
  route->addPosition(wpt1->lon, wpt1->lat, "");
  route->addPosition(wpt2->lon, wpt2->lat, "");
  CTrack* track = route->convertToTrack(30.0);
  // Conversion of waypoints to a route looses the altitude data. But the new intermediate points need to receive their
  // altitude from the DEM anyway
  track->replaceElevationByLocal(true);
  return track->getCorrectedDistance();
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
    // Line impedance calculation see Harvey, p. 299
    // Assumptions: Equal spacing of cables on the poles, seven-strand cable, 
    //              reactance from capacitive effect can be ignored for overhead cables,
    //		    phase and neutral have the same diameter in unbalanced systems
    // Effect of line impedance on the total power factor and losses incurred by connecting
    // the cables are ignored
    double d = 0.3; // 30 cm spacing between cables on the pole
    double D = sqrt(4.0 / M_PI * crossSection/7.0); // diameter of each cable strand in [mm]
    double r = 3.0 * D/2.0 / 1000.0; // effective cable radius in [m]
    double L = length * (5 + 46 * log10(d/r)) * 1E-8; // inductance in Henry
    double X_L = 2.0 * M_PI * 50.0 * L; // reactance due to inductive effect in Ohm
    
    double rphase = length / (conductivity * crossSection); // resistance of one phase
    double zphase = sqrt(X_L*X_L + rphase*rphase); // impedance of one phase
    //qDebug() << "Cable: " << crossSection << "mm², Length: " << length << "m , r: " << r << "m, L: " << L << "H, X_L: " << X_L << "Ohm"
    //         << ", R: " << rphase << "Ohm" << ", Z: " << zphase << "Ohm";
    //qDebug() << "Percentage of increase of resistance: " << (zphase/rphase - 1.0) * 100.0;
    double rneutral = length / (conductivity * crossSection); // resistance of neutral line
    double zneutral = sqrt(X_L*X_L + rneutral*rneutral);
    
    // TODO: Neutral cross-section might be different for
    // two-phase case and unbalanced three-phase case

    if (getNumPhases() == 1)
        resistance = zphase + zneutral; // because current goes there and back
    else if (getNumPhases() == 2)
        resistance =  zphase/2  + zneutral; // two single phases, current divided one way there but united on way back in neutral
    else
        resistance  = zphase/3; // three-phase system, if balanced then no current on neutral

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

const QList<QPointF> getIntersectionPoints(const QRect& r, const QLine& l)
{
    QList<QPoint> points;
    points << r.topRight() << r.bottomRight() << r.bottomLeft() << r.topLeft();
    QList<QPointF> result;
    QPoint p1 = r.topLeft();

    for (QList<QPoint>::const_iterator pt = points.begin(); pt != points.end(); pt++) {
        QLineF line(p1, *pt);
        QPointF p_inter;
        if (line.intersect(l, &p_inter) == QLineF::BoundedIntersection)
            result << p_inter;
        p1 = *pt;
    }

    return result;
}

const QPoint CPowerLine::getVisibleMiddle(const QPoint &middle, const QLine &line, const QRect &rect) {
    bool p1visible = rect.contains(line.p1());
    bool p2visible = rect.contains(line.p2());

    // Quick intersection test
    if (p1visible && p2visible) {
        // Line completely visible
        return middle;
    } else {
        // Exact intersection test
        QList<QPointF> points = getIntersectionPoints(rect, line);

        if (points.empty()) {
            // Line not visible at all
            return QPoint();
        } else if (points.size() == 1) {
            if (p1visible)
                points << line.p1();
            else if (p2visible)
                points << line.p2();
            else {
                // line touches at a corner
                return QPoint();
            }
        }

        // Line only partially visible
        QLineF visibleSegment(points.front(), points.back());
        if (visibleSegment.length() > 50.0)
            return ((visibleSegment.p1() + visibleSegment.p2())/2.0).toPoint();

        return QPoint();
    }
}
