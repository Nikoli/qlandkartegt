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

#include "CMapSearchWidget.h"
#include "CMainWindow.h"
#include "CCanvas.h"
#include "GeoMath.h"
#include "CMapDB.h"
#include "CPicProcess.h"
#include "CMapSearchCanvas.h"
#include "CTabWidget.h"
#include "CImage.h"

#include <QtGui>

CMapSearchWidget::CMapSearchWidget(QWidget * parent)
: QWidget(parent)
{
    setupUi(this);
    setObjectName("CMapSearchWidget");
    setAttribute(Qt::WA_DeleteOnClose,true);
    toolExit->setIcon(QIcon(":/icons/iconExit16x16.png"));
    connect(toolExit, SIGNAL(clicked()), this, SLOT(close()));

    connect(pushArea, SIGNAL(clicked()), this, SLOT(slotSelectArea()));
    connect(pushMask, SIGNAL(clicked()), this, SLOT(slotSelectMask()));
    connect(pushSearch, SIGNAL(clicked()), this, SLOT(slotSearch()));

    connect(sliderThreshold, SIGNAL( valueChanged (int)), this, SLOT(slotThreshold(int)));
}


CMapSearchWidget::~CMapSearchWidget()
{
    if(!canvas.isNull()) {
        delete canvas;
    }

}


void CMapSearchWidget::slotSelectArea()
{
    theMainWindow->getCanvas()->setMouseMode(CCanvas::eMouseSelectArea);
}


void CMapSearchWidget::slotSelectMask()
{
    labelMask->setText(tr("No mask selected."));
    binarizeViewport(-1);
}



void CMapSearchWidget::slotSearch()
{
    if(!mask.size().isValid()) return;
    CImage imgMask(mask);

    CImage xxx(CMapDB::self().getMap().getBuffer());
    xxx.binarize(sliderThreshold->value());

    QList<QPoint> symbols;
    xxx.findSymbol(symbols, imgMask);
}

void CMapSearchWidget::slotThreshold(int i)
{
    binarizeViewport(sliderThreshold->value());
}

void CMapSearchWidget::slotMaskSelection(const QPixmap& pixmap)
{
    mask = pixmap;

    CImage img(mask);
    labelMask->setPixmap(QPixmap::fromImage(img.mask()));
}

void CMapSearchWidget::setArea(const CMapSelection& ms)
{
    area = ms;
    QString pos1, pos2;

    GPS_Math_Deg_To_Str(ms.lon1 * RAD_TO_DEG, ms.lat1 * RAD_TO_DEG, pos1);
    GPS_Math_Deg_To_Str(ms.lon2 * RAD_TO_DEG, ms.lat2 * RAD_TO_DEG, pos2);

    labelArea->setText(QString("%1\n%2\n%3").arg(ms.description).arg(pos1).arg(pos2));
}


void CMapSearchWidget::binarizeViewport(int t)
{
    CImage xxx(CMapDB::self().getMap().getBuffer());

    if(canvas.isNull()) {
        canvas = new CMapSearchCanvas(this);
        connect(canvas, SIGNAL(sigSelection(const QPixmap&)), this, SLOT(slotMaskSelection(const QPixmap&)));
        theMainWindow->getCanvasTab()->addTab(canvas, tr("Symbols"));

    }

    canvas->setBuffer(QPixmap::fromImage(xxx.binarize(t)));

    if(t < 0)sliderThreshold->setValue(xxx.getThreshold());
}
