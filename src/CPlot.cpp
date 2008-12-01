/**********************************************************************************************
    Copyright (C) 2006, 2007 Oliver Eichler oliver.eichler@gmx.de

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

#include "CPlot.h"
#include "CPlotAxis.h"
#include "CResources.h"

#include <QtGui>

CPlot::CPlot(QWidget * parent)
: QWidget(parent)
, fontWidth(0)
, fontHeight(0)
, scaleWidthX1(0)
, scaleWidthY1(0)
, left(0)
, right(0)
, top(0)
, bottom(0)
, fm(QFont())
{
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    m_pData = new CPlotData(this);
    createActions();
}


CPlot::~CPlot()
{

}


void CPlot::clear()
{
    m_pData->lines.clear();
    m_pData->marks.points.clear();
    m_pData->tags.clear();
    update();
}


double CPlot::getXValByPixel(int px)
{
    return m_pData->x().pt2val(px - left);
}


void CPlot::setYLabel(const QString& str)
{
    m_pData->ylabel = str;
    setSizes();
    update();
}


void CPlot::setXLabel(const QString& str)
{
    m_pData->xlabel = str;
    setSizes();
    update();
}


void CPlot::setLimits()
{
    m_pData->setLimits();
}


void CPlot::newLine(const QPolygonF& line, const QPointF& focus, const QString& label)
{
    m_pData->lines.clear();

    m_pData->point1.point = focus;

    CPlotData::line_t l;
    l.points    = line;
    l.label     = label;

    m_pData->lines << l;
    setSizes();
    m_pData->x().setScale( rectGraphArea.width() );
    m_pData->y().setScale( rectGraphArea.height() );
    update();

}


void CPlot::addLine(const QPolygonF& line, const QString& label)
{
    CPlotData::line_t l;
    l.points    = line;
    l.label     = label;

    m_pData->lines << l;
    setSizes();
    m_pData->x().setScale( rectGraphArea.width() );
    m_pData->y().setScale( rectGraphArea.height() );
    update();
}


void CPlot::newMarks(const QPolygonF& line)
{
    m_pData->marks.points = line;
}


void CPlot::addTag(CPlotData::point_t& tag)
{
    m_pData->tags << tag;
}


void CPlot::paintEvent(QPaintEvent * )
{
    QPainter p(this);
    draw(p);
}


void CPlot::resizeEvent(QResizeEvent * )
{
    setSizes();
    update();
}


void CPlot::setSizes()
{
    fm = QFontMetrics(CResources::self().getMapFont());
    left = 0;

    scaleWidthX1    = m_pData->x().getScaleWidth( fm );
    scaleWidthY1    = m_pData->y().getScaleWidth( fm );

    scaleWidthY1    = scaleWidthX1 > scaleWidthY1 ? scaleWidthX1 : scaleWidthY1;

    fontWidth       = fm.maxWidth();
    fontHeight      = fm.height();
    deadAreaX       = fontWidth >> 1;
    deadAreaY       = ( fontHeight + 1 ) >> 1;

    setLRTB();
    setSizeIconArea();
    setSizeXLabel();
    setSizeYLabel();
    setSizeDrawArea();

}


void CPlot::setLRTB()
{
    left = 0;

    left += m_pData->ylabel.isEmpty() ? 0 : fontHeight;
    left += scaleWidthY1;
    left += deadAreaX;

    right = size().width();
    right -= deadAreaX;
    right -= scaleWidthX1 / 2;

    top = 0;
    if(!m_pData->tags.isEmpty()) {
        top += fontHeight;
        top += 16;
    }
    top += deadAreaY;

    bottom = size().height();
    bottom -= m_pData->xlabel.isEmpty() ? 0 : fontHeight;
    // tick marks
    bottom -= fontHeight;
    bottom -= deadAreaY;
}


void CPlot::setSizeIconArea()
{
    rectIconArea = QRect(left, deadAreaY, right - left, 16 + fontHeight + deadAreaY);
}


/*
  x = a <br>
  y = widget height - xlabel height <br>
  width = b-a <br>
  height = font height <br>
*/
void CPlot::setSizeXLabel()
{
    int y;
    if ( m_pData->xlabel.isEmpty() ) {
        rectX1Label = QRect( 0, 0, 0, 0 );
    }
    else {
        rectX1Label.setWidth( right - left );
        rectX1Label.setHeight( fontHeight );
        y = ( size().height() - rectX1Label.height());
        rectX1Label.moveTopLeft( QPoint( left, y ) );
    }
}


