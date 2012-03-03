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
#ifndef CDLGEDITELECTRICWPT_H
#define CDLGEDITELECTRICWPT_H

#include <QDialog>

#include "ui_IDlgEditElectricWpt.h"

class CWpt;

/// dialog to edit waypoint electric properties
class CDlgEditElectricWpt : public QDialog, private Ui::IDlgEditElectricWpt
{
    Q_OBJECT;
    public:
        CDlgEditElectricWpt(CWpt &wpt, QWidget * parent);
        virtual ~CDlgEditElectricWpt();

    public slots:
        int exec();
        void accept();

    private:
        CWpt &wpt;
        double oldConsumers;
        double oldConsumers1;
        double oldConsumers2;
        double oldConsumers3;
        double oldLoad;
        double oldLoad1;
        double oldLoad2;
        double oldLoad3;
        double oldPowerfactor;
};
#endif                           //CDLGEditElectricWpt_H
