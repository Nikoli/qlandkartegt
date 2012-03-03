/**********************************************************************************************
    Copyright (C) 2011 Jan Rheinländer jrheinlaender@users.sourceforge.net

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

#include "CMouseEditPower.h"
#include "CDlgEditPowerLine.h"
#include "CDlgEditElectricWpt.h"
#include "CMainWindow.h"
#include "CCanvas.h"

#include <QtGui>

CMouseEditPower::CMouseEditPower(CCanvas * canvas)
: IMouse(canvas)
{
    cursor = QCursor(QPixmap(":/cursors/cursorEditPower.png"),0,0);
}


CMouseEditPower::~CMouseEditPower()
{

}


void CMouseEditPower::mouseMoveEvent(QMouseEvent * e)
{
    mouseMoveEventWpt(e);
}


void CMouseEditPower::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        if(!selWpt.isNull())
        {
            CDlgEditElectricWpt dlg(*selWpt,canvas);
            dlg.exec();
        }

        if(!(selLine == NULL))
        {
            CDlgEditPowerLine dlg(*selLine, canvas);
            dlg.exec();
        }
    }
}


void CMouseEditPower::mouseReleaseEvent(QMouseEvent * e)
{
}


void CMouseEditPower::draw(QPainter& p)
{
    drawSelWpt(p);
}
