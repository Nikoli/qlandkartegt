/*
 * CUndoCommandTrackDelete.cpp
 *
 *  Created on: 11.06.2009
 *      Author: feld
 */

#include "CUndoCommandTrackDelete.h"
#include <QMap>
#include <QString>
#include <QDebug>
#include "CTrack.h"
#include "CTrackDB.h"
#include "CTrackToolWidget.h"

CUndoCommandTrackDelete::CUndoCommandTrackDelete(QMap<QString,CTrack*> *tracks, const QString &key)
: tracks(tracks), key(key)
{
    setText("delte track");
    track = 0;
}


CUndoCommandTrackDelete::~CUndoCommandTrackDelete()
{
    qDebug() << Q_FUNC_INFO << track << track->ref;
    if (track->ref < 1)
        delete track;
}


void CUndoCommandTrackDelete::redo()
{
    qDebug() << Q_FUNC_INFO;
    track = tracks->take(key);
    track->ref++;
    // Just for testing slotDBChanged should be private and done only once
    CTrackDB::self().getToolWidget()->slotDBChanged();
}


void CUndoCommandTrackDelete::undo()
{
    qDebug() << Q_FUNC_INFO << track;
    tracks->insert(key, track);
    track->ref--;
    // Just for testing slotDBChanged should be private
    CTrackDB::self().getToolWidget()->slotDBChanged();
}
