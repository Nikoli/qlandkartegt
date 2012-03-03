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
#include "CWptDB.h"
#include "CPowerToolWidget.h"
#include "CPowerDB.h"
#include "CPowerNW.h"
#include "CMapDB.h"
#include "CDlgEditPowerNW.h"
#include "CMegaMenu.h"

#include <QtGui>
#include <QSqlQuery>
#include <QSqlError>

#define QUERY_EXEC(cmd) \
if(!query.exec())\
{\
    qDebug() << query.lastQuery();\
    qDebug() << query.lastError();\
    cmd;\
}\

#define N_LINES 3

// Voltage at next node assumed to be voltage at current node for power calculation
// Traditional RESAP as in distribunet
//#define CALCMETHOD 1
// Current calculated from resistance of the load as U_rated²/P_rated * PF
// Assuming balanced three-phase system
// Approximation of single-phase loads in three-phase systems, which assumes that
// all single-phase loads are balanced on the three-phase node the originate from,
// including the single-phase line resistances!
#define CALCMETHOD 2

CPowerToolWidget::CPowerToolWidget(QTabWidget * parent)
: QWidget(parent)
, originator(false)
{
    setupUi(this);
    setObjectName("Electric");
    parent->addTab(this,QIcon(":/icons/iconNetworks16x16.png"),"");
    parent->setTabToolTip(parent->indexOf(this), tr("Networks"));

    connect(&CPowerDB::self(), SIGNAL(sigChanged(const bool)), this, SLOT(slotDBChanged(const bool)));
    connect(&CWptDB::self(), SIGNAL(sigChanged()), this, SLOT(slotDBChanged()));

    // Lines tab
    connect(listLines,SIGNAL(itemClicked(QListWidgetItem*) ),this,SLOT(slotLineItemClicked(QListWidgetItem*)));
    connect(listLines,SIGNAL(itemDoubleClicked(QListWidgetItem*) ),this,SLOT(slotLineItemDoubleClicked(QListWidgetItem*)));
    connect(listLines,SIGNAL(itemSelectionChanged() ),this,SLOT(slotLineSelectionChanged()));
    connect(listLines,SIGNAL(customContextMenuRequested(const QPoint&) ),this,SLOT(slotLineContextMenu(const QPoint&)));

    // Networks tab
    connect(listNetworks,SIGNAL(itemClicked(QListWidgetItem*) ),this,SLOT(slotNWItemClicked(QListWidgetItem*)));
    connect(listNetworks,SIGNAL(itemDoubleClicked(QListWidgetItem*) ),this,SLOT(slotNWItemDoubleClicked(QListWidgetItem*)));
    connect(listNetworks,SIGNAL(itemSelectionChanged() ),this,SLOT(slotNWSelectionChanged()));
    connect(listNetworks,SIGNAL(customContextMenuRequested(const QPoint&) ),this,SLOT(slotNWContextMenu(const QPoint&)));

    // Setup tab
    connect(linePH,SIGNAL(textChanged(QString)),this,SLOT(slotPHChanged()));
    connect(spinVoltage,SIGNAL(valueChanged(int)),this,SLOT(slotVoltageChanged()));
    connect(spinWatts,SIGNAL(valueChanged(int)),this,SLOT(slotWattsChanged()));
    connect(spinConductivity,SIGNAL(valueChanged(int)),this,SLOT(slotConductivityChanged()));
    connect(spinPF,SIGNAL(valueChanged(double)),this,SLOT(slotPowerfactorChanged()));
    connect(spinRatedVoltage,SIGNAL(valueChanged(double)),this,SLOT(slotRatedVoltageChanged()));

    tabWidget->setTabIcon(eTabNetwork, QIcon(":/icons/iconPowerNW16x16.png"));
    tabWidget->setTabIcon(eTabLine, QIcon(":/icons/iconPowerLine16x16.png"));
    tabWidget->setTabIcon(eTabSetup, QIcon(":/icons/iconConfig16x16.png"));
    tabWidget->setTabIcon(eTabHelp, QIcon(":/icons/iconHelp16x16.png"));

    listNetworks->installEventFilter(this);
    listLines->installEventFilter(this);
}

CPowerToolWidget::~CPowerToolWidget()
{

}

const QString CPowerToolWidget::getFirstSelectedNetwork()
{
  const QList<QListWidgetItem*>& items = listNetworks->selectedItems();
  return (items.isEmpty() ? "" : (*items.begin())->data(Qt::UserRole).toString());
}

