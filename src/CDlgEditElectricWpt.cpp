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

#include "CDlgEditElectricWpt.h"
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

CDlgEditElectricWpt::CDlgEditElectricWpt(CWpt &wpt, QWidget * parent)
: QDialog(parent)
, wpt(wpt)

{
    setupUi(this);

}


CDlgEditElectricWpt::~CDlgEditElectricWpt()
{

}


int CDlgEditElectricWpt::exec()
{

    CPowerDB::wpt_eElectric electricData = CPowerDB::self().getElectricData(wpt.getKey());
    oldConsumers = electricData.consumers;
    oldConsumers1 = electricData.consumers1;
    oldConsumers2 = electricData.consumers2;
    oldConsumers3 = electricData.consumers3;
    oldLoad = electricData.load;
    oldLoad1 = electricData.loadphase1;
    oldLoad2 = electricData.loadphase2;
    oldLoad3 = electricData.loadphase3;
    oldPowerfactor = electricData.powerfactor;
    
    spinConsumers->setValue(oldConsumers);
    spinConsumers1->setValue(oldConsumers1);
    spinConsumers2->setValue(oldConsumers2);
    spinConsumers3->setValue(oldConsumers3);
    spinLoad->setValue(oldLoad);
    spinLoad1->setValue(oldLoad1);
    spinLoad2->setValue(oldLoad2);
    spinLoad3->setValue(oldLoad3);
    spinPowerfactor->setValue(oldPowerfactor);

    return QDialog::exec();
}


void CDlgEditElectricWpt::accept()
{

    double consumers = spinConsumers->value();
    double consumers1 = spinConsumers1->value();
    double consumers2 = spinConsumers2->value();
    double consumers3 = spinConsumers3->value();
    double load = spinLoad->value();
    double load1 = spinLoad1->value();
    double load2 = spinLoad2->value();
    double load3 = spinLoad3->value();
    double powerfactor = spinPowerfactor->value();
    
    CPowerDB::wpt_eElectric e;
    e.consumers = oldConsumers;
    e.consumers1 = oldConsumers1;
    e.consumers2 = oldConsumers2;
    e.consumers3 = oldConsumers3;
    e.load = oldLoad;
    e.loadphase1 = oldLoad1;
    e.loadphase2 = oldLoad2;
    e.loadphase3 = oldLoad3;
    e.powerfactor = oldPowerfactor;
    bool changed = false;
    
    if (fabs(consumers - oldConsumers) > 0.01) {
      e.consumers = consumers;
      changed = true;
    }

    if (fabs(consumers1 - oldConsumers1) > 0.01) {
        e.consumers1 = consumers1;
        changed = true;
    }

    if (fabs(consumers2 - oldConsumers2) > 0.01) {
        e.consumers2 = consumers2;
        changed = true;
    }

    if (fabs(consumers3 - oldConsumers3) > 0.01) {
        e.consumers3 = consumers3;
        changed = true;
    }
      
    if (fabs(load - oldLoad) > 0.01) {
      e.load = load;
      changed = true;
    }

    if (fabs(load1 - oldLoad1) > 0.01) {
      e.loadphase1 = load1;
      changed = true;
    }

    if (fabs(load2 - oldLoad2) > 0.01) {
      e.loadphase2 = load2;
      changed = true;
    }

    if (fabs(load3 - oldLoad3) > 0.01) {
      e.loadphase3 = load3;
      changed = true;
    }

    if (fabs(powerfactor - oldPowerfactor) > 0.01) {
      e.powerfactor = powerfactor;
      changed = true;
    }
    
    if (changed) {
        // Ensure totals match
        if (e.consumers1 + e.consumers2 + e.consumers3 > 0.1)
            e.consumers = e.consumers1 + e.consumers2 + e.consumers3;
        if (e.loadphase1 + e.loadphase2 + e.loadphase3 > 0.1)
            e.load = e.loadphase1 + e.loadphase2 + e.loadphase3;

        CPowerDB::self().setElectricData(wpt.getKey(), e);
    }

    QDialog::accept();
}

