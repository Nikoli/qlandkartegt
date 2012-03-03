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
#include "CResources.h"
#include "CCanvas.h"
#include "CMapNoMap.h"
#include "CMapQMAP.h"
#include "CMainWindow.h"
#include "CCreateMapGeoTiff.h"
#include "CMouseMoveMap.h"
#include "CMouseZoomMap.h"
#include "CMouseSelMap.h"
#include "CMouseAddWpt.h"
#include "CMouseMoveWpt.h"
#include "CMouseEditWpt.h"
#ifdef HAS_POWERDB
#include "CMouseEditPowerNW.h"
#include "CMouseEditPower.h"
#include "CMouseAddPowerNW.h"
#include "CMouseAddPowerLine.h"
#endif
#include "CMouseRefPoint.h"
#include "CMouseCutTrack.h"
#include "CMouseSelTrack.h"
#include "CMouseAddText.h"
#include "CMouseAddTextBox.h"
#include "CMouseAddDistance.h"
#include "CMouseOverlay.h"
#include "CMouseColorPicker.h"
#include "CWpt.h"
#include "CTrack.h"
#include "CSearchDB.h"
#include "CWptDB.h"
#include "CMapDB.h"
#include "CTrackDB.h"
#include "CTrackToolWidget.h"
#include "CRouteDB.h"
#include "CLiveLogDB.h"
#include "COverlayDB.h"
#include "CMegaMenu.h"
#include "GeoMath.h"
#include "WptIcons.h"
#include "Platform.h"
#include "IUnit.h"
#include "CMenus.h"
#include "CUndoStackView.h"
#include "CCanvasUndoCommandZoom.h"
#include "CMapUndoCommandMove.h"
#include "CPlot.h"
#ifdef HAS_POWERDB
#include "CPowerDB.h"
#endif

#include <QtGui>

QPen CCanvas::penBorderBlue(QColor(10,10,150,220),3);
QPen CCanvas::penBorderBlack(QColor(0,0,0,200),3);
QBrush CCanvas::brushBackWhite(QColor(255,255,255,210));
QBrush CCanvas::brushBackYellow(QColor(0xff, 0xff, 0xcc, 0xE0));

#ifdef WIN32
#define isnan(x) _isnan(x)
#endif

CCanvas::CCanvas(QWidget * parent)
: QWidget(parent)
, mouse(0)
, info(0)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    mouseMoveMap    = new CMouseMoveMap(this);
    mouseZoomMap    = new CMouseZoomMap(this);
    mouseSelMap     = new CMouseSelMap(this);
    mouseAddWpt     = new CMouseAddWpt(this);
    mouseMoveWpt    = new CMouseMoveWpt(this);
    mouseEditWpt    = new CMouseEditWpt(this);
#ifdef HAS_POWERDB
    mouseEditPowerNW     = new CMouseEditPowerNW(this);
    mouseEditPower   = new CMouseEditPower(this);
    mouseAddPowerNW   = new CMouseAddPowerNW(this);
    mouseAddPowerLine = new CMouseAddPowerLine(this);
#endif
    mouseRefPoint   = new CMouseRefPoint(this);
    mouseCutTrack   = new CMouseCutTrack(this);
    mouseSelTrack   = new CMouseSelTrack(this);
    mouseAddText    = new CMouseAddText(this);
    mouseAddTextBox = new CMouseAddTextBox(this);
    mouseAddDistance= new CMouseAddDistance(this);
    mouseOverlay    = new CMouseOverlay(this);
    mouseColorPicker = new CMouseColorPicker(this);

    cursorFocus = false;

    profile = new CPlot(CPlotData::eLinear, CPlot::eIcon, this);
    profile->resize(300,120);
    profile->hide();
//    profile->setToolTip(tr("Click to edit track and to see large profile"));

    connect(profile, SIGNAL(activePointSignal(double)), this, SLOT(slotActiveTrackPoint(double)));
    connect(mouseMoveMap, SIGNAL(sigTrkPt(CTrack::pt_t*)), profile, SLOT(slotTrkPt(CTrack::pt_t*)));
}


CCanvas::~CCanvas()
{
}


QColor CCanvas::getSelectedColor()
{
    return mouseColorPicker->getSelectedColor();
}