void CPowerToolWidget::fillListLines(const QString& selected_nw)
{
    //qDebug() << "CPowerToolWidget::fillListLines() for " << selected_nw;
    QFontMetrics fm(listNetworks->font());
    QPixmap icon(16,N_LINES*fm.height());
    icon.fill(Qt::white);

    fm = QFontMetrics(listLines->font());
    icon = QPixmap(16,N_LINES*fm.height());
    icon.fill(Qt::white);

    listLines->clear();
    listLines->setIconSize(icon.size());

    QListWidgetItem * highlighted = 0;

    QString key;
    QStringList keys = CPowerDB::self().getPowerLines();

    //qDebug() << "Selected nw key " << selected_nw;

    foreach(key, keys)
    {
        CPowerLine * l = CPowerDB::self().getPowerLineByKey(key);

        if ((l->keyNetwork == selected_nw) || (selected_nw == ""))
            // only show items corresponding to selected network
        {
            QListWidgetItem * item = new QListWidgetItem(listLines);

            icon.fill(Qt::transparent);
            QPainter p;
            p.begin(&icon);
            p.drawPixmap(0,0,l->getIcon());
            p.end();

            item->setText(l->getInfo());
            item->setData(Qt::UserRole, l->getKey());
            item->setIcon(icon);

            if(CPowerDB::self().isHighlightedPowerLine(key))
            {
                highlighted = item;
            }
        }
        ++l;
    }

    if(highlighted)
    {
        listLines->setCurrentItem(highlighted);
    }
}

void CPowerToolWidget::fillDefaults(const QString& selected_nw)
{
    if (selected_nw != "")
    {
        CPowerNW * nw = CPowerDB::self().getPowerNWByKey(selected_nw);
        CWpt * wpt = CWptDB::self().getWptByKey(nw->ph);

        linePH->setText((wpt == NULL ? "" : wpt->getName()));
        spinVoltage->setValue(nw->voltage);
        spinWatts->setValue(nw->watts);
        spinConductivity->setValue(nw->conductivity);
        spinPF->setValue(nw->powerfactor);
        spinRatedVoltage->setValue(nw->ratedVoltage);
    }
}

void CPowerToolWidget::slotDBChanged()
{
    slotDBChanged(false);
}

void CPowerToolWidget::slotDBChanged(const bool highlightOnly)
{
    if(originator) return;
    qDebug() << "CPowerToolWidget::slotDBChanged(), highlightOnly=" << (highlightOnly ? "true" : "false");
    
    if (highlightOnly)
    {
        CPowerNW* highlighted_nw = CPowerDB::self().highlightedPowerNW();
        fillListLines(highlighted_nw == NULL ? "" : highlighted_nw->getKey());
        return;
    }

    // Networks
    QFontMetrics fm(listNetworks->font());
    QPixmap icon(16,N_LINES*fm.height());
    icon.fill(Qt::white);

    listNetworks->clear();
    listNetworks->setIconSize(icon.size());

    QListWidgetItem * highlighted = 0;
    QString highlighted_nw_key = "";

    QString key;
    QStringList keys = CPowerDB::self().getPowerNWs();

    foreach(key, keys)
    {
        CPowerNW * nw = CPowerDB::self().getPowerNWByKey(key);

        QListWidgetItem * item = new QListWidgetItem(listNetworks);

        icon.fill(Qt::transparent);
        QPainter p;
        p.begin(&icon);
        p.drawPixmap(0,0,nw->getIcon());
        p.end();

        item->setText(nw->getInfo());
        item->setData(Qt::UserRole, nw->getKey());
        item->setIcon(icon);

        if(CPowerDB::self().isHighlightedPowerNW(key))
        {
            highlighted = item;
            highlighted_nw_key = nw->getKey();
        }

        ++nw;
    }

    if(highlighted)
    {
        listNetworks->setCurrentItem(highlighted);
    }
    
    // Lines
    fillListLines(highlighted_nw_key);
    fillDefaults(highlighted_nw_key);
    slotCalcPowerNWs();
}


void CPowerToolWidget::slotNWItemClicked(QListWidgetItem * item)
{
    //qDebug() << "slotNWItemClicked";
    originator = true;
    QString key = item->data(Qt::UserRole).toString();
    CPowerDB::self().highlightPowerNW(key);
    fillListLines(key);
    fillDefaults(key);
    originator = false;
}

void CPowerToolWidget::slotLineItemClicked(QListWidgetItem * item)
{
    originator = true;
    CPowerDB::self().highlightPowerLine(item->data(Qt::UserRole).toString());
    originator = false;
}


void CPowerToolWidget::slotNWItemDoubleClicked(QListWidgetItem * item)
{
    QString key = item->data(Qt::UserRole).toString();

    QRectF r = CPowerDB::self().getPowerNWByKey(key)->getBoundingRectF();
    if (!r.isNull ())
    {
        CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
    }

    fillListLines(key);
    fillDefaults(key);
}

