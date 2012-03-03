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

#include "CDlgEditPowerLine.h"
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

CDlgEditPowerLine::CDlgEditPowerLine(CPowerLine &l, QWidget * parent)
: QDialog(parent)
, line(l)

{
    setupUi(this);

}

CDlgEditPowerLine::CDlgEditPowerLine(CPowerLine &l, CCanvas * parent)
: QDialog(parent)
, line(l)

{
    setupUi(this);

}


CDlgEditPowerLine::~CDlgEditPowerLine()
{

}


int CDlgEditPowerLine::exec()
{
    oldName     = line.getName();
    oldCrossSection = line.getCrossSection();
    oldPhases = line.getPhases();
    oldLength = line.getLength();
    oldConductivity = line.getConductivity();
    
    lineName->setText(oldName);
    spinCrossSection->setValue(oldCrossSection);
    checkPhase1->setChecked(oldPhases & 1);
    checkPhase2->setChecked(oldPhases & 2);
    checkPhase3->setChecked(oldPhases & 4);
    spinLength->setValue(oldLength);
    spinConductivity->setValue(oldConductivity);

    return QDialog::exec();
}


void CDlgEditPowerLine::accept()
{
    QString name    = lineName->text();
    double crossSection = spinCrossSection->value();
    unsigned phases =
            (checkPhase1->isChecked() ? 1 : 0) +
            (checkPhase2->isChecked() ? 2 : 0) +
            (checkPhase3->isChecked() ? 4 : 0);
    double length   = spinLength->value();
    double conductivity = spinConductivity->value();
    
    bool changed = false;
    
    if (name != oldName) {
      line.setName(name);
      changed = true;
    }
    
    if (fabs(crossSection - oldCrossSection) > 0.01) {
      line.setCrossSection(crossSection);
      changed = true;
    }
    
    if (phases != oldPhases) {
        line.setPhases(phases);
      changed = true;
    }
      
    if (fabs(length - oldLength) > 0.01) {
      line.setLength(length);
      changed = true;
    }

    if (fabs(conductivity - oldConductivity) > 0.01) {
        line.setConductivity(conductivity);
        changed = true;
      }
    
    if (changed) {    
        qDebug() << "Powerline data changed";
        CPowerDB::self().setPowerLineData(line);
    }

    QDialog::accept();
}
