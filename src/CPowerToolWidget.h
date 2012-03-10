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
#ifndef CPOWERTOOLWIDGET_H
#define CPOWERTOOLWIDGET_H

#include <QWidget>
#include <QSet>
#include "CPowerDB.h"
#include "ui_IPowerToolWidget.h"

class CPower;
class QDomDocument;
class QDomElement;
class QHttp;
class QTimer;

class CPowerToolWidget : public QWidget, private Ui::IPowerToolWidget
{
    Q_OBJECT;
    public:
        CPowerToolWidget(QTabWidget * parent);
        virtual ~CPowerToolWidget();

        const QString getFirstSelectedNetwork();

    protected:
        void keyPressEvent(QKeyEvent * e);
        bool eventFilter(QObject *obj, QEvent *event);

    public slots:
        void slotPhaseBalance();
        void slotMaterialUsage();

    private slots:
        void slotDBChanged(const bool highlightOnly);
        void slotDBChanged();
        void slotNWItemClicked(QListWidgetItem * item);
        void slotNWItemDoubleClicked(QListWidgetItem * item);
        void slotNWContextMenu(const QPoint& pos);
        void slotLineItemClicked(QListWidgetItem * item);
        void slotLineItemDoubleClicked(QListWidgetItem * item);
        void slotLineContextMenu(const QPoint& pos);
        void slotAssignToPowerNW(QAction* action);
        void slotEditNW();
        void slotEditLine();
        void slotDeleteNW();
        void slotDeleteLine(); 
        void slotCalcPowerNW();
        void slotCalcPowerNWs();

        void slotNWSelectionChanged();
        void slotLineSelectionChanged();

        void slotZoomToFitNW();
        void slotZoomToFitLine();

        void slotPHChanged() { changeSetup("ph"); }
        void slotVoltageChanged() { changeSetup("voltage"); }
        void slotWattsChanged() { changeSetup("watts"); }
        void slotConductivityChanged() { changeSetup("conductivity"); }
        void slotPowerfactorChanged() { changeSetup("powerfactor"); }
        void slotRatedVoltageChanged() { changeSetup("ratedvoltage"); }

    private:
        bool originator;

        enum tab_e {
            eTabNetwork = 0
            ,eTabLine = 1
            ,eTabSetup = 2
            ,eTabHelp = 3
        };

        void fillListLines(const QString& selected_nw);
        void fillDefaults(const QString& selected_nw);
        void changeSetup(const QString& which);
        void calcPowerNW(const QString& key);
        void getNodeLoads(const QString& wpt_key, const QString& nw_key, const double wpc,
                          const QString& fromLine, QStringList& loads,
                          double& ph1, double& ph2, double& ph3);
        void getMaterials(const QString& wpt_key, const QString& nw_key, const QString& fromLine,
                          QMap<QString, unsigned>& mats);

};
#endif                           //CPOWERTOOLWIDGET_H

