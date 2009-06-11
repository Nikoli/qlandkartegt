/*
 * CUndoCommandTrackDelete.h
 *
 *  Created on: 11.06.2009
 *      Author: feld
 */

#ifndef CUNDOCOMMANDTRACKDELETE_H_
#define CUNDOCOMMANDTRACKDELETE_H_
#include <QUndoCommand>
#include <QString>
#include <QMap>
class CTrack;

class CUndoCommandTrackDelete : public QUndoCommand
{
    public:
        CUndoCommandTrackDelete(QMap<QString,CTrack*> *tracks, const QString & key);
        virtual ~CUndoCommandTrackDelete();
        virtual void undo();
        virtual void redo();
    private:
        QMap<QString,CTrack*> *tracks;
        CTrack *track;
        QString key;
};
#endif                           /* CUNDOCOMMANDTRACKDELETE_H_ */
