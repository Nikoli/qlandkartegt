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
#ifndef CDLGEDITPOWERNW_H
#define CDLGEDITPOWERNW_H

#include <QDialog>

#include "ui_IDlgEditPowerNW.h"
#include "CCanvas.h"

class CPowerNW;

/// dialog to edit power network properties
class CDlgEditPowerNW : public QDialog, private Ui::IDlgEditPowerNW
{
    Q_OBJECT;
    public:
        CDlgEditPowerNW(CPowerNW &nw, QWidget * parent);
        CDlgEditPowerNW(CPowerNW &nw, CCanvas * parent);
        CDlgEditPowerNW(CWpt &wpt, CCanvas * parent);
        virtual ~CDlgEditPowerNW();

    public slots:
        int exec();
        void accept();

    private:
        CPowerNW *network;
        QString oldName;
        double oldVoltage;
        double oldWatts;
};
#endif                           //CDLGEditPowerNW_H

