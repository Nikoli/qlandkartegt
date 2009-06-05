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

#include "CMegaMenu.h"
#include "CCanvas.h"
#include "CSearchDB.h"
#include "CWptDB.h"
#include "CMapDB.h"
#include "CTrackDB.h"
#include "CDiaryDB.h"
#include "CLiveLogDB.h"
#include "COverlayDB.h"
#include "CRouteDB.h"
#include "CTrackToolWidget.h"
#include "CCreateMapGeoTiff.h"
#include "CMainWindow.h"
#include "CResources.h"
#include "IDevice.h"
#include "CDlgCreateWorldBasemap.h"
#include "CActionGroupProvider.h"
#include "CActions.h"
#ifdef PLOT_3D
#include "CMap3DWidget.h"
#endif

#include <QtGui>
/// Enhanced QLabel used by CMegaMenu
class CLabel: public QLabel
{
public:
    CLabel(QWidget * parent) :
        QLabel(parent)
    {
        setMouseTracking(true);
    }
    ~CLabel()
    {
    }

    void mouseMoveEvent(QMouseEvent * e)
    {
        setBackgroundRole(QPalette::Highlight);
        setForegroundRole(QPalette::HighlightedText);
    }

    void leaveEvent(QEvent *)
    {
        setBackgroundRole(QPalette::Window);
        setForegroundRole(QPalette::WindowText);
    }

    void paintEvent(QPaintEvent * e)
    {

        QPainter p;
        p.begin(this);
        if (backgroundRole() == QPalette::Highlight)
        {
            p.setBrush(QBrush(QColor(66, 121, 206, 150)));
        } else
        {
            p.setBrush(QBrush(QColor(66, 121, 206, 0)));
        }
        p.setPen(Qt::NoPen);
        p.drawRect(rect());
        p.end();

        QLabel::paintEvent(e);
    }
};

CMegaMenu * CMegaMenu::m_self = 0;

