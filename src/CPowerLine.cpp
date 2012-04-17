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
//#include <QtGui>
//#include <QtXml>



/*QDataStream& operator >>(QDataStream& s, CPowerLine& wpt)
{
    QIODevice * dev = s.device();
    qint64      pos = dev->pos();

    char magic[9];
    s.readRawData(magic,9);

    if(strncmp(magic,"QLWpt   ",9))
    {
        dev->seek(pos);
        //         throw(QObject::tr("This is not waypoint data."));
        return s;
    }

    QList<wpt_head_entry_t> entries;

    while(1)
    {
        wpt_head_entry_t entry;
        s >> entry.type >> entry.offset;
        entries << entry;
        if(entry.type == CPowerLine::eEnd) break;
    }

    QList<wpt_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end())
    {
        qint64 o = pos + entry->offset;
        dev->seek(o);
        s >> entry->data;

        switch(entry->type)
        {
            case CPowerLine::eBase:
            {
                QString icon;
                QString key;

                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);

                s1 >> key;
                s1 >> wpt.sticky;
                s1 >> wpt.timestamp;
                s1 >> icon;
                s1 >> wpt.name;
                s1 >> wpt.comment;
                s1 >> wpt.lat;
                s1 >> wpt.lon;
                s1 >> wpt.ele;
                s1 >> wpt.prx;
                s1 >> wpt.link;
                s1 >> wpt.description;
                s1 >> wpt.urlname;
                s1 >> wpt.type;
                s1 >> wpt.parentWpt;
                s1 >> wpt.selected;

                wpt.setIcon(icon);
                wpt.setKey(key);
                break;
            }

            case CPowerLine::eImage:
            {
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);
                CPowerLine::image_t img;

                wpt.images.clear();

                s1 >> img.offset;
                while(img.offset)
                {
                    wpt.images << img;
                    s1 >> img.offset;
                }

                QList<CPowerLine::image_t>::iterator image = wpt.images.begin();
                while(image != wpt.images.end())
                {
                    s1.device()->seek(image->offset);
                    s1 >> image->filePath;
                    s1 >> image->info;
                    s1 >> image->pixmap;
                    ++image;
                }
                break;
            }

            case CPowerLine::eGeoCache:
            {
                quint32 N, n;
                QDataStream s1(&entry->data, QIODevice::ReadOnly);
                s1.setVersion(QDataStream::Qt_4_5);
                wpt.geocache = CPowerLine::geocache_t();
                CPowerLine::geocache_t& cache = wpt.geocache;

                s1 >> (quint8&)cache.service;
                s1 >> cache.hasData;
                s1 >> cache.id;
                s1 >> cache.available;
                s1 >> cache.archived;
                s1 >> cache.difficulty;
                s1 >> cache.terrain;
                s1 >> cache.status;
                s1 >> cache.name;
                s1 >> cache.owner;
                s1 >> cache.ownerId;
                s1 >> cache.type;
                s1 >> cache.container;
                s1 >> cache.shortDesc;
                s1 >> cache.longDesc;
                s1 >> cache.hint;
                s1 >> cache.country;
                s1 >> cache.state;
                s1 >> cache.locale;

                s1 >> N;

                for(n = 0; n < N; n++)
                {
                    CPowerLine::geocachelog_t log;

                    s1 >> log.id;
                    s1 >> log.date;
                    s1 >> log.type;
                    s1 >> log.finderId;
                    s1 >> log.finder;
                    s1 >> log.text;

                    cache.logs << log;
                }

                s1 >> cache.exportBuddies;

                cache.hasData = true;

                break;
            }

            default:;
        }

        ++entry;
    }
    return s;
}


QDataStream& operator <<(QDataStream& s, CPowerLine& wpt)
{
    QList<wpt_head_entry_t> entries;

    //---------------------------------------
    // prepare base data
    //---------------------------------------
    wpt_head_entry_t entryBase;
    entryBase.type = CPowerLine::eBase;
    QDataStream s1(&entryBase.data, QIODevice::WriteOnly);
    s1.setVersion(QDataStream::Qt_4_5);

    s1 << wpt.getKey();
    s1 << wpt.sticky;
    s1 << wpt.timestamp;
    s1 << wpt.iconString;
    s1 << wpt.name;
    s1 << wpt.comment;
    s1 << wpt.lat;
    s1 << wpt.lon;
    s1 << wpt.ele;
    s1 << wpt.prx;
    s1 << wpt.link;
    s1 << wpt.description;
    s1 << wpt.urlname;
    s1 << wpt.type;
    s1 << wpt.getParentWpt();
    s1 << wpt.selected;

    entries << entryBase;

    //---------------------------------------
    // prepare image data
    //---------------------------------------
    wpt_head_entry_t entryImage;
    entryImage.type = CPowerLine::eImage;
    QDataStream s2(&entryImage.data, QIODevice::WriteOnly);
    s2.setVersion(QDataStream::Qt_4_5);

    // write place holder for image offset
    QList<CPowerLine::image_t>::iterator image = wpt.images.begin();
    while(image != wpt.images.end())
    {
        s2 << (quint32)0;
        ++image;
    }
    // offset terminator
    s2 << (quint32)0;

    // write image data and store the actual offset
    image = wpt.images.begin();
    while(image != wpt.images.end())
    {
        image->offset = (quint32)s2.device()->pos();
        s2 << image->filePath;
        s2 << image->info;
        s2 << image->pixmap;
        ++image;
    }

    // finally write image offset table
    (quint32)s2.device()->seek(0);
    image = wpt.images.begin();
    while(image != wpt.images.end())
    {
        s2 << image->offset;
        ++image;
    }

    entries << entryImage;

    //---------------------------------------
    // prepare geocache data
    //---------------------------------------
    if(wpt.geocache.hasData)
    {

        wpt_head_entry_t entryGeoCache;
        entryGeoCache.type = CPowerLine::eGeoCache;
        QDataStream s3(&entryGeoCache.data, QIODevice::WriteOnly);
        s3.setVersion(QDataStream::Qt_4_5);

        CPowerLine::geocache_t& cache = wpt.geocache;

        s3 << (quint8)cache.service;
        s3 << cache.hasData;
        s3 << cache.id;
        s3 << cache.available;
        s3 << cache.archived;
        s3 << cache.difficulty;
        s3 << cache.terrain;
        s3 << cache.status;
        s3 << cache.name;
        s3 << cache.owner;
        s3 << cache.ownerId;
        s3 << cache.type;
        s3 << cache.container;
        s3 << cache.shortDesc;
        s3 << cache.longDesc;
        s3 << cache.hint;
        s3 << cache.country;
        s3 << cache.state;
        s3 << cache.locale;

        s3 << cache.logs.count();

        foreach(const CPowerLine::geocachelog_t& log, cache.logs)
        {
            s3 << log.id;
            s3 << log.date;
            s3 << log.type;
            s3 << log.finderId;
            s3 << log.finder;
            s3 << log.text;
        }

        s3 << cache.exportBuddies;

        entries << entryGeoCache;
    }
    //---------------------------------------
    // prepare terminator
    //---------------------------------------
    wpt_head_entry_t entryEnd;
    entryEnd.type = CPowerLine::eEnd;
    entries << entryEnd;

    //---------------------------------------
    //---------------------------------------
    // now start to actually write data;
    //---------------------------------------
    //---------------------------------------
    // write magic key
    s.writeRawData("QLWpt   ",9);

    // calculate offset table
    quint32 offset = entries.count() * 8 + 9;

    QList<wpt_head_entry_t>::iterator entry = entries.begin();
    while(entry != entries.end())
    {
        entry->offset = offset;
        offset += entry->data.size() + sizeof(quint32);
        ++entry;
    }

    // write offset table
    entry = entries.begin();
    while(entry != entries.end())
    {
        s << entry->type << entry->offset;
        ++entry;
    }

    // write entry data
    entry = entries.begin();
    while(entry != entries.end())
    {
        s << entry->data;
        ++entry;
    }

    return s;
}


void operator >>(QFile& f, CPowerLine& wpt)
{
    f.open(QIODevice::ReadOnly);
    QDataStream s(&f);
    s.setVersion(QDataStream::Qt_4_5);
    s >> wpt;
    f.close();
}


void operator <<(QFile& f, CPowerLine& wpt)
{
    f.open(QIODevice::WriteOnly);
    QDataStream s(&f);
    s.setVersion(QDataStream::Qt_4_5);
    s << wpt;
    f.close();
}*/


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
    angle = atan2(v2 - v1, u2 - u1) * 180.0 / PI;
    
    return QLine(u1, v1, u2, v2);
} // getLine()

const double CPowerLine::getDistance() const {
  double a1, a2;
  XY pt1, pt2;
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
