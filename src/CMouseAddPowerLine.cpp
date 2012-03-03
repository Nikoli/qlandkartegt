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
#include "CMouseAddPowerLine.h"
#include "CCanvas.h"
#include "CDlgEditWpt.h"
#include "CWptDB.h"
//#include "CMapDB.h"
#include "CPowerDB.h"

#include <QtGui>

CMouseAddPowerLine::CMouseAddPowerLine(CCanvas * canvas)
: IMouse(canvas)
{
    cursor = QCursor(QPixmap(":/cursors/cursorAdd.png"),0,0);
}


CMouseAddPowerLine::~CMouseAddPowerLine()
{

}


void CMouseAddPowerLine::mouseMoveEvent(QMouseEvent * e)
{
    mouseMoveEventWpt(e);
}


void CMouseAddPowerLine::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        if(!selWpt.isNull())
        {
            if (prevWpt == "")
            {
                prevWpt = selWpt->getKey();
                selWpt->selected = true;
            } else if (selWpt->getKey() != prevWpt) {
                CPowerDB::self().newPowerLine(prevWpt, selWpt->getKey());
                CWpt * prev = CWptDB::self().getWptByKey(prevWpt);
                if (prev != NULL) prev->selected = false;
                prevWpt = selWpt->getKey();
                selWpt->selected = true;
            } // else do nothing
        } else {
          CWpt * prev = CWptDB::self().getWptByKey(prevWpt);
          if (prev != NULL) prev->selected = false;
          prevWpt = "";
        }
    }
}


void CMouseAddPowerLine::mouseReleaseEvent(QMouseEvent * e)
{
}

void CMouseAddPowerLine::draw(QPainter& p)
{
    drawSelWpt(p);
}
