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
#ifndef CDLGEDITWPT_H
#define CDLGEDITWPT_H

#include <QDialog>

#include "ui_IDlgEditWpt.h"

class CWpt;

/// dialog to edit waypoint properties
class CDlgEditWpt : public QDialog, private Ui::IDlgEditWpt
{
    Q_OBJECT;
    public:
        CDlgEditWpt(CWpt &wpt, QWidget * parent);
        virtual ~CDlgEditWpt();

    public slots:
        int exec();
        void accept();

    private slots:
        void slotAddImage();
        void slotDelImage();
        void slotNextImage();
        void slotPrevImage();
        void slotSelectIcon();
        void slotOpenLink(const QString& link);
        void slotOpenLink(const QUrl& url);
        void slotEditLink();
        void slotSaveBarcode();
        void slotUpdateBarcode();

    private:
        void showImage(int idx);
        CWpt &wpt;
        qint32 idxImg;
        QString link;
        double oldLon;
        double oldLat;
};
#endif                           //CDLGEDITWPT_H