void CPowerToolWidget::slotLineItemDoubleClicked(QListWidgetItem * item)
{
    //QString key = item->data(Qt::UserRole).toString();

/*    QRectF r = CPowerDB::self().getBoundingRectF(key);
    if (!r.isNull ())
    {
        CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
    }*/
}


void CPowerToolWidget::keyPressEvent(QKeyEvent * e)
{
    if(e->key() == Qt::Key_Delete)
    {
        (tabWidget->currentIndex() == 0) ? slotDeleteNW() : slotDeleteLine();
        e->accept();
    }
    else
    {
        QWidget::keyPressEvent(e);
    }
}


void CPowerToolWidget::slotNWContextMenu(const QPoint& pos)
{
    qDebug() << "slotNWContextMenu()";
    QListWidgetItem * item = listNetworks->currentItem();
    if(item)
    {
        QPoint p = listNetworks->mapToGlobal(pos);

        QMenu contextMenu;
        contextMenu.addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit"),this,SLOT(slotEditNW()));
        contextMenu.addAction(QPixmap(":/icons/iconWizzard16x16.png"),tr("Calc. network"),this,SLOT(slotCalcPowerNW()));
        contextMenu.addAction(QPixmap(":/icons/iconWizzard16x16.png"),tr("Check phase balance"),this,SLOT(slotPhaseBalance()));
        contextMenu.addSeparator();
        contextMenu.addAction(QPixmap(":/icons/iconZoomArea16x16.png"),tr("Zoom to fit"),this,SLOT(slotZoomToFitNW()));
        contextMenu.addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDeleteNW()),Qt::CTRL + Qt::Key_Delete);
        contextMenu.exec(p);
    }
}

void CPowerToolWidget::slotLineContextMenu(const QPoint& pos)
{
    qDebug() << "slotLineContextMenu()";
    QListWidgetItem * item = listLines->currentItem();
    
    if(item)
    {
        QPoint p = listLines->mapToGlobal(pos);
        qDebug() << "entered";

        QMenu contextMenu;
        contextMenu.addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit"),this,SLOT(slotEditLine()));

        contextMenu.addSeparator();
        contextMenu.addAction(QPixmap(":/icons/iconChange16x16.png"),tr("Assign to network ..."));

        CPowerLine* l = CPowerDB::self().getPowerLineByKey(item->data(Qt::UserRole).toString());

        QString key;
        QStringList keys = CPowerDB::self().getPowerNWs();

        foreach(key, keys)
        {
            CPowerNW* nw = CPowerDB::self().getPowerNWByKey(key);
            //if (nw->getName() == "unassigned power lines") continue; // But we might want to un-assign lines
            if (l->keyNetwork == nw->getKey()) continue;

            QAction* theAction = new QAction(QPixmap(":/icons/iconPowerNW16x16.png"), "  " + nw->getName(),this);
            contextMenu.addAction(theAction);
            connect(&contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotAssignToPowerNW(QAction*)));
        }

        contextMenu.addSeparator();
        contextMenu.addAction(QPixmap(":/icons/iconZoomArea16x16.png"),tr("Zoom to fit"),this,SLOT(slotZoomToFitLine()));
        contextMenu.addAction(QPixmap(":/icons/iconClear16x16.png"),tr("Delete"),this,SLOT(slotDeleteLine()),Qt::CTRL + Qt::Key_Delete);
        contextMenu.exec(p);
    }
}


void CPowerToolWidget::slotEditNW()
{
    QListWidgetItem * item = listNetworks->currentItem();
    if(item == NULL) return;

    QString key     = item->data(Qt::UserRole).toString();
    CPowerNW* nw   = CPowerDB::self().getPowerNWByKey(key);
    if(nw == NULL) return;

    CDlgEditPowerNW dlg(*nw, this);
    dlg.exec();
}

void CPowerToolWidget::slotEditLine()
{
    QListWidgetItem * item = listLines->currentItem();
    if(item == NULL) return;

    QString key     = item->data(Qt::UserRole).toString();
    CPowerLine* l   = CPowerDB::self().getPowerLineByKey(key);
    if(l == NULL) return;

    CDlgEditPowerLine dlg(*l, this);
    dlg.exec();
}

