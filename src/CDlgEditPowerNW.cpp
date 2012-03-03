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

#include "CDlgEditPowerNW.h"
#include "CPowerNW.h"
#include "CPowerDB.h"
/*#include "CWpt.h"
#include "CWptDB.h"
#include "WptIcons.h"
#include "GeoMath.h"
#include "CDlgWptIcon.h"
#include "CResources.h"
#include "CMapDB.h"
#include "IMap.h"
#include "IUnit.h"*/

//#include "config.h"

#include <QtGui>

CDlgEditPowerNW::CDlgEditPowerNW(CWpt &wpt, CCanvas * parent)
: QDialog(parent)
{
    // Find network which this waypoint belongs to
    // TODO
    
    setupUi(this);
}

CDlgEditPowerNW::CDlgEditPowerNW(CPowerNW &nw, QWidget * parent)
: QDialog(parent)
, network(&nw)

{
    setupUi(this);

}

CDlgEditPowerNW::CDlgEditPowerNW(CPowerNW &nw, CCanvas * parent)
: QDialog(parent)
, network(&nw)

{
    setupUi(this);

}


CDlgEditPowerNW::~CDlgEditPowerNW()
{
}


int CDlgEditPowerNW::exec()
{

    oldName    = network->getName();
    oldVoltage = network->voltage;
    oldWatts = network->watts;
    
    lineName->setText(oldName);
    lineVoltage->setText(QString::number(oldVoltage));
    lineWatts->setText(QString::number(oldWatts));

    return QDialog::exec();
}


void CDlgEditPowerNW::accept()
{

    QString name = lineName->text();
    double voltage = lineVoltage->text().toDouble();
    double watts = lineWatts->text().toDouble();
    
    bool changed = false;
    
    if (name != oldName) {
      network->setName(name);
      changed = true;
    }
    
    if (fabs(voltage - oldVoltage) > 0.01) {
      network->voltage = voltage;
      changed = true;
    }
    
    if (fabs(watts - oldWatts) > 0.01) {
      network->watts = watts;
      changed = true;
    }
    
    if (changed) {    
      CPowerDB::self().setPowerNWData(*network);
      // TODO: Recalculate network
    }

    QDialog::accept();
}
