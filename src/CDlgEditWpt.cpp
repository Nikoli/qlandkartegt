/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include "CDlgEditWpt.h"
#include "CWpt.h"
#include "CWptDB.h"
#include "WptIcons.h"
#include "GeoMath.h"
#include "CDlgWptIcon.h"
#include "CResources.h"
#include "CMapDB.h"
#include "IMap.h"
#include "IUnit.h"

#include <QtGui>

CDlgEditWpt::CDlgEditWpt(CWpt &wpt, QWidget * parent)
: QDialog(parent)
, wpt(wpt)
, idxImg(0)
{
    setupUi(this);
    connect(pushAdd, SIGNAL(clicked()), this, SLOT(slotAddImage()));
    connect(pushDel, SIGNAL(clicked()), this, SLOT(slotDelImage()));
    connect(pushNext, SIGNAL(clicked()), this, SLOT(slotNextImage()));
    connect(pushPrev, SIGNAL(clicked()), this, SLOT(slotPrevImage()));
    connect(toolIcon, SIGNAL(clicked()), this, SLOT(slotSelectIcon()));
    connect(labelLink, SIGNAL(linkActivated(const QString&)),this,SLOT(slotOpenLink(const QString&)));
    connect(toolLink, SIGNAL(pressed()),this,SLOT(slotEditLink()));

    labelUnitElevation->setText(IUnit::self().baseunit);
    labelUnitProximity->setText(IUnit::self().baseunit);
}


CDlgEditWpt::~CDlgEditWpt()
{

}


int CDlgEditWpt::exec()
{
    QString val, unit;
    toolIcon->setIcon(getWptIconByName(wpt.icon));
    toolIcon->setObjectName(wpt.icon);

    lineName->setText(wpt.name);
    checkSticky->setChecked(wpt.sticky);

    QString pos;
    GPS_Math_Deg_To_Str(wpt.lon, wpt.lat, pos);
    linePosition->setText(pos);

    oldLon = wpt.lon;
    oldLat = wpt.lat;

    //TODO: that has to be metric/imperial
    if(wpt.ele != WPT_NOFLOAT) {
        IUnit::self().meter2elevation(wpt.ele, val, unit);
        lineAltitude->setText(val);
    }
    if(wpt.prx != WPT_NOFLOAT) {
        IUnit::self().meter2elevation(wpt.prx, val, unit);
        lineProximity->setText(val);
    }

    textComment->setPlainText(wpt.comment);

    if(wpt.images.count() != 0) {
        showImage(0);
        pushDel->setEnabled(true);
    }

    link = wpt.link;

    if(!link.isEmpty()) {
        QString str;
        str = "<a href='" + link + "'>" + link + "</a>";
        labelLink->setText(str);
    }

    return QDialog::exec();
}


void CDlgEditWpt::accept()
{
    if(lineName->text().isEmpty()) {
        QMessageBox::warning(0,tr("Error"),tr("You must provide a waypoint indentifier."),QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    if(linePosition->text().isEmpty()) {
        QMessageBox::warning(0,tr("Error"),tr("You must provide a waypoint position."),QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }

    if(!GPS_Math_Str_To_Deg(linePosition->text(), wpt.lon, wpt.lat)) {
        return;
    }
    wpt.icon        = toolIcon->objectName();
    wpt.name        = lineName->text();
    wpt.sticky      = checkSticky->isChecked();

    wpt.ele         = lineAltitude->text().isEmpty() ? WPT_NOFLOAT : IUnit::self().elevation2meter(lineAltitude->text());

    // change elevation if position has changed and DEM data is present
    if(oldLon != wpt.lon || oldLat != wpt.lat) {
        float ele = CMapDB::self().getDEM().getElevation(wpt.lon * DEG_TO_RAD, wpt.lat * DEG_TO_RAD);
        if(ele != WPT_NOFLOAT) wpt.ele = ele;
    }

    wpt.prx         = lineProximity->text().isEmpty() ? WPT_NOFLOAT : IUnit::self().elevation2meter(lineProximity->text());
    wpt.comment     = textComment->toPlainText();
    wpt.link        = link;

    if(!lineDistance->text().isEmpty() && !lineBearing->text().isEmpty()) {
        double bearing  = lineBearing->text().toDouble() * DEG_TO_RAD;
        double distance = lineDistance->text().toDouble();

        XY pt1, pt2;
        pt1.u   = wpt.lon * DEG_TO_RAD;
        pt1.v   = wpt.lat * DEG_TO_RAD;
        pt2     = GPS_Math_Wpt_Projection(pt1, distance, bearing);

        CWpt * wpt2 = new CWpt(&CWptDB::self());
        wpt2->lon = pt2.u * RAD_TO_DEG;
        wpt2->lat = pt2.v * RAD_TO_DEG;
        wpt2->icon = wpt.icon;
        wpt2->name = wpt.name + tr("(proj.)");

        CWptDB::self().addWpt(wpt2);
    }

    emit CWptDB::self().sigChanged();
    emit CWptDB::self().sigModified();

    QDialog::accept();
}


void CDlgEditWpt::slotSelectIcon()
{
    CDlgWptIcon dlg(*toolIcon);
    dlg.exec();
}


void CDlgEditWpt::slotAddImage()
{
    QString filename = QFileDialog::getOpenFileName( 0, tr("Select image file")
        ,"./"
        ,"Image (*)"
        );
    if(filename.isEmpty()) return;

    QString info =  QInputDialog::getText( this, tr("Add comment ..."), tr("comment"), QLineEdit::Normal, QFileInfo(filename).fileName());

    CWpt::image_t img;
    img.info = info;
    img.pixmap = QPixmap(filename);
    wpt.images.push_back(img);
    showImage(wpt.images.count() - 1);

    pushDel->setEnabled(true);

}


void CDlgEditWpt::slotDelImage()
{
    wpt.images.removeAt(idxImg);
    while(idxImg >= wpt.images.count()) --idxImg;
    showImage(idxImg);

    pushDel->setEnabled(wpt.images.count() != 0);
}


void CDlgEditWpt::slotNextImage()
{
    showImage(idxImg + 1);
}


void CDlgEditWpt::slotPrevImage()
{
    showImage(idxImg - 1);
}


void CDlgEditWpt::showImage(int idx)
{
    if(idx < 0) idx = 0;

    if(idx < wpt.images.count()) {
        idxImg = idx;

        CWpt::image_t& img = wpt.images[idx];
        labelImage->setPixmap(img.pixmap.scaledToWidth(100,Qt::SmoothTransformation));
        labelInfo->setText(img.info);

        pushNext->setEnabled(idx < (wpt.images.count() - 1) && wpt.images.count() != 1);
        pushPrev->setEnabled(idx > 0);
    }
    else {
        labelImage->setText(tr("no image"));
        labelInfo->setText("");
    }
}


void CDlgEditWpt::slotOpenLink(const QString& link)
{
    CResources::self().openLink(link);
}


void CDlgEditWpt::slotEditLink()
{
    bool ok = false;
    QString _link = QInputDialog::getText(0,tr("Edit link ..."),tr("Link: 'http://...'"),QLineEdit::Normal,link,&ok);
    if(ok) {
        link = _link;
        labelLink->setText(tr("None"));
    }

    if(!link.isEmpty()) {
        QString str;
        str = "<a href='" + link + "'>" + link + "</a>";
        labelLink->setText(str);
    }
}