void CCanvas::slotCopyPosition()
{
    IMap& map = CMapDB::self().getMap();

    double u = posMouse.x();
    double v = posMouse.y();
    map.convertPt2Rad(u,v);
    u = u * RAD_TO_DEG;
    v = v * RAD_TO_DEG;

    QString position;
    GPS_Math_Deg_To_Str(u, v, position);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(position);
}


void CCanvas::setMouseMode(mouse_mode_e mode)
{
    QApplication::restoreOverrideCursor();

    if(mouse) mouse->looseFocus();
    COverlayDB::self().looseFocus();

    switch(mode)
    {

        case eMouseMoveArea:
            mouse = mouseMoveMap;
            break;

        case eMouseZoomArea:
            mouse = mouseZoomMap;
            break;

        case eMouseAddWpt:
            mouse = mouseAddWpt;
            break;

        case eMouseEditWpt:
            mouse = mouseEditWpt;
            break;

#ifdef HAS_POWERDB
        case eMouseAddPowerNW:
            mouse = mouseAddPowerNW;
            break;
            
        case eMouseAddPowerLine:
            mouse = mouseAddPowerLine;
            break;
            
        case eMouseEditPowerNW:
            mouse = mouseEditPowerNW;
            break;
            
        case eMouseEditPower:
            mouse = mouseEditPower;
            break;
#endif

        case eMouseMoveWpt:
            mouse = mouseMoveWpt;
            break;

        case eMouseMoveRefPoint:
            mouse = mouseRefPoint;
            break;

        case eMouseSelectArea:
            mouse = mouseSelMap;
            break;

        case eMouseCutTrack:
            mouse = mouseCutTrack;
            break;

        case eMouseSelTrack:
            mouse = mouseSelTrack;
            break;

        case eMouseAddText:
            mouse = mouseAddText;
            break;

        case eMouseAddTextBox:
            mouse = mouseAddTextBox;
            break;

        case eMouseAddDistance:
            mouse = mouseAddDistance;
            break;

        case eMouseOverlay:
            mouse = mouseOverlay;
            break;

        case eMouseColorPicker:
            mouse = mouseColorPicker;
            break;

        default:;

    }
    if(underMouse())
    {
        QApplication::setOverrideCursor(*mouse);
    }
    mouseMode = mode;
    update();
}


void CCanvas::resizeEvent(QResizeEvent * e)
{
    QWidget::resizeEvent(e);

    QSize s = e->size();

    profile->move(20, s.height() - profile->height() - 20);


    emit sigResize(e->size());
}


void CCanvas::paintEvent(QPaintEvent * e)
{
    QWidget::paintEvent(e);

    QPainter p;
    p.begin(this);
    p.fillRect(rect(),Qt::white);
    p.setFont(CResources::self().getMapFont());
    draw(p);

    p.end();
}


void CCanvas::mouseMoveEvent(QMouseEvent * e)
{
    posMouse = e->pos();
    mouseMoveEventCoord(e);
    mouse->mouseMoveEvent(e);
}


void CCanvas::mousePressEvent(QMouseEvent * e)
{
    posMouse = e->pos();
    mouse->mousePressEvent(e);
}


void CCanvas::mouseReleaseEvent(QMouseEvent * e)
{
    posMouse = e->pos();
    mouse->mouseReleaseEvent(e);
}

void CCanvas::mouseDoubleClickEvent(QMouseEvent * e)
{
    posMouse = e->pos();
    mouse->mouseDoubleClickEvent(e);
}

void CCanvas::keyPressEvent(QKeyEvent * e)
{
    mouse->keyPressEvent(e);
}

void CCanvas::keyReleaseEvent(QKeyEvent * e)
{
    mouse->keyReleaseEvent(e);
}


void CCanvas::enterEvent(QEvent * )
{
    if (!cursorFocus)
    {
        QApplication::setOverrideCursor(*mouse);
        cursorFocus = true;
    }
}


void CCanvas::leaveEvent(QEvent * )
{
    if (cursorFocus)
    {
        QApplication::restoreOverrideCursor();
        cursorFocus = false;
    }
}


#define BORDER  0

