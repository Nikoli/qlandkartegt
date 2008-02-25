/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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

#ifndef CTRACK_H
#define CTRACK_H

#include <QObject>
#include <QVector>
#include <QColor>
#include <QPolygon>
#include "CWpt.h"

class CTrack : public QObject
{
    Q_OBJECT
    public:
        CTrack(QObject * parent);
        virtual ~CTrack();

        struct pt_t {

            enum flag_e
            {
                 eSelected  = 1       ///< selected by track info view
                ,eCursor    = 2      ///< selected by cursor
                ,eDeleted   = 4      ///< mark point as deleted
                ,eFocus     = 8      ///< mark current point of user focus

            };

            pt_t() : idx(-1), lon(WPT_NOFLOAT), lat(WPT_NOFLOAT), ele(WPT_NOFLOAT), time(0),
                     speed(WPT_NOFLOAT), delta(WPT_NOFLOAT), azimuth(WPT_NOFLOAT), distance(WPT_NOFLOAT), flags(0){}
            qint32  idx;
            /// longitude [�]
            float   lon;
            /// latitude [�]
            float   lat;
            /// elevation [m]
            float   ele;
            quint32 time;

            /// secondary data: the speed between this and the previous point
            float speed;
            /// secondary data: the distance between this and the previous point
            float delta;
            /// secondary data: the azimuth to the next point
            float azimuth;
            /// secondary data: the total distance of all visible points up to this point
            float distance;
            /// display flags
            quint32 flags;

        };

        /// set color by id
        void setColor(unsigned i);
        /// get QT color
        const QColor& getColor(){return color;}
        const unsigned getColorIdx(){return colorIdx;}
        /// set track name
        void setName(const QString& n){name = n;}
        /// get track name
        const QString& getName(){return name;}
        /// get unique track key
        const QString& key();

        /// set the highlight flag
        void setHighlight(bool yes){highlight = yes;}
        /// get the value of the highlight flag
        bool isHighlighted(){return highlight;}

        /// append point to track
        CTrack& operator<<(pt_t& pt);
        /// rebuild secondary track data from primary
        void rebuild(bool reindex);

        QVector<pt_t>& getTrackPoints(){return track;};

        QPolygon& getPolyline(){return polyline;}
    signals:
        void sigChanged();

    private:
        void genKey();
        static const QColor colors[];
        /// unique key to address tarck
        QString _key_;
        /// creation timestamp
        quint32 timestamp;
        /// track name
        QString name;
        /// a comment string
        QString comment;
        /// the track line color
        QColor  color;
        /// the track line color by index
        unsigned colorIdx;
        /// the track points
        QVector<pt_t> track;

        /// set true to draw track highlighted
        bool highlight;

        /// total time covered by all track points
        quint32 totalTime;
        /// total distance of track [m]
        double  totalDistance;

        /// the Qt polyline for faster processing
        QPolygon polyline;

};

#endif //CTRACK_H

