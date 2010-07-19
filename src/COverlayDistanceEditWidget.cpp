/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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

#include "COverlayDistanceEditWidget.h"
#include "COverlayDistance.h"
#include "IUnit.h"
#include "GeoMath.h"
#include "CMegaMenu.h"
#include "CMainWindow.h"
#include "CCanvas.h"

#include <QtGui>

COverlayDistanceEditWidget::COverlayDistanceEditWidget(QWidget * parent, COverlayDistance * ovl)
: QWidget(parent)
, ovl(ovl)
{
    ovl->isEdit = true;

    setupUi(this);

    lineName->setText(ovl->name);
    textComment->setText(ovl->comment);

    labelUnit->setText(IUnit::self().speedunit);
    lineSpeed->setText(QString::number(ovl->speed * IUnit::self().speedfactor));

    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(slotApply()));
    connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));

    connect(ovl, SIGNAL(sigChanged()), this, SLOT(slotChanged()));
    connect(ovl, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(ovl, SIGNAL(sigSelectionChanged()), this, SLOT(slotSelectionChanged()));
    connect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotContextMenu(const QPoint&)));

    contextMenu = new QMenu(this);
    contextMenu->addAction(QIcon(":/icons/editcopy"), tr("Copy"), this, SLOT(slotCopy()));
    contextMenu->addAction(QIcon(":/icons/iconDelete16x16"), tr("Delete"), this, SLOT(slotDelete()));

    slotChanged();
    slotSelectionChanged();

    CMegaMenu::self().switchByKeyWord("OverlayDistance");
}

COverlayDistanceEditWidget::~COverlayDistanceEditWidget()
{
    ovl->isEdit = false;
    CMegaMenu::self().switchByKeyWord("Overlay");    
}

bool COverlayDistanceEditWidget::isAboutToClose()
{
    return !ovl->isEdit;
}

void COverlayDistanceEditWidget::slotApply()
{
    ovl->name = lineName->text();
    ovl->comment = textComment->toPlainText();
    ovl->speed = lineSpeed->text().toDouble() / IUnit::self().speedfactor;

    emit ovl->sigChanged();
}

void COverlayDistanceEditWidget::slotChanged()
{
    QString pos;

    int i;
    const int size = ovl->points.size();

    treeWidget->clear();

    for(i = 0; i < size; i++)
    {
        COverlayDistance::pt_t pt = ovl->points[i];
        GPS_Math_Deg_To_Str(pt.u * RAD_TO_DEG, pt.v * RAD_TO_DEG, pos);

        QTreeWidgetItem * item = new QTreeWidgetItem(treeWidget);
        item->setText(eNo, QString::number(pt.idx));
        item->setData(eNo,Qt::UserRole, pt.idx);

        item->setText(ePos, pos);

    }

    treeWidget->header()->setResizeMode(eNo,QHeaderView::ResizeToContents);
}

void COverlayDistanceEditWidget::slotSelectionChanged()
{
    QTreeWidgetItem * item = 0;
    int idx;
    const int size = ovl->points.size();
    for(idx = 0; idx < size; idx++)
    {
        if(ovl->selectedPoints.contains(idx))
        {
            if(!item) item = treeWidget->topLevelItem(idx);
            treeWidget->topLevelItem(idx)->setSelected(true);
        }
        else
        {
            treeWidget->topLevelItem(idx)->setSelected(false);
        }
    }

    if(item)
    {
        treeWidget->scrollToItem(item);
    }
}

void COverlayDistanceEditWidget::slotItemSelectionChanged()
{
    ovl->selectedPoints.clear();
    const QList<QTreeWidgetItem *>& items = treeWidget->selectedItems();
    QTreeWidgetItem * item;

    foreach(item, items)
    {
        ovl->selectedPoints << item->data(eNo, Qt::UserRole).toInt();
    }

    theMainWindow->getCanvas()->update();

}

void COverlayDistanceEditWidget::slotContextMenu(const QPoint& pos)
{
    int cnt = treeWidget->selectedItems().count();
    if(cnt > 0)
    {

//        actSplit->setEnabled(cnt == 1);
        QPoint p = treeWidget->mapToGlobal(pos);
        contextMenu->exec(p);
    }
}

void COverlayDistanceEditWidget::slotCopy()
{

}

void COverlayDistanceEditWidget::slotDelete()
{
    QList<int> idx;
    QTreeWidgetItem * item;
    const QList<QTreeWidgetItem*>& items = treeWidget->selectedItems();

    foreach(item, items)
    {
        idx << item->data(eNo, Qt::UserRole).toInt();
    }

    ovl->delPointsByIdx(idx);


}