void CCanvas::print(QPainter& p, const QSize& pagesize)
{
    bool  rotate = false;
    QSize _size_ = pagesize;
    qreal s = 0.0;

    qDebug() << "pagesize" << pagesize;

    p.save();

    if(pagesize.height() > pagesize.width())
    {
        _size_.setWidth(pagesize.height());
        _size_.setHeight(pagesize.width());
        rotate = true;
        p.rotate(90.0);
        p.translate(0,-pagesize.width());
    }

    qreal s1 = (qreal)(_size_.width() - 2 * BORDER) / (qreal)size().width();
    qreal s2 = (qreal)(_size_.height() - 2 * BORDER) / (qreal)size().height();

    s = (s1 > s2) ? s2 : s1;

    p.translate(BORDER,BORDER);
    p.scale(s,s);
    p.setClipRegion(rect());
    p.setFont(CResources::self().getMapFont());
    draw(p);
    p.restore();
}


void CCanvas::print(QPrinter& printer)
{
    QPainter p;

    p.begin(&printer);
    print(p, printer.pageRect().size());
    p.end();

}


void CCanvas::print(QImage& img)
{
    QPainter p;

    p.begin(&img);
    p.fillRect(rect(), QBrush(Qt::white));
    draw(p);
    p.end();

}

void CCanvas::print(QImage& img, const QSize& pagesize)
{

    bool rotate = false;
    QSize s = pagesize ;
    if(pagesize.width() < pagesize.height())
    {
        s = QSize(s.height(), s.width());
        rotate = true;
    }

    img = QImage(s,  QImage::Format_ARGB32);
    img.fill(Qt::white);

    {
        QPainter p(&img);
        print(p, s);
    }

    if(rotate)
    {
        QMatrix matrix;
        matrix.rotate(90);
        img = img.transformed(matrix, Qt::SmoothTransformation);
    }


//    bool rotate = false;
//    QImage _img_(size(), QImage::Format_ARGB32);
//    if(pagesize.height() > pagesize.width())
//    {
//        _img_ = QImage(size().height(), size().width(), QImage::Format_ARGB32);
//        rotate = true;
//    }

//    _img_.fill(Qt::white);

//    QPainter p;
//    p.begin(&_img_);
//    if(rotate)
//    {
//        p.rotate(90.0);
//        p.translate(0,-size().height());
//    }
//    draw(p);
//    p.end();

//    qreal s1 = (qreal)(_img_.width())  / (qreal)pagesize.width();
//    qreal s2 = (qreal)(_img_.height()) / (qreal)pagesize.height();

//    if(s1 < s2)
//    {
//        img = _img_.scaledToHeight(pagesize.height(),  Qt::SmoothTransformation);
//    }
//    else
//    {
//        img = _img_.scaledToWidth(pagesize.width(),  Qt::SmoothTransformation);
//    }

}


void CCanvas::draw(QPainter& p)
{
    IMap& map = CMapDB::self().getMap();
    bool needsRedraw = map.getNeedsRedraw();

    // printf("draw canvas %d\n",needsRedraw);

    USE_ANTI_ALIASING(p,!map.getFastDrawFlag() && CResources::self().useAntiAliasing());

    CMapDB::self().draw(p,rect(), needsRedraw);
    CRouteDB::self().draw(p, rect(), needsRedraw);
    CTrackDB::self().draw(p, rect(), needsRedraw);
    COverlayDB::self().draw(p, rect(), needsRedraw);
    CLiveLogDB::self().draw(p, rect(), needsRedraw);
    CWptDB::self().draw(p, rect(), needsRedraw);
    CSearchDB::self().draw(p, rect(), needsRedraw);
#ifdef HAS_POWERDB
    CPowerDB::self().draw(p, rect(), needsRedraw);
#endif

    drawRefPoints(p);
    drawScale(p);
    drawCompass(p);

    mouse->draw(p);
}


void CCanvas::drawRefPoints(QPainter& p)
{
    CCreateMapGeoTiff * dlg = CCreateMapGeoTiff::self();
    if(dlg == 0) return;

    IMap& map = CMapDB::self().getMap();

    QMap<quint32,CCreateMapGeoTiff::refpt_t>& refpts         = dlg->getRefPoints();
    QMap<quint32,CCreateMapGeoTiff::refpt_t>::iterator refpt = refpts.begin();
    while(refpt != refpts.end())
    {
        double x = refpt->x;
        double y = refpt->y;
        map.convertM2Pt(x,y);

        if(rect().contains(x,y))
        {
            p.drawPixmap(x - 15,y - 31,QPixmap(":/icons/iconRefPoint31x31.png"));
            drawText(refpt->item->text(CCreateMapGeoTiff::eLabel),p,QPoint(x, y - 35));
        }

        ++refpt;
    }
}


