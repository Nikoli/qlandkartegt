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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

**********************************************************************************************/

#ifndef CDEVICEGARMIN_H
#define CDEVICEGARMIN_H

#include "IDevice.h"

#include <QTime>

class QTimer;

namespace Garmin
{
    class IDevice;
}

class CDeviceGarmin : public IDevice
{
    Q_OBJECT;
    public:
        CDeviceGarmin(const QString& devkey, const QString& port, QObject * parent);
        virtual ~CDeviceGarmin();

        const QString getDevKey(){return QString("Garmin");}

        void uploadWpts(const QList<CWpt*>& wpts);
        void downloadWpts(QList<CWpt*>& wpts);

        void downloadTracks(QList<CTrack*>& trks);

        void setLiveLog(bool on);
        bool liveLog();

    private slots:
        void slotTimeout();

    private:
        Garmin::IDevice * getDevice();

        QString port;

        friend void GUICallback(int progress, int * ok, int * cancel, const char * title, const char * msg, void * self);
        struct dlgdata_t
        {
            dlgdata_t() : dlgProgress(0), canceled(false)
                {}

            QProgressDialog * dlgProgress;
            QTime timeProgress;
            bool canceled;
        };

        dlgdata_t dlgData;

        QTimer * timer;

};

#endif //CDEVICEGARMIN_H

