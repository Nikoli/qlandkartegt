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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#ifndef WPTICONS_H
#define WPTICONS_H

#include <QString>
#include <QPixmap>

//struct wpt_icon_t
//{
//    QString icon;
//    QString name;
//};

#define N_CUSTOM_ICONS 24

extern void initWptIcons();
extern QPixmap loadIcon(const QString& path);
extern const QMap<QString, QString>& getWptIcons();
extern QPixmap getWptIconByName(const QString& name, QString * src = 0);
extern void setWptIconByName(const QString& name, const QString& filename);
extern void setWptIconByName(const QString& name, const QPixmap& icon);
#endif                           //WPTICONS_H