void CCanvas::drawScale(QPainter& p)
{
    if(!CResources::self().showScale())
    {
        return;
    }

    IMap& map = CMapDB::self().getMap();

    double a,b,d;
    int yshift = 0;
    if (QApplication::desktop()->height() < 650) yshift = 60 ;
    QPoint px1(rect().bottomRight() - QPoint(100,50 + yshift));

    // step I: get the approximate distance for 200px in the bottom right corner
    double u1 = px1.x();
    double v1 = px1.y();

    double u2 = px1.x() - 200;
    double v2 = v1;

    map.convertPt2M(u1,v1);
    map.convertPt2M(u2,v2);

    if(map.isLonLat())
    {
        XY p1,p2;
        double a1,a2;
        p1.u = u1;
        p1.v = v1;
        p2.u = u2;
        p2.v = v2;
        d = distance(p1, p2, a1, a2);

    }
    else
    {
        d = u1 - u2;
        //     qDebug() << log10(d) << d << a << b;
    }

    // step II: derive the actual scale length in [m]
    a = (int)log10(d);
    b = log10(d) - a;

    if(0 <= b && b < log10(3.0f))
    {
        d = 1 * pow(10,a);
    }
    else if(log10(3.0f) < b && b < log10(5.0f))
    {
        d = 3 * pow(10,a);
    }
    else
    {
        d = 5 * pow(10,a);
    }

    //     qDebug() << "----" << d;

    // step III: convert the scale length from [m] into [px]
    XY pt1, pt2;
    pt1.u = px1.x();
    pt1.v = px1.y();
    map.convertPt2Rad(pt1.u,pt1.v);

    if(pt1.u == px1.x() && pt1.v == px1.y())
    {
        return;
    }

    pt2 = GPS_Math_Wpt_Projection(pt1, d, -90 * DEG_TO_RAD);
    map.convertRad2Pt(pt2.u, pt2.v);

    if(isnan(pt2.u) || isnan(pt2.v) || abs(pt2.u) > 5000) return;

    // step IV: draw the scale
    QPoint px2(px1 - QPoint(px1.x() - pt2.u,0));

    p.setPen(QPen(Qt::white, 9));
    p.drawLine(px1, px2 + QPoint(9,0));
    p.setPen(QPen(Qt::black, 7));
    p.drawLine(px1, px2 + QPoint(9,0));
    p.setPen(QPen(Qt::white, 5));
    p.drawLine(px1, px2 + QPoint(9,0));

    QVector<qreal> pattern;
    pattern << 2 << 4;
    QPen pen(Qt::black, 5, Qt::CustomDashLine);
    pen.setDashPattern(pattern);
    p.setPen(pen);
    p.drawLine(px1, px2 + QPoint(9,0));

    QPoint px3(px2.x() + (px1.x() - px2.x())/2, px2.y());

    QString val, unit;
    IUnit::self().meter2distance(d,val,unit);
    drawText(QString("%1 %2").arg(val).arg(unit), p, px3, Qt::black);
}


void CCanvas::drawCompass(QPainter& p)
{
    if(!CResources::self().showNorthIndicator())
    {
        return;
    }
    QPolygon arrow;

#define H 60
#define W 30

    arrow << QPoint(0, -H/2) << QPoint(-W/2, H/2) << QPoint(0, H/3) << QPoint(W/2, H/2);

    p.save();

    p.translate(size().width() - 50, size().height() - 50);
    p.rotate(-CMapDB::self().getMap().getAngleNorth());

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(Qt::white,3));
    p.drawPolygon(arrow);

    p.setPen(QPen(Qt::black,1));
    p.setBrush(QColor(150,150,255,100));
    p.drawPolygon(arrow);

    drawText("N", p, QPoint(0, -H/2));
    drawText("S", p, QPoint(0, +H/2 + 15));

    p.restore();
}