/*
  assume a -90 rotated coordinate grid

  x = widget height - d <br>
  y = 0 <br>
  width = d-c<br>
  height = font height <br>
*/
void CPlot::setSizeYLabel()
{
    if ( m_pData->ylabel.isEmpty() ) {
        rectY1Label = QRect( 0, 0, 0, 0 );
    }
    else {
        rectY1Label.setWidth( bottom - top );
        rectY1Label.setHeight( fontHeight );
        rectY1Label.moveTopLeft( QPoint( size().height() - bottom, 0 ) );
    }
}


void CPlot::setSizeDrawArea()
{
    rectGraphArea.setWidth( right - left );
    rectGraphArea.setHeight( bottom - top );
    rectGraphArea.moveTopLeft( QPoint( left, top ) );

    m_pData->x().setScale( rectGraphArea.width() );
    m_pData->y().setScale( rectGraphArea.height() );
}


void CPlot::draw(QPainter& p)
{
    p.fillRect(rect(),Qt::white);

    if(m_pData->lines.isEmpty()) return;

    p.setFont(CResources::self().getMapFont());
    p.setClipping(true);
    p.setClipRect(rectGraphArea);
    drawData(p);
    p.setClipping(false);
    drawLabels(p);
    drawXScale(p);
    drawYScale(p);
    drawGridX(p);
    drawGridY(p);
    drawXTic(p);
    drawYTic(p);
    p.setPen(QPen(Qt::black,2));
    p.drawRect(rectGraphArea);
    drawTags(p);

    drawLegend(p);

}


void CPlot::drawLabels( QPainter &p )
{
    p.setPen(Qt::darkBlue);

    if ( rectX1Label.isValid() ) {
        p.drawText( rectX1Label, Qt::AlignCenter, m_pData->xlabel );
    }

    p.save();
    QMatrix m = p.matrix();
    m.translate( 0, size().height() );
    m.rotate( -90 );
    p.setMatrix( m );

    if ( rectY1Label.isValid() ) {
        p.drawText( rectY1Label, Qt::AlignCenter, m_pData->ylabel );
    }
    p.restore();
}


void CPlot::drawXScale( QPainter &p )
{
    QRect recText;

    if ( m_pData->x().getTicType() == CPlotAxis::notic )
        return ;

    p.setPen(Qt::darkBlue);
    recText.setHeight( fontHeight );
    recText.setWidth( scaleWidthX1 );

    int ix;
    int ix_ = -1;
    int iy;

    iy = bottom + deadAreaY;
    const CPlotAxis::TTic * t = m_pData->x().ticmark();
    while ( t ) {
        ix = left + m_pData->x().val2pt( t->val ) - ( scaleWidthX1 + 1 ) / 2;
        if ( ( ( ix_ < 0 ) || ( ( ix - ix_ ) > scaleWidthX1 + 5 ) ) && !t->lbl.isEmpty() ) {
            recText.moveTopLeft( QPoint( ix, iy ) );
            p.drawText( recText, Qt::AlignCenter, t->lbl );
            ix_ = ix;
        }
        t = m_pData->x().ticmark( t );
    }

    double limMin, limMax, useMin, useMax;
    m_pData->x().getLimits(limMin, limMax, useMin, useMax);

    if((limMax - limMin) <= (useMax - useMin)) return;

    double scale = (right - left) / (limMax - limMin);
    double val   = m_pData->x().pt2val(0);

    int x = left + (useMin - limMin) * scale;
    int y = bottom + 5;
    int w = (useMax - useMin) * scale;

    p.setPen(QPen(Qt::red,3));
    p.drawLine(x,y, x + w, y);

}


void CPlot::drawYScale( QPainter &p )
{
    QRect recText;
    if ( m_pData->y().getTicType() == CPlotAxis::notic )
        return ;

    p.setPen(Qt::darkBlue);
    recText.setHeight( fontHeight );
    recText.setWidth( scaleWidthY1 );

    int ix;
    int iy;

    ix = left - scaleWidthY1 - deadAreaX;
    const CPlotAxis::TTic * t = m_pData->y().ticmark();
    while ( t ) {
        iy = bottom - m_pData->y().val2pt( t->val ) - fontHeight / 2;

        recText.moveTopLeft( QPoint( ix, iy ) );
        p.drawText( recText, Qt::AlignRight, t->lbl );
        t = m_pData->y().ticmark( t );
    }

    double limMin, limMax, useMin, useMax;
    m_pData->y().getLimits(limMin, limMax, useMin, useMax);

    if((limMax - limMin) <= (useMax - useMin)) return;

    double scale = (top - bottom) / (limMax - limMin);
    double val   = m_pData->y().pt2val(0);

    int x = left - 5;
    int y = bottom + (useMin - limMin) * scale;
    int h = (useMax - useMin) * scale;

    p.setPen(QPen(Qt::red,3));
    p.drawLine(x,y, x, y + h);

}


