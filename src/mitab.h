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
#ifndef MITAB_H
#define MITAB_H

struct MapInfoDatumInfo {
    int         nMapInfoDatumID;
    const char  *pszOGCDatumName;
    int         nEllipsoid;
    double      dfShiftX;
    double      dfShiftY;
    double      dfShiftZ;
    double      dfDatumParm0; /* RotX */
    double      dfDatumParm1; /* RotY */
    double      dfDatumParm2; /* RotZ */
    double      dfDatumParm3; /* Scale Factor */
    double      dfDatumParm4; /* Prime Meridian */
};

struct MapInfoSpheroidInfo
{
    int         nMapInfoId;
    const char *pszMapinfoName;
    double      dfA; /* semi major axis in meters */
    double      dfInvFlattening; /* Inverse flattening */
};

extern MapInfoDatumInfo asDatumInfoList[];
extern MapInfoSpheroidInfo asSpheroidInfoList[];

#endif //MITAB_H