void CCanvas::drawText(const QString& str, QPainter& p, const QPoint& center, const QColor& color)
{
    CCanvas::drawText(str,p,center, color, CResources::self().getMapFont());
}


void CCanvas::drawText(const QString& str, QPainter& p, const QPoint& center, const QColor& color, const QFont& font)
{

    QFontMetrics    fm(font);
    QRect           r = fm.boundingRect(str);

    r.moveCenter(center);

    p.setPen(Qt::white);
    p.setFont(font);

    p.drawText(r.topLeft() - QPoint(-1,-1), str);
    p.drawText(r.topLeft() - QPoint( 0,-1), str);
    p.drawText(r.topLeft() - QPoint(+1,-1), str);

    p.drawText(r.topLeft() - QPoint(-1, 0), str);
    p.drawText(r.topLeft() - QPoint(+1, 0), str);

    p.drawText(r.topLeft() - QPoint(-1,+1), str);
    p.drawText(r.topLeft() - QPoint( 0,+1), str);
    p.drawText(r.topLeft() - QPoint(+1,+1), str);

    p.setPen(color);
    p.drawText(r.topLeft(),str);

}


void CCanvas::drawText(const QString& str, QPainter& p, const QRect& r, const QColor& color)
{

    p.setPen(Qt::white);
    p.setFont(CResources::self().getMapFont());

    p.drawText(r.adjusted(-1,-1,-1,-1),Qt::AlignCenter,str);
    p.drawText(r.adjusted( 0,-1, 0,-1),Qt::AlignCenter,str);
    p.drawText(r.adjusted(+1,-1,+1,-1),Qt::AlignCenter,str);

    p.drawText(r.adjusted(-1, 0,-1, 0),Qt::AlignCenter,str);
    p.drawText(r.adjusted(+1, 0,+1, 0),Qt::AlignCenter,str);

    p.drawText(r.adjusted(-1,+1,-1,+1),Qt::AlignCenter,str);
    p.drawText(r.adjusted( 0,+1, 0,+1),Qt::AlignCenter,str);
    p.drawText(r.adjusted(+1,+1,+1,+1),Qt::AlignCenter,str);

    p.setPen(color);
    p.drawText(r,Qt::AlignCenter,str);

}


void CCanvas::wheelEvent(QWheelEvent * e)
{
    zoom(CResources::self().flipMouseWheel() ? (e->delta() > 0) : (e->delta() < 0), e->pos());
}


void CCanvas::zoom(bool in, const QPoint& p)
{
    CUndoStackView::getInstance()->push(new CCanvasUndoCommandZoom(in, p));
    update();
}


void CCanvas::move(double lon, double lat)
{
    IMap& map = CMapDB::self().getMap();
    double u = lon * DEG_TO_RAD;
    double v = lat * DEG_TO_RAD;
    map.convertRad2Pt(u,v);

    CUndoStackView::getInstance()->push(new CMapUndoCommandMove(&map,QPoint(u,v), rect().center()));
    update();
}


void CCanvas::move(move_direction_e dir)
{
    IMap& map = CMapDB::self().getMap();
    QPoint p1 = geometry().center();
    QPoint p2 = p1;

    switch(dir)
    {

        case eMoveLeft:
            p2.rx() += width() / 4;
            break;

        case eMoveRight:
            p2.rx() -= width() / 4;
            break;

        case eMoveUp:
            p2.ry() += height() / 4;
            break;

        case eMoveDown:
            p2.ry() -= height() / 4;
            break;

        case eMoveCenter:
        {
            double lon1 = 0, lat1 = 0, lon2 = 0, lat2 = 0;

            map.dimensions(lon1, lat1, lon2, lat2);

            lon1 += (lon2 - lon1)/2;
            lat2 += (lat1 - lat2)/2;
            map.convertRad2Pt(lon1,lat2);

            p1.rx() = lon1;
            p1.ry() = lat2;

            p2 = geometry().center();

        }
        break;
    }
    CUndoStackView::getInstance()->push(new CMapUndoCommandMove(&map,p1, p2));

    update();
}


