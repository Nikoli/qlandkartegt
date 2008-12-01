/**********************************************************************************************
    Copyright (C) 2008 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CMAPQMAPEXPORT_H
#define CMAPQMAPEXPORT_H

#include <QDialog>
#include <QProcess>
#include "ui_IMapQMAPExport.h"

class CMapSelectionRaster;

class CMapQMAPExport : public QDialog, private Ui::IMapQMAPExport
{
    Q_OBJECT;
    public:
        CMapQMAPExport(const CMapSelectionRaster& mapsel, QWidget * parent);
        virtual ~CMapQMAPExport();

    private slots:
        void slotStart();
        void slotOutputPath();
        void slotStderr();
        void slotStdout();
        void slotFinished( int exitCode, QProcess::ExitStatus status);

    private:
        const CMapSelectionRaster& mapsel;

        QProcess cmd;

        struct job_t
        {
            QString srcFilename;
            QString tarFilename;
            quint32 xoff;
            quint32 yoff;
            quint32 width;
            quint32 height;
            int idx;
        };

        QList<job_t> jobs;
};
#endif                           //CMAPQMAPEXPORT_H