void CPlot::drawXTic( QPainter & p )
{
    int ix;
    int iyb, iyt;
    const CPlotAxis::TTic * t = m_pData->x().ticmark();

    p.setPen(QPen(Qt::black,2));
    iyb = rectGraphArea.bottom();
    iyt = rectGraphArea.top();
    while ( t ) {
        ix = left + m_pData->x().val2pt( t->val );
        p.drawLine( ix, iyb, ix, iyb - 5 );
        p.drawLine( ix, iyt, ix, iyt + 5 );
        t = m_pData->x().ticmark( t );
    }
}


void CPlot::drawYTic( QPainter &p )
{
    int ixl, ixr;
    int iy;
    const CPlotAxis::TTic * t = m_pData->y().ticmark();

    p.setPen(QPen(Qt::black,2));
    ixl = rectGraphArea.left();
    ixr = rectGraphArea.right();
    while ( t ) {
        iy = bottom - m_pData->y().val2pt( t->val );
        p.drawLine( ixl, iy, ixl + 5, iy );
        p.drawLine( ixr, iy, ixr - 5, iy );
        t = m_pData->y().ticmark( t );
    }
}


void CPlot::drawGridX( QPainter &p )
{
    int ix;
    int iy, dy;

    CPlotAxis::ETicType oldtic = m_pData->x().setTicType( CPlotAxis::norm );

    dy = rectGraphArea.height();
    const CPlotAxis::TTic * t = m_pData->x().ticmark();

    QPen oldpen = p.pen();
    p.setPen( QPen( QColor(150,150,150,128), 1, Qt::DotLine ) );

    iy = rectGraphArea.top();
    while ( t ) {
        ix = left + m_pData->x().val2pt( t->val );
        p.drawLine( ix, iy, ix, iy + dy );
        t = m_pData->x().ticmark( t );
    }
    p.setPen( oldpen );
    m_pData->x().setTicType( oldtic );
}


void CPlot::drawGridY( QPainter &p )
{
    int ix, dx;
    int iy;

    CPlotAxis::ETicType oldtic = m_pData->y().setTicType( CPlotAxis::norm );
    dx = rectGraphArea.width();
    const CPlotAxis::TTic * t = m_pData->y().ticmark();

    QPen oldpen = p.pen();
    p.setPen( QPen( QColor(150,150,150,128), 1, Qt::DotLine ) );

    ix = rectGraphArea.left();
    while ( t ) {
        iy = bottom - m_pData->y().val2pt( t->val );
        p.drawLine( ix, iy, ix + dx, iy );
        t = m_pData->y().ticmark( t );
    }
    p.setPen( oldpen );
    m_pData->y().setTicType( oldtic );
}


QPen pens[] =
{
    QPen(Qt::blue,3)
    , QPen(Qt::red,1)
    , QPen(Qt::darkYellow,2)
    , QPen(Qt::darkGreen,2)

};

void CPlot::drawData(QPainter& p)
{
    int penIdx = 0;
    int ptx, pty, ptx_old, pty_old;
    QList<CPlotData::line_t> lines                  = m_pData->lines;
    QList<CPlotData::line_t>::const_iterator line   = lines.begin();

    CPlotAxis& xaxis = m_pData->x();
    CPlotAxis& yaxis = m_pData->y();

    while(line != lines.end()) {

        const QPolygonF& polyline       = line->points;
        QPolygonF::const_iterator point = polyline.begin();

        ptx_old = left   + xaxis.val2pt( point->x() );
        pty_old = bottom - yaxis.val2pt( point->y() );

        p.setPen(pens[penIdx++]);
        while(point != polyline.end()) {

            ptx = left   + xaxis.val2pt( point->x() );
            pty = bottom - yaxis.val2pt( point->y() );

            p.drawLine(ptx_old,pty_old,ptx,pty);

            ptx_old = ptx;
            pty_old = pty;

            ++point;
        }

        ++line;
    }

    {
        QPolygonF& marks                = m_pData->marks.points;
        QPolygonF::const_iterator point = marks.begin();
        p.setPen(QPen(Qt::red,2));

        while(point != marks.end()) {
            ptx = left   + xaxis.val2pt( point->x() );
            pty = bottom - yaxis.val2pt( point->y() );

            p.drawLine(ptx-2,pty,ptx+2,pty);
            p.drawLine(ptx,pty-2,ptx,pty+2);

            ++point;
        }
    }

    if(!m_pData->point1.point.isNull()) {
        p.setPen(QPen(Qt::darkGreen));
        ptx = left   + xaxis.val2pt( m_pData->point1.point.x() );
        pty = bottom - yaxis.val2pt( m_pData->point1.point.y() );

        p.drawLine(rectGraphArea.left(),pty,rectGraphArea.right(),pty);
        p.drawLine(ptx,rectGraphArea.top(),ptx,rectGraphArea.bottom());
    }
}