void CPowerToolWidget::slotAssignToPowerNW(QAction* action)
{
    QListWidgetItem * item = listLines->currentItem();
    if(item == NULL) return;

    CPowerLine* l = CPowerDB::self().getPowerLineByKey(item->data(Qt::UserRole).toString());

    qDebug() << "slotAssignToPowerNW " << l->getName() << " to " << action->text().remove(0,2);

    CPowerNW* nw = CPowerDB::self().getPowerNWByName(action->text().remove(0,2));
    if (nw != NULL)
    {
        l->keyNetwork = nw->getKey();
        CPowerDB::self().setPowerLineData(*l);
        delete nw;
    }
}


void CPowerToolWidget::slotDeleteNW()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listNetworks->selectedItems();
    foreach(item,items)
    {
        keys << item->data(Qt::UserRole).toString();
    }
    //originator = true;
    CPowerDB::self().delPowerNWs(keys);
   // originator = false;
}

void CPowerToolWidget::slotDeleteLine()
{
    QStringList keys;
    QListWidgetItem * item;
    const QList<QListWidgetItem*>& items = listLines->selectedItems();
    foreach(item,items)
    {
        keys << item->data(Qt::UserRole).toString();
    }
    //originator = true;
    CPowerDB::self().delPowerLines(keys);
    //originator = false;
}

const QStringList getOriginatingPowerLines(const QString& wpt_key, const QString &nw_key,
                                           const QString& fromLine)
{
    QStringList result;

    QSqlQuery query = CPowerDB::self().getCustomQuery();
    query.prepare("SELECT key FROM lines WHERE nw_key=:nw_key AND "
                  "((first_key=:wpt_key) OR (second_key=:wpt_key_again))");
    query.bindValue(":nw_key", nw_key);
    query.bindValue(":wpt_key", wpt_key);
    query.bindValue(":wpt_key_again", wpt_key); // specifying :wpt_key twice does NOT work
    QUERY_EXEC(return result);

    qDebug() << "Lines originating from " << wpt_key << " for " << nw_key << ": ";

    while (query.next()) {
        QString key = query.value(0).toString();

        if (key != fromLine)
        {
            result << key;
            qDebug() << query.value(0).toString() << ", ";
        }
    }

    return result;
}

void setPF(const QString& wpt_key, const QString& nw_key, const QString& fromLine, const double pf)
{
    CPowerDB::wpt_eElectric wpt_data = CPowerDB::self().getElectricData(wpt_key);
    if (wpt_data.powerfactor != pf)
    {
        wpt_data.powerfactor = pf;
        CPowerDB::self().setElectricData(wpt_key, wpt_data);
    }

    QStringList lines = getOriginatingPowerLines(wpt_key, nw_key, fromLine);
    QString line_key;

    foreach (line_key, lines)
    {
        CPowerLine * l = CPowerDB::self().getPowerLineByKey(line_key);
        QString next_wpt_key = (l->keyFirst == wpt_key ? l->keySecond : l->keyFirst);
        setPF(next_wpt_key, nw_key, line_key, pf);
    }
}

void CPowerToolWidget::changeSetup(const QString& which)
{
    QListWidgetItem * item;
    QList<QListWidgetItem *> items = listNetworks->selectedItems();
    if (items.isEmpty())
        return;

    foreach(item, items)
    {
        QString key   = item->data(Qt::UserRole).toString();
        CPowerNW * nw = CPowerDB::self().getPowerNWByKey(key);

        if (which == "voltage") {
            nw->voltage = spinVoltage->value();
            CPowerDB::wpt_eElectric ph_electric = CPowerDB::self().getElectricData(nw->ph);
            ph_electric.voltage = nw->voltage;
            CPowerDB::self().setElectricData(nw->ph, ph_electric);
        } else if (which == "ph") {
            QMap<QString,CWpt*>wpts = CWptDB::self().getWpts();
            CWpt * wpt;

            foreach (wpt, wpts)
            {
                if (wpt->getName() == linePH->text())
                {
                    nw->ph = wpt->getKey();
                }
            }
        } else if (which == "watts") {
            nw->watts = spinWatts->value();
        } else if (which == "conductivity") {
            double oldConductivity  = nw->conductivity;
            nw->conductivity = spinConductivity->value();

            // Propagate conductivity to all lines of the network that don't have one yet
            QSqlQuery query = CPowerDB::self().getCustomQuery();
            query.prepare("UPDATE lines SET conductivity=:conductivity "
                          "WHERE (conductivity=:oldConductivity) AND (nw_key=:nw_key)");
            query.bindValue(":nw_key", nw->getKey());
            query.bindValue(":conductivity", nw->conductivity);
            query.bindValue(":oldConductivity", oldConductivity);
            QUERY_EXEC(;)
        } else if (which == "powerfactor") {
            nw->powerfactor = spinPF->value();

            // Propagate powerfactor to all waypoints of the network that don't have one yet
            originator = true;
            setPF(nw->ph, nw->getKey(), "", nw->powerfactor);
            originator = false;
        } else if (which == "ratedvoltage") {
            nw->ratedVoltage = spinRatedVoltage->value();
        } else {
            return; // Or give an error?
        }

        CPowerDB::self().setPowerNWData(*nw);
    }
}