/// Left hand side multi-function menu
CMegaMenu::CMegaMenu(CCanvas * canvas) :
    QLabel(canvas), canvas(canvas), current(0), fsMain(11, func_key_state_t()),
            fsMap(11, func_key_state_t()), fsMap3D(11, func_key_state_t()),
            fsWpt(11, func_key_state_t()), fsTrack(11, func_key_state_t()),
            fsLiveLog(11, func_key_state_t()),
            fsOverlay(11, func_key_state_t()),
            fsMainMore(11, func_key_state_t()), fsRoute(11, func_key_state_t())
{

    actionGroup = theMainWindow->getActionGroupProvider();
    actions = actionGroup->getActions();
    menu=0;
    scrollArea=0;

    actionGroup->addAction(CActionGroupProvider::MainMenu, "aSwitchToMap");
    actionGroup->addAction(CActionGroupProvider::MainMenu, "aSwitchToWpt");
    actionGroup->addAction(CActionGroupProvider::MainMenu, "aSwitchToTrack");
    actionGroup->addAction(CActionGroupProvider::MainMenu, "aSwitchToRoute");
    actionGroup->addAction(CActionGroupProvider::MainMenu, "aSwitchToLiveLog");
    actionGroup->addAction(CActionGroupProvider::MainMenu, "aSwitchToOverlay");
    actionGroup->addAction(CActionGroupProvider::MainMenu, "aSwitchToMainMore");
    actionGroup->addAction(CActionGroupProvider::MainMenu, "aClearAll");
    actionGroup->addAction(CActionGroupProvider::MainMenu, "aUploadAll");
    actionGroup->addAction(CActionGroupProvider::MainMenu, "aDownloadAll");

    actionGroup->addAction(CActionGroupProvider::MapMenu, "aSwitchToMain");
    actionGroup->addAction(CActionGroupProvider::MapMenu, "aSwitchToMain");
    actionGroup->addAction(CActionGroupProvider::MapMenu, "aMoveArea");
    actionGroup->addAction(CActionGroupProvider::MapMenu, "aZoomArea");
    actionGroup->addAction(CActionGroupProvider::MapMenu, "aCenterMap");
    //    fsMap[4] = func_key_state_t(0,tr("-"),0,tr(""));
    actionGroup->addAction(CActionGroupProvider::MapMenu, "aSelectArea");
    actionGroup->addAction(CActionGroupProvider::MapMenu, "aEditMap");
    actionGroup->addAction(CActionGroupProvider::MapMenu, "aSearchMap");
    //#ifdef PLOT_3D
    //    fsMap[8] = func_key_state_t(":/icons/icon3D16x16.png",tr("3D Map..."), &CMegaMenu::funcSwitchToMap3D, tr("Show 3D map"));
    //#endif
    actionGroup->addAction(CActionGroupProvider::MapMenu, "aUploadMap");
    //    fsMap[10] = func_key_state_t(0,tr("-"),0,tr(""));

#ifdef PLOT_3D
    actionGroup->addAction(CActionGroupProvider::Map3DMenu, "aCloseMap3D");
    actionGroup->addAction(CActionGroupProvider::Map3DMenu, "aMap3DMode");
    actionGroup->addAction(CActionGroupProvider::Map3DMenu, "aMap3DZoomPlus");
    actionGroup->addAction(CActionGroupProvider::Map3DMenu, "aMap3DZoomMinus");
    actionGroup->addAction(CActionGroupProvider::Map3DMenu, "Map3DLighting");
    //    fsMap3D[5] = func_key_state_t(0,tr("-"),0,tr(""));
    //    fsMap3D[6] = func_key_state_t(0,tr("-"),0,tr(""));
    //    fsMap3D[7] = func_key_state_t(0,tr("-"),0,tr(""));
    //    fsMap3D[8] = func_key_state_t(0,tr("-"),0,tr(""));
    //    fsMap3D[9] = func_key_state_t(0,tr("-"),0,tr(""));
    //    fsMap3D[10] = func_key_state_t(0,tr("-"),0,tr(""));
#endif

    actionGroup->addAction(CActionGroupProvider::WptMenu, "aSwitchToMain");
    actionGroup->addAction(CActionGroupProvider::WptMenu, "aMoveArea");
    actionGroup->addAction(CActionGroupProvider::WptMenu, "aZoomArea");
    actionGroup->addAction(CActionGroupProvider::WptMenu, "aCenterMap");
    //    fsWpt[4] = func_key_state_t(0,tr("-"),0,tr(""));
    actionGroup->addAction(CActionGroupProvider::WptMenu, "aNewWpt");
    actionGroup->addAction(CActionGroupProvider::WptMenu, "aEditWpt");
    actionGroup->addAction(CActionGroupProvider::WptMenu, "aMoveWpt");
#ifdef HAS_EXIF
    actionGroup->addAction(CActionGroupProvider::WptMenu, "aImageWpt");
#else
    fsWpt[8] = func_key_state_t(0, tr("-"), 0, tr(""));
#endif
    actionGroup->addAction(CActionGroupProvider::WptMenu, "aUploadWpt");
    actionGroup->addAction(CActionGroupProvider::WptMenu, "aDownloadWpt");

    actionGroup->addAction(CActionGroupProvider::TrackMenu, "aSwitchToMain");
    actionGroup->addAction(CActionGroupProvider::TrackMenu, "aMoveArea");
    actionGroup->addAction(CActionGroupProvider::TrackMenu, "aZoomArea");
    actionGroup->addAction(CActionGroupProvider::TrackMenu, "aCenterMap");
    //    fsTrack[4] = func_key_state_t(0,tr("-"),0,tr(""));
    actionGroup->addAction(CActionGroupProvider::TrackMenu, "aCombineTrack");
    actionGroup->addAction(CActionGroupProvider::TrackMenu, "aEditTrack");
    actionGroup->addAction(CActionGroupProvider::TrackMenu, "aCutTrack");
    actionGroup->addAction(CActionGroupProvider::TrackMenu, "aSelTrack");
    actionGroup->addAction(CActionGroupProvider::TrackMenu, "aUploadTrack");
    actionGroup->addAction(CActionGroupProvider::TrackMenu, "aDownloadTrack");

    actionGroup->addAction(CActionGroupProvider::LiveLogMenu, "aSwitchToMain");
    actionGroup->addAction(CActionGroupProvider::LiveLogMenu, "aMoveArea");
    actionGroup->addAction(CActionGroupProvider::LiveLogMenu, "aZoomArea");
    actionGroup->addAction(CActionGroupProvider::LiveLogMenu, "aCenterMap");
    //    fsLiveLog[4] = func_key_state_t(0,tr("-"),0,tr(""));
    actionGroup->addAction(CActionGroupProvider::LiveLogMenu, "aLiveLog");
    actionGroup->addAction(CActionGroupProvider::LiveLogMenu, "aLockMap");
    actionGroup->addAction(CActionGroupProvider::LiveLogMenu, "aAddWpt");
    //    fsLiveLog[8] = func_key_state_t(0,tr("-"),0,tr(""));
    //    fsLiveLog[9] = func_key_state_t(0,tr("-"),0,tr(""));
    //    fsLiveLog[10] = func_key_state_t(0,tr("-"),0,tr(""));

    actionGroup->addAction(CActionGroupProvider::OverlayMenu, "aSwitchToMain");
    actionGroup->addAction(CActionGroupProvider::OverlayMenu, "aMoveArea");
    actionGroup->addAction(CActionGroupProvider::OverlayMenu, "aZoomArea");
    actionGroup->addAction(CActionGroupProvider::OverlayMenu, "aCenterMap");
    //    fsOverlay[4] = func_key_state_t(0,tr("-"),0,tr(""));
    actionGroup->addAction(CActionGroupProvider::OverlayMenu, "aText");
    actionGroup->addAction(CActionGroupProvider::OverlayMenu, "aTextBox");
    actionGroup->addAction(CActionGroupProvider::OverlayMenu, "aDistance");
    //    fsOverlay[8] = func_key_state_t(0,tr("-"),0,tr(""));
    //    fsOverlay[9] = func_key_state_t(0,tr("-"),0,tr(""));
    //    fsOverlay[10] = func_key_state_t(0,tr("-"),0,tr(""));

    actionGroup->addAction(CActionGroupProvider::MainMoreMenu, "aSwitchToMain");
    actionGroup->addAction(CActionGroupProvider::MainMoreMenu, "aMoveArea");
    actionGroup->addAction(CActionGroupProvider::MainMoreMenu, "aZoomArea");
    actionGroup->addAction(CActionGroupProvider::MainMoreMenu, "aCenterMap");
    //    fsMainMore[4] = func_key_state_t(0,tr("-"),0,tr(""));
    actionGroup->addAction(CActionGroupProvider::MainMoreMenu, "aDiary");
    actionGroup->addAction(CActionGroupProvider::MainMoreMenu, "aColorPicker");
    actionGroup->addAction(CActionGroupProvider::MainMoreMenu, "aWorldBasemap");
    //    fsMainMore[8] = func_key_state_t(0,tr("-"),0,tr(""));
    //    fsMainMore[9] = func_key_state_t(0,tr("-"),0,tr(""));
    //    fsMainMore[10] = func_key_state_t(0,tr("-"),0,tr(""));

    actionGroup->addAction(CActionGroupProvider::RouteMenu, "aSwitchToMain");
    actionGroup->addAction(CActionGroupProvider::RouteMenu, "aMoveArea");
    actionGroup->addAction(CActionGroupProvider::RouteMenu, "aZoomArea");
    actionGroup->addAction(CActionGroupProvider::RouteMenu, "aCenterMap");
    actionGroup->addAction(CActionGroupProvider::RouteMenu, "aUploadRoute");
    actionGroup->addAction(CActionGroupProvider::RouteMenu, "aDownloadRoute");

    m_self = this;
    setScaledContents(true);

    int i;

    mainLayout = new QVBoxLayout(this);

    QHBoxLayout * titleLayout = new QHBoxLayout();
    mainLayout->addLayout(titleLayout);

    menuTitle = new QLabel(this);
    menuTitle->setAlignment(Qt::AlignCenter);
    titleLayout->addWidget(menuTitle);
        //
    layout = new QGridLayout();

    mainLayout->addLayout(layout);


    //    keyEsc   = new QLabel("Esc",this);
    //    layout->addWidget(keyEsc,0,0);
    //    keyF1   = new QLabel("F1",this);
    //    layout->addWidget(keyF1,1,0);
    //    keyF2 = new QLabel("F2",this);
    //    layout->addWidget(keyF2,2,0);
    //    keyF3 = new QLabel("F3",this);
    //    layout->addWidget(keyF3,3,0);
    //    keyF4 = new QLabel("F4",this);
    //    layout->addWidget(keyF4,4,0);
    //    keyF5 = new QLabel("F5",this);
    //    layout->addWidget(keyF5,5,0);
    //    keyF6 = new QLabel("F6",this);
    //    layout->addWidget(keyF6,6,0);
    //    keyF7 = new QLabel("F7",this);
    //    layout->addWidget(keyF7,7,0);
    //    keyF8 = new QLabel("F8",this);
    //    layout->addWidget(keyF8,8,0);
    //    keyF9 = new QLabel("F9",this);
    //    layout->addWidget(keyF9,9,0);
    //    keyF10 = new QLabel("F10",this);
    //    layout->addWidget(keyF10,10,0);
    //
    //    for(i=0; i<11; ++i) {
    //        icons[i] = new QLabel(this);
    //        layout->addWidget(icons[i],i,1);
    //    }
    //
    //    for(i=0; i<11; ++i) {
    //        names[i] = new CLabel(this);
    //        names[i]->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    //        layout->addWidget(names[i],i,2);
    //    }

}

