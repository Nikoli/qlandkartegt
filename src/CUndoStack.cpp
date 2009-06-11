/*
 * CUndoStack.cpp
 *
 *  Created on: 11.06.2009
 *      Author: feld
 */

#include "CUndoStack.h"

CUndoStack::CUndoStack(QObject * parent): QUndoStack(parent)
{

}


CUndoStack::~CUndoStack()
{

}


CUndoStack *CUndoStack::getInstance( QObject * parent)
{
    static CUndoStack *instance = 0;

    if (!instance)
        instance = new CUndoStack(parent);

    return instance;

}
