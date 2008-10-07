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
#ifndef CTRACK3DWIDGET_H
#define CTRACK3DWIDGET_H

#include <QGLWidget>
#include <QPointer>
#include <QWidget>

#include "CTrack.h"

class CTrack3DWidget: public QGLWidget 
{
    Q_OBJECT;
    public:
        CTrack3DWidget(QWidget * parent);
        virtual ~CTrack3DWidget();
        QSize minimumSizeHint() const;
        QSize sizeHint() const;

    protected:
        QPointer<CTrack> track;
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void wheelEvent ( QWheelEvent * e );

    private:
        GLuint makeObject();
        void setMapTexture();    
        void quad(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2);
        void normalizeAngle(double *angle);

        GLuint object;
        double xRot;
        double zRot;
        double xRotSens;
        double zRotSens;
        GLuint mapTexture;
        double xShift, yShift, zoomFactor, eleZoomFactor;

        double maxElevation, minElevation;

        QPoint lastPos;
        QColor wallCollor;
        QColor highBorderColor;


    private slots:
        void slotChanged();

    public slots:
        void setXRotation(double angle);
        void setZRotation(double angle);

    signals:
        void xRotationChanged(double angle);
        void zRotationChanged(double angle);
};

#endif //CTRACK3DWIDGET_H

