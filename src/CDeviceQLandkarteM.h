/**********************************************************************************************
    Copyright (C) 2009 Oliver Eichler oliver.eichler@gmx.de

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
#ifndef CDEVICEQLANDKARTEM_H
#define CDEVICEQLANDKARTEM_H

#include "IDevice.h"
#include <QtNetwork>

class CDeviceQLandkarteM : public IDevice
{
    Q_OBJECT;
    public:
        CDeviceQLandkarteM(const QString& ipaddr, quint16 port, QObject * parent);
        virtual ~CDeviceQLandkarteM();

        void uploadWpts(const QList<CWpt*>& wpts);
        void downloadWpts(QList<CWpt*>& wpts);

        void uploadTracks(const QList<CTrack*>& trks);
        void downloadTracks(QList<CTrack*>& trks);

        void uploadRoutes(const QList<CRoute*>& rtes);
        void downloadRoutes(QList<CRoute*>& rtes);


        void uploadMap(const QList<IMapSelection*>& mss);

    private:
        enum packet_e
        {
            eNone           = 0
            , eError        = 1     ///< error occured
            , eAck          = 2     ///<
            , eC2HAlive     = 3
            , eH2CAlive     = 4
            , eC2HWpt       = 5     ///< send waypoint data from client to host
            , eH2CWptQuery  = 6     ///< request waypoint keys from host
            , eH2CWpt       = 7     ///< request waypoint data from host
            , eH2CTrkQuery  = 8     ///< request track keys from host
            , eH2CTrk       = 9     ///< request track data from host
            , eC2HTrk       = 10    ///< send track data from from client to host
        };
        bool startDeviceDetection();
        bool acquire(const QString& operation, int max);
        void send(const packet_e type, const QByteArray& data);
        bool recv(packet_e& type, QByteArray& data);
        bool exchange(packet_e& type,QByteArray& data);
        void release();

        QUdpSocket udpSocket;
        QTcpSocket tcpSocket;
        QString ipaddr;
        quint16 port;
        const int timeout;

    private slots:
        void detectedDevice();

};

#endif //CDEVICEQLANDKARTEM_H

