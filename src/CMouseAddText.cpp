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

#include "CMouseAddText.h"
#include "CCanvas.h"
#include "CMapDB.h"
#include "COverlayDB.h"
#include "COverlayText.h"
#include <QtGui>

CMouseAddText::CMouseAddText(CCanvas * canvas)
: IMouse(canvas)
, selArea(false)
{
    cursor = QCursor(QPixmap(":/cursors/cursorText"),0,0);
}


CMouseAddText::~CMouseAddText()
{

}


void CMouseAddText::draw(QPainter& p)
{

    if(selArea) {
        p.setBrush(Qt::white);
        p.setPen(Qt::black);
        p.drawRect(rect);
    }
}


void CMouseAddText::mouseMoveEvent(QMouseEvent * e)
{
    if(selArea) {
        resizeRect(e->pos());
    }
}


void CMouseAddText::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        if(!selArea) {
            startRect(e->pos());
            selArea = true;
        }
    }
}


void CMouseAddText::mouseReleaseEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton) {
        if(selArea) {
            resizeRect(e->pos());
            selArea     = false;
            COverlayDB::self().addText("",rect);
            canvas->setMouseMode(CCanvas::eMouseMoveArea);
        }
    }
}