CMegaMenu::~CMegaMenu()
{

}

void CMegaMenu::switchState(CActionGroupProvider::ActionGroupName groupName)
{
    actionGroup->switchToActionGroup(groupName);

    if (menu)
        delete menu;

    if (scrollArea)
        delete scrollArea;

    menu = new QMenu(this);
    menu->setWindowFlags(Qt::Widget);

    foreach(QAction *a, *actionGroup->getActiveActions())
        menu->addAction(a);

    scrollArea = new QScrollArea();
    //scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(menu);

    layout->addWidget(scrollArea);
    menu->show();

}

void CMegaMenu::switchByKeyWord(const QString& key)
{
    if (!isEnabled())
        return;

    if (key == "Main")
    {
        actions->funcSwitchToMain();
    } else if (key == "Waypoints" && current != &fsWpt)
    {
        actions->funcSwitchToWpt();
        actions->funcMoveArea();
    } else if (key == "Search" && current != &fsMain)
    {
        actions->funcSwitchToMain();
        actions->funcMoveArea();
    } else if (key == "Maps" && current != &fsMap)
    {
        actions->funcSwitchToMap();
        actions->funcMoveArea();
    } else if (key == "Tracks" && current != &fsTrack)
    {
        actions->funcSwitchToTrack();
        actions->funcMoveArea();
    } else if (key == "LiveLog" && current != &fsLiveLog)
    {
        actions->funcSwitchToLiveLog();
        actions->funcMoveArea();
    } else if (key == "Overlay" && current != &fsOverlay)
    {
        actions->funcSwitchToOverlay();
        actions->funcMoveArea();
    } else if (key == "Routes" && current != &fsRoute)
    {
        actions->funcSwitchToRoute();
        actions->funcMoveArea();
    }

}

