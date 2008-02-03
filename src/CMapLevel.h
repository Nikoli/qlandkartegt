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

#ifndef CMAPLEVEL_H
#define CMAPLEVEL_H

#include <projects.h>
#include <QObject>
#include <QVector>

class CMapRaster;
class CMapFile;

/// data object to define a resolution level
/**

    A map set is defined by one or several CMapLevel objects. Each CMapLevel
    holds a list of GeoTiff files (CMapFile) with the <b>same</b> projection and
    resolution. Additionally it stores the minimum and maximum zoom level these files
    should be displayed. The zoom levels must be positive integers continously distributed
    over the different map levels. All maplevels cover (more or less) the same area.

    <pre>
    1 .. 3   -> level 1 (highest resolution)
    4 .. 7   -> level 2
    8 .. 99  -> level 3 (lowest resolution)
    </pre>
*/
class CMapLevel : public QObject
{
    Q_OBJECT
    public:
        /**
            @param min minimum zoom level
            @param max maximum zoom level
            @param parent the raster map object using that level definition
        */
        CMapLevel(quint32 min, quint32 max, CMapRaster * parent);
        virtual ~CMapLevel();

        /// add a GeoTiff file by filename
        void addMapFile(const QString& filename);

        /// add a GeoTiff file with DEM data
        void addDEMFile(const QString& filename);

        /// get iterator to first GeoTiff file in list
        QVector<CMapFile*>::const_iterator begin(){return mapfiles.begin();}
        /// end iterator of GeoTiff file list
        QVector<CMapFile*>::const_iterator end(){return mapfiles.end();}

        /// minimum zoom level
        const quint32 min;
        /// maximum zoom level
        const quint32 max;

        void dimensions(double& lon1, double& lat1, double& lon2, double& lat2);

    private:
        QVector<CMapFile*> mapfiles;

        PJ * pjtar;
        PJ * pjsrc;

        double westbound;
        double northbound;
        double eastbound;
        double southbound;

};

#endif //CMAPLEVEL_H