void getPhaseLoads(CPowerDB::wpt_eElectric& e, const QString& fromLine, const double wpc,
                   double& ph1, double& ph2, double& ph3)
{
    CPowerLine* l = CPowerDB::self().getPowerLineByKey(fromLine); // get line leading to this node
    double load = e.voltage * e.voltage / e.resistance * e.powerfactor; // Calculate actual load
    ph1 = ph2 = ph3 = 0.0;

    // Find nominal load on this node
    // We assume that either there is only a summary load, then we use this.
    // Or both the single loads and the summary load is given, then we use the single loads
    double nom1, nom2, nom3;
    nom1 = nom2 = nom3 = 0.0;
    bool singleC = (e.consumers1 + e.consumers2 + e.consumers3 > 0.1);
    bool singleL = (e.loadphase1 + e.loadphase2 + e.loadphase3 > 0.1);

    switch (l->getNumPhases())
    {
    case 3: {
        nom1 = (singleC ? e.consumers1 : e.consumers / 3) * wpc;
        nom2 = (singleC ? e.consumers2 : e.consumers / 3) * wpc;
        nom3 = (singleC ? e.consumers3 : e.consumers / 3) * wpc;
        nom1 += (singleL ? e.loadphase1 : e.load / 3);
        nom2 += (singleL ? e.loadphase2 : e.load / 3);
        nom3 += (singleL ? e.loadphase3 : e.load / 3);
        break;
      }
    case 2: {
        // The first phase must be either 1 or 2, the second must be either 2 or 3;
        if (l->firstPhase() == 1) {
            nom1 = (singleC ? e.consumers1 : e.consumers / 3) * wpc;
            nom1 += (singleL ? e.loadphase1 : e.load / 3);
        } else {
            nom2 = (singleC ? e.consumers2 : e.consumers / 3) * wpc;
            nom2 += (singleL ? e.loadphase2 : e.load / 3);
        }

        if (l->secondPhase() == 2) {
            nom2 = (singleC ? e.consumers2 : e.consumers / 3) * wpc;
            nom2 += (singleL ? e.loadphase2 : e.load / 3);
        } else {
            nom3 = (singleC ? e.consumers3 : e.consumers / 3) * wpc;
            nom3 += (singleL ? e.loadphase3 : e.load / 3);
        }
        break;
      }
    case 1: {
        if (l->firstPhase() == 1) {
            nom1 = (singleC ? e.consumers1 : e.consumers / 3) * wpc;
            nom1 += (singleL? e.loadphase1 : e.load / 3);
        } else if (l->firstPhase() == 2) {
            nom2 = (singleC ? e.consumers2 : e.consumers / 3) * wpc;
            nom2 += (singleL ? e.loadphase2 : e.load / 3);
        } else {
            nom3 = (singleC ? e.consumers3 : e.consumers / 3) * wpc;
            nom3 += (singleL ? e.loadphase3 : e.load / 3);
        }
        break;
      }
    }

    // Divide actual load in same ratio as nominal loads
    double sum = nom1 + nom2 + nom3;
    ph1 = (sum > 0.1) ? load * nom1 / sum : 0.0;
    ph2 = (sum > 0.1) ? load * nom2 / sum : 0.0;
    ph3 = (sum > 0.1) ? load * nom3 / sum : 0.0;
}

const int tabLength = 8; // TODO: Make this OS-independent!