void CPlot::drawLegend(QPainter& p)
{
    if(m_pData->lines.size() < 2) return;

    int penIdx = 0;
    QFontMetrics fm(p.font());
    int h = fm.height();

    int x = rectGraphArea.left() + 10;
    int y = rectGraphArea.top()  + 2 + h;

    QList<CPlotData::line_t> lines                  = m_pData->lines;
    QList<CPlotData::line_t>::const_iterator line   = lines.begin();

    while(line != lines.end()) {
        p.setPen(Qt::black);
        p.drawText(x + 30 ,y,line->label);
        p.setPen(pens[penIdx++]);
        p.drawLine(x, y, x + 20, y);

        y += fm.height();
        ++line;
    }

}


void CPlot::drawTags(QPainter& p)
{
    if(m_pData->tags.isEmpty()) return;

    QRect rect;
    int ptx, pty;
    CPlotAxis& xaxis = m_pData->x();
    CPlotAxis& yaxis = m_pData->y();

    QFontMetrics fm(p.font());

    QVector<CPlotData::point_t>::const_iterator tag = m_pData->tags.begin();
    while(tag != m_pData->tags.end()) {
        ptx = left   + xaxis.val2pt( tag->point.x() );
        pty = bottom - yaxis.val2pt( tag->point.y() );

        if (left < ptx &&  ptx < right) {
            rect = fm.boundingRect(tag->label);
            rect.moveCenter(QPoint(ptx, fontHeight / 2));
            p.setPen(Qt::darkBlue);
            p.drawText(rect, Qt::AlignCenter, tag->label);

            p.drawPixmap(ptx - tag->icon.width() / 2, fontHeight, tag->icon);

            p.setPen(QPen(Qt::white, 3));
            if (fontHeight + 16 < pty) {
                if (pty > bottom){
                    pty = bottom;
                }

                p.drawLine(ptx, fontHeight + 16, ptx, pty);
                p.setPen(QPen(Qt::black, 1));
                p.drawLine(ptx, fontHeight + 16, ptx, pty);
            }
        }
        ++tag;
    }
}


void CPlot::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(hZoomAct);
    menu.addAction(vZoomAct);
    menu.addAction(resetZoomAct);

    menu.exec(event->globalPos());
}


void CPlot::createActions()
{
    hZoomAct = new QAction("horizontal zoom", this);
    hZoomAct->setCheckable(true);
    hZoomAct->setChecked(true);

    vZoomAct = new QAction(tr("vertical zoom"), this);
    vZoomAct->setCheckable(true);
    vZoomAct->setChecked(true);

    resetZoomAct = new QAction(tr("reset zoom"), this);
    connect(resetZoomAct, SIGNAL(triggered()), this, SLOT(resetZoom()));
}


void CPlot::resetZoom()
{
    m_pData->x().resetZoom();
    m_pData->y().resetZoom();
    setSizes();
    update();
}


void CPlot::zoom(CPlotAxis &axis, bool in, int curInt)
{
    axis.zoom(in, curInt);
    setSizes();
    m_pData->x().setScale( rectGraphArea.width() );
    m_pData->y().setScale( rectGraphArea.height() );
    update();
}


void CPlot::wheelEvent ( QWheelEvent * e )
{
    bool in = CResources::self().flipMouseWheel() ? (e->delta() > 0) : (e->delta() < 0);
    if (hZoomAct->isChecked())
        zoom(m_pData->x(), in, e->pos().x() - left);
    if (vZoomAct->isChecked())
        zoom(m_pData->y(), in, - e->pos().y() + bottom);
}


void CPlot::mouseMoveEvent(QMouseEvent * e)
{
    checkClick = false;

    CPlotAxis &xaxis = m_pData->x();
    xaxis.move(startMovePos.x() - e->pos().x());

    CPlotAxis &yaxis = m_pData->y();
    yaxis.move(e->pos().y() - startMovePos.y());

    startMovePos = e->pos();
    update();
}


void CPlot::mouseReleaseEvent(QMouseEvent * e)
{
    QApplication::restoreOverrideCursor();
    if (checkClick && e->button() == Qt::LeftButton) {
        QPoint pos = e->pos();
        double dist = getXValByPixel(pos.x());
        emit activePointSignal(dist);
    }
}


void CPlot::mousePressEvent(QMouseEvent * e)
{
    QApplication::setOverrideCursor(QCursor(QPixmap(":/cursors/cursorMove")));
    startMovePos = e->pos();
    checkClick = true;
}
