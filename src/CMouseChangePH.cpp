/**********************************************************************************************
    Copyright (C) 2012 Jan Rheinländer <jrheinlaender@users.sourceforge.net>

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

#include "CMouseChangePH.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "CPowerDB.h"
#include "CPowerNW.h"
#include "CPowerToolWidget.h"

#include <QtGui>

CMouseChangePH::CMouseChangePH(CCanvas * canvas)
: IMouse(canvas)
{
    cursor = QCursor(QPixmap(":/cursors/cursorEdit.png"),0,0);
}


CMouseChangePH::~CMouseChangePH()
{

}


void CMouseChangePH::mouseMoveEvent(QMouseEvent * e)
{
    mouseMoveEventWpt(e);
}


void CMouseChangePH::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        if(!selWpt.isNull())
        {
            QString nw_key = CPowerDB::self().getToolWidget()->getFirstSelectedNetwork();
            CPowerNW * nw = CPowerDB::self().getPowerNWByKey(nw_key);
            nw->ph = selWpt->getKey();
            CPowerDB::self().setPowerNWData(*nw);
            canvas->setMouseMode(CCanvas::eMouseMoveArea);
        }
    }
}


void CMouseChangePH::mouseReleaseEvent(QMouseEvent * e)
{
}


void CMouseChangePH::draw(QPainter& p)
{
    drawSelWpt(p);
}