void CPowerToolWidget::getNodeLoads(const QString& wpt_key, const QString& nw_key, const double wpc,
                                    const QString& fromLine, QStringList& loads,
                                    double& ph1, double& ph2, double& ph3)
{
    qDebug() << "getNodeLoads() for " << wpt_key << ", network " << nw_key;
    ph1 = ph2 = ph3 = 0.0;

    QStringList lines = getOriginatingPowerLines(wpt_key, nw_key, fromLine);
    QString line_key;

    foreach (line_key, lines)
    {
        CPowerLine * l = CPowerDB::self().getPowerLineByKey(line_key);
        double newph1, newph2, newph3;

        if (l->keyFirst == wpt_key)
        {
            getNodeLoads(l->keySecond, nw_key, wpc, line_key, loads, newph1, newph2, newph3);
         } else {
            getNodeLoads(l->keyFirst, nw_key, wpc, line_key, loads, newph1, newph2, newph3);
        }

        ph1 += newph1 + l->getPowerOnPhase(1);
        ph2 += newph2 + l->getPowerOnPhase(2);
        ph3 += newph3 + l->getPowerOnPhase(3);
    }

    CWpt* wpt = CWptDB::self().getWptByKey(wpt_key);
    CPowerDB::wpt_eElectric wpt_data = CPowerDB::self().getElectricData(wpt_key);
    double thisph1, thisph2, thisph3;
    getPhaseLoads(wpt_data, fromLine, wpc, thisph1, thisph2, thisph3);
    ph1 += thisph1;
    ph2 += thisph2;
    ph3 += thisph3;

    QString wptname = wpt->getName();
    wptname.truncate(16);

    if (wptname.length() <= tabLength)
        wptname += "\t\t";
    else if (wptname.length() <= 2 * tabLength)
        wptname += "\t";

    loads += wptname +
             tr(" %1W\t").arg(ph1,0,'f',0) +
             tr(" %1W\t").arg(ph2,0,'f',0) +
             tr(" %1W\t").arg(ph3,0,'f',0) +
             tr(" %1W").arg(ph1+ph2+ph3,0,'f',0);

}



#if CALCMETHOD == 1
// wpt_key: The waypoint to calculate the load for
// nw_key: The network to calculate the load for
// wpc: Watts per consumer for this network
// fromLine: The power line we are coming from (exclude from the search for originating lines)
const double calcLoad(const QString& wpt_key, const QString &nw_key, const QString& fromLine,
                      const double wpc)
{
    CPowerDB::wpt_eElectric wpt_data = CPowerDB::self().getElectricData(wpt_key);
    QStringList lines = getOriginatingPowerLines(wpt_key, nw_key, fromLine);
    QString line_key;

    wpt_data.totalload = wpt_data.consumers * wpc + wpt_data.load;

    foreach (line_key, lines)
    {
        CPowerLine * l = CPowerDB::self().getPowerLineByKey(line_key);

        if (l->keyFirst == wpt_key)
        {
            wpt_data.totalload += calcLoad(l->keySecond, nw_key, line_key, wpc);
         } else {
            wpt_data.totalload += calcLoad(l->keyFirst, nw_key, line_key, wpc);
        }
    }

    CPowerDB::self().setElectricData(wpt_key, wpt_data);
    CWpt * wpt = CWptDB::self().getWptByKey(wpt_key);
    if (wpt != NULL)
      qDebug() << "Total load on " << wpt->getName() << "(" << wpt_key << "): " << wpt_data.totalload;
    return wpt_data.totalload;
}
#endif
#if CALCMETHOD == 2
// wpt_key: The waypoint to calculate the resistance for
// nw_key: The network to calculate the resistance for
// wpc: Watts per consumer for this network
// rv: Rated voltage of load
// fromLine: The power line we are coming from (exclude from the search for originating lines)
const double calcResistance(const QString& wpt_key, const QString &nw_key, const QString& fromLine,
                      const double wpc, const double rv)
{
    CPowerDB::wpt_eElectric wpt_data = CPowerDB::self().getElectricData(wpt_key);
    //if (wpt_data.voltage < 0.01) wpt_data.voltage = 240; // Hack, find bug instead!
    //if (wpt_data.powerfactor < 0.01) wpt_data.powerfactor = 0.75; // Hack, find bug instead!
    qDebug() << "Rated voltage: " << rv << ", power factor: " << wpt_data.powerfactor;

    double load = wpt_data.consumers * wpc + wpt_data.load;
    qDebug() << "Load: " << load;

    if (load > 0.01)
        wpt_data.resistance = rv*rv / load * wpt_data.powerfactor;
    else
        wpt_data.resistance = 1E10; // Should be enough ...
    qDebug() << "Resistance: " << wpt_data.resistance;

    // Parallel resistance -> 1/R = 1/R1 + 1/R2 <==> R = R1 * R2 / (R1 + R2)
    // All loads on the originating lines are parallel to the load of this node
    // This assumes that all three-phase loads are balanced across the phases at each node,
    // including the line resistances!
    wpt_data.totalresistance = wpt_data.resistance;
    QStringList lines = getOriginatingPowerLines(wpt_key, nw_key, fromLine);
    QString line_key;

    foreach (line_key, lines)
    {
        CPowerLine * l = CPowerDB::self().getPowerLineByKey(line_key);

        double R1 = wpt_data.totalresistance;

        // Assume single phase loads are balanced across the phases, so a single-phase load
        // is equivalent to three single loads of 1/3 size balanced on the three phases
        // We don't get the advantage of neutral current cancelling out, so line resistance
        // remains as in the single-phase case. Otherwise we could set resistance/6 here
        // Mathematically it doesn't matter whether we add all three single-phase resistances
        // together first (giving 1/3 of each individual resistance) and then parallel them to
        // this node, or whether we parallel each single-phase resistance separately to this node
        double R2 = l->getResistance();
        qDebug() << "Line resistance: " << R2;

        if (l->keyFirst == wpt_key)
        {
            R2 += calcResistance(l->keySecond, nw_key, line_key, wpc, rv); // serial resistance
         } else {
            R2 += calcResistance(l->keyFirst, nw_key, line_key, wpc, rv);
        }
        wpt_data.totalresistance = (R1 * R2)/(R1 + R2);
        qDebug() << "New total resistance: " << wpt_data.totalresistance;
    }

    CPowerDB::self().setElectricData(wpt_key, wpt_data);
    CWpt * wpt = CWptDB::self().getWptByKey(wpt_key);
    if (wpt != NULL)
      qDebug() << "Total resistance on " << wpt->getName() << "(" << wpt_key << "): " << wpt_data.totalresistance;
    return wpt_data.totalresistance;
}
#endif

