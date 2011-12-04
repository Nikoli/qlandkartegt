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

#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>
class QSplitter;
class QLabel;
class QTabWidget;
class CCanvas;
class CMegaMenu;
class CMapDB;
class CWptDB;
class CTrackDB;
class CSearchDB;
class CDiaryDB;
class CRouteDB;
class CResources;
class CTabWidget;
class CLiveLogDB;
class COverlayDB;
class QComboBox;
class CActions;
class CMenus;
class QSocketNotifier;
class QAction;
class CGeoDB;
#ifdef HAS_DBUS
class CDBus;
#endif
class CGridDB;

class CMainWindow : public QMainWindow
{
    Q_OBJECT;
    public:
        CMainWindow();
        virtual ~CMainWindow();

        void setPositionInfo(const QString& info);

        CCanvas * getCanvas(){return canvas;}

        CTabWidget * getCanvasTab(){return canvasTab;}

        void setTempWidget(QWidget * w, const QString& label);

        void clear();
        void clearAll();

        const QString& getCurrentFilename(){return wksFile;}

        CMenus *getActionGroupProvider() { return actionGroupProvider;}

        void exportToOcm();
        void loadData(const QString& filename, const QString& filter);

        bool didCrash(){return crashed;}

    protected:
        void closeEvent(QCloseEvent * e);
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);

    private slots:
        void slotReloadArgs();
        void switchState();
        void slotLoadMapSet();
        void slotCopyright();
        void slotToolBoxChanged(int idx);
        void slotConfig();
        void slotLoadData();
        void slotAddData();
        void slotSaveData();
        void slotExportData();
        void slotSaveImage();
        void slotPrint();
        void slotModified();
        void slotDataChanged();
        void slotOpenLink(const QString& link);
        void slotCurrentDeviceChanged(int);
        void slotDeviceChanged();
        void slotScreenshot();
        void slotLoadRecent();
        void slotItemDestroyed(QObject *);
        void slotTabCloseRequest(int i);

        void slotFAQ();
        void slotHelp();
        void slotSupport();

        void slotToggleToolView();

    private:
        friend class CDBus;
        CMenus *actionGroupProvider;
        void setupMenuBar();
        void addRecent(const QString& filename);

        void setTitleBar();
        bool maybeSave();
        void saveData(QString& filename, const QString& filter, bool exportFlag = false);
        bool convertData(const QString& inFormat, const QString& inFile, const QString& outFormat, const QString& outFile);
        bool isGPSBabel();
        QString getGeoDataFormats();

        QMenu *setupMenu;
        QMenu *groupProvidedMenu;
        /// horizontal main splitter holding the canvas and the tool view
        QSplitter * mainSplitter;
        /// the vertical splitter holding the tool views
        QSplitter * leftSplitter;
        /// the vertical splitter holding the canvas and track info view
        QSplitter * rightSplitter;

        /// left hand context sensitive menu
        CMegaMenu * megaMenu;

        /// left hand tool box
        QTabWidget  * tabbar;

        CTabWidget * canvasTab;

        /// the map canvas
        CCanvas * canvas;

        /// coordinate label
        QLabel * statusCoord;

        /// root path of geo data
        QString pathData;

        CResources * resources;

        /// the waypoint data base
        CWptDB * wptdb;
        /// the search database
        CSearchDB * searchdb;
        /// the map data base
        CMapDB * mapdb;
        /// the track data base
        CTrackDB * trackdb;
        /// diary database
        CDiaryDB * diarydb;

        CLiveLogDB * livelogdb;

        COverlayDB * overlaydb;

        CRouteDB * routedb;

        CGeoDB * geodb;

        CGridDB * griddb;

        /// the current loaded geo data (workspace) file
        QString wksFile;
        /// true if any data has been changed
        bool modified;

        /// project summary
        QLabel * summary;
        /// combobox to switch device
        QComboBox * comboDevice;

        QSocketNotifier * snRead;

        QMenu * menuMostRecent;

        QStringList mostRecent;

        QTabWidget * tmpTabWidget;
#ifdef HAS_DBUS
        CDBus * dbus;
#endif
        bool locked;
        bool crashed;

};

extern CMainWindow * theMainWindow;
#endif                           //CMAINWINDOW_H
