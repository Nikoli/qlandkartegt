/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de
    Copyright (C) 2010 Joerg Wunsch <j@uriah.heep.sax.de>

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

#include "IDB.h"

#include <QtGui>
#include <QString>
#include <QDateTime>

#include "CWptDB.h"
#include "CTrackDB.h"

IDB::IDB(QTabWidget * tb, QObject * parent)
: QObject(parent)
, tabbar(tb)
{

}


IDB::~IDB()
{

}


void IDB::gainFocus()
{
    if(toolview && tabbar->currentWidget() != toolview)
    {
        tabbar->setCurrentWidget(toolview);
    }
}

QDateTime IDB::parseTimestamp(const QString &timetext, int& tzoffset)
{
     const QRegExp tzRE("[-+]\\d\\d:\\d\\d$");
     int i;

     tzoffset = 0;

     QString format = "yyyy-MM-dd'T'hh:mm:ss";
     if (timetext.indexOf(".") != -1) format += ".zzz";
     // trailing "Z" explicitly declares the timestamp to be UTC
     if (timetext.indexOf("Z") != -1)
     {
          format += "'Z'";
     }
     else if ((i = tzRE.indexIn(timetext)) != -1)
     {
          // trailing timezone offset [-+]HH:MM present
          // This does not match the original intentions of the GPX
          // file format but appears to be found occasionally in
          // the wild.  Try our best parsing it.

          // add the literal string to the format so fromString()
          // will succeed
          format += "'";
          format += timetext.right(6);
          format += "'";

          // calculate the offset
          int offsetHours(timetext.mid(i + 1, 2).toUInt());
          int offsetMinutes(timetext.mid(i + 4, 2).toUInt());
          if (timetext[i] == '-')
          {
               tzoffset = -(60 * offsetHours + offsetMinutes);
          }
          else
          {
               tzoffset = 60 * offsetHours + offsetMinutes;
          }
          tzoffset *= 60; // seconds
     }

     QDateTime datetime = QDateTime::fromString(timetext, format);

     return datetime;
}


static bool parseTstampInternal(const QString &timetext, quint32 &tstamp,
                                bool do_msec, quint32 &tstamp_msec)
{

    int tzoffset;
    QDateTime datetime = IDB::parseTimestamp(timetext, tzoffset);

    if (!datetime.isValid())
    {
      return false;
    }

    datetime.setTimeSpec(Qt::UTC);

    tstamp = datetime.toTime_t();
    tstamp -= tzoffset;

    if (do_msec)
    {
      tstamp_msec = datetime.time().msec();
    }

    return true;
}

bool IDB::parseTimestamp(const QString &time, quint32 &tstamp)
{
     quint32 dummy;

     return parseTstampInternal(time, tstamp, false, dummy);
}

bool IDB::parseTimestamp(const QString &time, quint32 &tstamp,
                         quint32 &tstamp_msec)
{
     return parseTstampInternal(time, tstamp, true, tstamp_msec);
}

void IDB::signalsOff()
{
    CWptDB::self().blockSignals(true);
    CTrackDB::self().blockSignals(true);
}

void IDB::signalsOn()
{
    CWptDB::self().blockSignals(false);
    CWptDB::self().emitSigChanged();
    CTrackDB::self().blockSignals(false);
    CTrackDB::self().emitSigChanged();
}