void calcVoltage(const QString& wpt_key, const QString &nw_key, const QString& fromLine)
{
    // Assumes that the voltage of this node is already set (always true for powerhouse node)
    CPowerDB::wpt_eElectric wpt_data = CPowerDB::self().getElectricData(wpt_key);
    QStringList lines = getOriginatingPowerLines(wpt_key, nw_key, fromLine);
    QString line_key;
    qDebug() << "This node voltage: " << wpt_data.voltage;

    foreach (line_key, lines)
    {
        CPowerLine * l = CPowerDB::self().getPowerLineByKey(line_key);

        // get end point of line
        QString next_wpt = (l->keyFirst == wpt_key ? l->keySecond : l->keyFirst);
        CPowerDB::wpt_eElectric next_wpt_data = CPowerDB::self().getElectricData(next_wpt);

        double oldCurrent = l->getCurrent();
#if CALCMETHOD == 1
        l->setCurrent(next_wpt_data.totalload / (wpt_data.voltage * next_wpt_data.powerfactor));
#elif CALCMETHOD == 2
        l->setCurrent(wpt_data.voltage / (l->getResistance() + next_wpt_data.totalresistance));
        qDebug() << "Set current to " << l->getCurrent();
#endif
        if (fabs(oldCurrent - l->getCurrent()) > 0.01)
          CPowerDB::self().setPowerLineData(*l);

        double oldVoltage = next_wpt_data.voltage;
        next_wpt_data.voltage = wpt_data.voltage - l->getDrop(); // l->drop is calculated in setCurrent()
#if CALCMETHOD == 2
        next_wpt_data.totalload = next_wpt_data.voltage * l->getCurrent() * next_wpt_data.powerfactor;
#endif
        if (fabs(oldVoltage - next_wpt_data.voltage) > 0.01)
          CPowerDB::self().setElectricData(next_wpt, next_wpt_data);

        calcVoltage(next_wpt, nw_key, line_key);
    }
}

void CPowerToolWidget::calcPowerNW(const QString& key) {
    CPowerNW * nw = CPowerDB::self().getPowerNWByKey(key);

    if (nw->getName() == "unassigned power lines") return;

    if (nw->ph != "")
    {
        qDebug() << "RECALCULATING " << nw->getName();
#if CALCMETHOD == 1
        double totalload = calcLoad(nw->ph, key, "", nw->watts);
        qDebug() << "Total load for nw " << nw->getName() << ": " << totalload;
#elif CALCMETHOD == 2
        double totalresistance = calcResistance(nw->ph, key, "", nw->watts, nw->ratedVoltage);
        qDebug() << "Total resistance for nw " << nw->getName() << ": " << totalresistance;
#endif

        // propagate voltage to powerhouse waypoint
        // Shouldn't be necessary really, but due to some bug it is...
        CPowerDB::wpt_eElectric ph_electric = CPowerDB::self().getElectricData(nw->ph);
        ph_electric.voltage = nw->voltage;
        CPowerDB::self().setElectricData(nw->ph, ph_electric);

        calcVoltage(nw->ph, key, "");

#if CALCMETHOD == 2
        CPowerDB::wpt_eElectric ph_data = CPowerDB::self().getElectricData(nw->ph);

        double oldtotalload = ph_data.totalload;
        ph_data.totalload = ph_data.voltage * ph_data.voltage / ph_data.totalresistance * ph_data.powerfactor;

        if (fabs(oldtotalload - ph_data.totalload) > 0.1) {
            CPowerDB::self().setElectricData(nw->ph, ph_data);
        }
#endif
        qDebug() << "Calculated voltage for nw " << nw->getName();
    } else {
        QMessageBox::warning(NULL, "No powerhouse", "Please define a powerhouse waypoint for this network");
    }
}

