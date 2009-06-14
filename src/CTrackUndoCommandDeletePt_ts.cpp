//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- Copyright (c) 2009 Marc Feld
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C-  ------------------------------------------------------------------

#include "CTrackUndoCommandDeletePt_ts.h"
#include "CTrackDB.h"
#include <QDebug>
CTrackUndoCommandDeletePt_ts::CTrackUndoCommandDeletePt_ts(CTrack *track)
: track(track)
{

}


CTrackUndoCommandDeletePt_ts::~CTrackUndoCommandDeletePt_ts()
{

}


void CTrackUndoCommandDeletePt_ts::redo()
{
    //qDebug() << Q_FUNC_INFO;
    oldList.clear();
    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    QList<CTrack::pt_t>::iterator trkpt   = trkpts.begin();
    while(trkpt != trkpts.end()) {
        oldList.append(*trkpt);
        if(trkpt->flags & CTrack::pt_t::eSelected) {
            trkpt = trkpts.erase(trkpt);
        }
        else {
            ++trkpt;
        }
    }
    emit CTrackDB::self().emitSigModified();
    emit CTrackDB::self().emitSigChanged();
}


void CTrackUndoCommandDeletePt_ts::undo()
{
    //qDebug() << Q_FUNC_INFO;
    QList<CTrack::pt_t>& trkpts           = track->getTrackPoints();
    trkpts.clear();

    foreach(CTrack::pt_t tp, oldList) {
        trkpts.append(tp);
    }
    track->rebuild(true);
    emit CTrackDB::self().emitSigModified();
    emit CTrackDB::self().emitSigChanged();
}