void CMegaMenu::keyPressEvent(QKeyEvent * e)
{
    qDebug() << e->key();
    //    if(!isEnabled()) return;
    //
    //    if((e->key() >= Qt::Key_F1) && (e->key() < Qt::Key_F11)) {
    //        unsigned i = e->key() - Qt::Key_F1 + 1;
    //        if((*current)[i].func) {
    //            (this->*(*current)[i].func)();
    //
    //        }
    //        return e->accept();
    //    }
    //    else if(e->key() == Qt::Key_Escape) {
    //        if((*current)[0].func) {
    //            (this->*(*current)[0].func)();
    //        }
    //        return e->accept();
    //    }
    //    else if(e->key() == Qt::Key_Plus) {
    //        canvas->zoom(true, canvas->geometry().center());
    //        return e->accept();
    //    }
    //    else if(e->key() == Qt::Key_Minus) {
    //        canvas->zoom(false, canvas->geometry().center());
    //        return e->accept();
    //    }
    //    else if(e->key() == Qt::Key_Left) {
    //        canvas->move(CCanvas::eMoveLeft);
    //        return e->accept();
    //    }
    //    else if(e->key() == Qt::Key_Right) {
    //        canvas->move(CCanvas::eMoveRight);
    //        return e->accept();
    //    }
    //    else if(e->key() == Qt::Key_Up) {
    //        canvas->move(CCanvas::eMoveUp);
    //        return e->accept();
    //    }
    //    else if(e->key() == Qt::Key_Down) {
    //        canvas->move(CCanvas::eMoveDown);
    //        return e->accept();
    //    }
    //    else if (e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier)
    //    {
    //      qDebug() << "ctrl + c pressed" ;
    //      CTrackDB::self().copyToClipboard();
    //      return e->accept();
    //    }
    //    else if (e->key() == Qt::Key_V && e->modifiers() == Qt::ControlModifier)
    //    {
    //      qDebug() << "ctrl + v pressed" ;
    //      CTrackDB::self().pasteFromClipboard();
    //      return e->accept();
    //    }

}

void CMegaMenu::mousePressEvent(QMouseEvent * e)
{
    //    unsigned i;
    //
    //    if(e->button() == Qt::RightButton)
    //      qDebug() << "Right Button in " << Q_FUNC_INFO;
    //    if(e->button() != Qt::LeftButton) return;
    //    for(i=0; i<11; ++i) {
    //        if(names[i]->geometry().contains(e->pos())) {
    //            if((*current)[i].func) {
    //                (this->*(*current)[i].func)();
    //            }
    //            return;
    //        }
    //    }
}