void CPowerToolWidget::slotCalcPowerNW()
{
    qDebug() << "slotCalcPowerNW()";
    QListWidgetItem * item;
    QList<QListWidgetItem *> items = listNetworks->selectedItems();
    if (items.isEmpty())
        return;

    originator = true;
    foreach(item, items)
    {
        QString key   = item->data(Qt::UserRole).toString();
        calcPowerNW(key);
    }

    originator = false;
}

void CPowerToolWidget::slotCalcPowerNWs()
{
    qDebug() << "slotCalcPowerNWs()";
    QString key;
    QStringList keys = CPowerDB::self().getPowerNWs();

    originator = true;
    foreach(key, keys)
    {
        calcPowerNW(key);
    }

    originator = false;
}

void CPowerToolWidget::slotPhaseBalance()
{
    // TODO: There is a slight difference in the wattage calculation to what is shown in the UI
    // calculation error or propagation of rounding errors?
    // In any case the output fulfills the purpose of showing the phase balance
    qDebug() << "slotPhaseBalance()";
    QListWidgetItem * item;
    QList<QListWidgetItem *> items = listNetworks->selectedItems();
    if (items.isEmpty())
        return;

    QMessageBox msgBox;
    msgBox.setText("Phase balance");

    originator = true;
    foreach(item, items)
    {
        QString nw_key   = item->data(Qt::UserRole).toString();
        CPowerNW* nw = CPowerDB::self().getPowerNWByKey(nw_key);
        QStringList loads;
        double ph1, ph2, ph3;

        getNodeLoads(nw->ph, nw_key, nw->watts, "", loads, ph1, ph2, ph3);
        QString iText = "\t\t Phase 1\t Phase 2\t Phase 3\t Total\n";
        iText += loads.join("\n");

        msgBox.setInformativeText(iText);
        //msgBox.setDetailedText("a b c d");
        msgBox.setStandardButtons( QMessageBox::Ok);
        msgBox.exec();

    }

    originator = false;
}

void CPowerToolWidget::slotNWSelectionChanged()
{
    qDebug() << "slotNWSelectionChanged";
    if(originator)
    {
        return;
    }

    if (listNetworks->hasFocus() && listNetworks->selectedItems().isEmpty())
    {
        qDebug() << "Removing selection";
        CPowerDB::self().highlightPowerNW("");
        fillListLines("");
        fillDefaults("");
    }
}

void CPowerToolWidget::slotLineSelectionChanged()
{
    if(originator)
    {
        return;
    }

    if (listLines->hasFocus() && listLines->selectedItems().isEmpty())
    {
        CPowerDB::self().highlightPowerLine("");
        fillListLines("");
    }
}

void CPowerToolWidget::slotZoomToFitNW()
{
    QRectF r;

    const QList<QListWidgetItem*>& items = listNetworks->selectedItems();
    QList<QListWidgetItem*>::const_iterator item = items.begin();

    r = CPowerDB::self().getPowerNWByKey((*item)->data(Qt::UserRole).toString())->getBoundingRectF();

    while(item != items.end())
    {
        r |= CPowerDB::self().getPowerNWByKey((*item)->data(Qt::UserRole).toString())->getBoundingRectF();
        ++item;
    }

    if (!r.isNull ())
    {
        CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
    }
}

void CPowerToolWidget::slotZoomToFitLine()
{
    QRectF r;

    const QList<QListWidgetItem*>& items = listLines->selectedItems();
    QList<QListWidgetItem*>::const_iterator item = items.begin();

    /*r = CPowerDB::self().getBoundingRectF((*item)->data(Qt::UserRole).toString());

    while(item != items.end())
    {
        r |= CPowerDB::self().getBoundingRectF((*item)->data(Qt::UserRole).toString());
        ++item;
    }

    if (!r.isNull ())
    {
        CMapDB::self().getMap().zoom(r.left() * DEG_TO_RAD, r.top() * DEG_TO_RAD, r.right() * DEG_TO_RAD, r.bottom() * DEG_TO_RAD);
    }*/
}

bool CPowerToolWidget::eventFilter(QObject *obj, QEvent *event)
{

    if(event->type() == QEvent::Leave)
    {
        listNetworks->clearFocus();
        listLines->clearFocus();
    }

    return QObject::eventFilter(obj, event);
}
