/*
 * CUndoStack.h
 *
 *  Created on: 11.06.2009
 *      Author: feld
 */

#ifndef CUNDOSTACK_H_
#define CUNDOSTACK_H_
#include <QUndoStack>
class QObject;

class CUndoStack : public QUndoStack
{
    public:
        CUndoStack( QObject * parent = 0);
        virtual ~CUndoStack();
        static CUndoStack *getInstance( QObject * parent = 0 );

};
#endif                           /* CUNDOSTACK_H_ */
