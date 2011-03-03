/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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

#ifndef IITEM_H
#define IITEM_H

#include <QObject>
#include <QPixmap>

/// interface class for all geo items to unify basic data and access
class IItem : public QObject
{
    Q_OBJECT;
    public:
        IItem(QObject * parent);
        virtual ~IItem();

        /// set the item's short name
        virtual void setName(const QString& str){name = str;}
        /// get the item's short name
        virtual QString getName(){return name;}

        /// set the item's long comment
        virtual void setComment(const QString& str){comment = str;}
        /// get the item's long comment
        virtual QString getComment(){return comment;}

        /// set the item's description
        virtual void setDescription(const QString& str){description = str;}
        /// get the item's description
        virtual QString getDescription(){return description;}

        /// get a summary of item's data to display on screen or in the toolview
        virtual QString getInfo()= 0;

        /// set the icon defined by a string
        virtual void setIcon(const QString& str) = 0;
        /// get the icon as pixmap
        virtual QPixmap getIcon(){return iconPixmap;}
        /// get the icon as string definition
        virtual QString getIconString(){return iconString;}

        /// set the name of the parent waypoint
        virtual void setParentWpt(const QString& name){parentWpt = name;}
        /// get the name of the parent waypoint
        virtual QString getParentWpt(){ return parentWpt;}

        /// set the internal unique key
        /**
            CAUTION! Use it wisely. Usually the key is generated by the item.
        */
        virtual void setKey(const QString& str){key = str;}
        /// get the unique key
        virtual QString getKey();

        virtual quint32 getTimestamp(){return timestamp;}

        static void resetKeyCnt(){keycnt = 0;}

    protected:

        quint32 timestamp;

        QString name;
        QString comment;
        QString description;
        QPixmap iconPixmap;
        QString iconString;

        QString parentWpt;

        static quint32 keycnt;
    private:
        virtual void genKey();
        QString key;

};

#endif //IITEM_H