void CCanvas::mouseMoveEventCoord(QMouseEvent * e)
{
    IMap& map = CMapDB::self().getMap();
    QString info;                // = QString("%1 %2, ").arg(e->x()).arg(e->y());

    double x = e->x();
    double y = e->y();

    double x_m = e->x();
    double y_m = e->y();

    map.convertPt2Rad(x,y);
    map.convertPt2M(x_m,y_m);

    //    qDebug() << x * RAD_TO_DEG << y * RAD_TO_DEG << ">>>" << x_m << y_m;

    if((x == e->x()) && (y == e->y()))
    {
        map.convertPt2M(x,y);
        info += QString(" (%1 %2)").arg(x,0,'f',0).arg(y,0,'f',0);
//        qDebug() << "--" << info;
    }
    else
    {

        float ele = CMapDB::self().getDEM().getElevation(x,y);
        if(ele != WPT_NOFLOAT)
        {
            QString val, unit;
            IUnit::self().meter2elevation(ele, val, unit);
            info += QString(" (ele: %1 %2) ").arg(val).arg(unit);
        }

        info += QString("[%1m, %2m] ").arg(x_m,0,'f',0).arg(y_m,0,'f',0);

        x *= RAD_TO_DEG;
        y *= RAD_TO_DEG;

        QString str;
        GPS_Math_Deg_To_Str(x,y, str);
        info += str + " ";
                                 //add plain degrees
        //         info += QString(", %1\260 %2\260 ").arg(y,0,'f',7).arg(x,0,'f',7);
    }
    //     qDebug() << "------" << info;
    theMainWindow->setPositionInfo(info);

}


void CCanvas::raiseContextMenu(const QPoint& pos)
{
    QMenu menu(this);

    if(!CMegaMenu::self().isEnabled())
    {
        foreach(QAction *a, *theMainWindow->getActionGroupProvider()->getActiveActions())
        {
            menu.addAction(a);
        }
        menu.addSeparator();
    }
//    menu.addAction(QIcon(":/icons/iconClipboard16x16.png"),tr("Copy Position"),this,SLOT(slotCopyPosition()));
    mouse->contextMenu(menu);

    QPoint p = mapToGlobal(pos);
    menu.exec(p);
}


void CCanvas::showEvent ( QShowEvent * event )
{
    IMap& map = CMapDB::self().getMap();
    map.resize(size());
}

void CCanvas::slotTrackChanged()
{
    CTrack * trk = CTrackDB::self().highlightedTrack();
    slotHighlightTrack(trk);
}

void CCanvas::slotActiveTrackPoint(double dist)
{
    CTrack * trk = CTrackDB::self().highlightedTrack();
    if(trk == 0)
    {
        return;
    }
    CTrack::pt_t * pt = trk->getPointOfFocus(dist);
    if(pt == 0)
    {
        return;
    }

    mouseMoveMap->setSelTrackPt(pt);
}

void CCanvas::slotHighlightTrack(CTrack * track)
{
    if(track && CResources::self().showTrackProfilePreview())
    {
        QPolygonF lineElev;
        QPointF   focusElev;
        float basefactor = IUnit::self().basefactor;

        profile->clear();

        QList<CTrack::pt_t>& trkpts = track->getTrackPoints();
        QList<CTrack::pt_t>::const_iterator trkpt = trkpts.begin();
        while(trkpt != trkpts.end())
        {
            if(trkpt->flags & CTrack::pt_t::eDeleted)
            {
                ++trkpt; continue;
            }

            if(trkpt->ele != WPT_NOFLOAT)
            {
                lineElev << QPointF(trkpt->distance, trkpt->ele * basefactor);
            }

            if(trkpt->flags & CTrack::pt_t::eFocus)
            {
                focusElev  = QPointF(trkpt->distance, trkpt->ele * basefactor);
            }

            trkpt++;
        }

        profile->newLine(lineElev,focusElev, "GPS");
        profile->setXLabel(track->getName());
        profile->setLimits();
        profile->resetZoom();
        profile->show();
        disconnect(profile, SIGNAL(sigClicked()), CTrackDB::self().getToolWidget(), SLOT(slotShowProfile()));
        connect(profile, SIGNAL(sigClicked()), CTrackDB::self().getToolWidget(), SLOT(slotShowProfile()));

    }
    else
    {
        profile->clear();
        profile->hide();
        disconnect(profile, SIGNAL(sigClicked()), CTrackDB::self().getToolWidget(), SLOT(slotShowProfile()));
    }
}
