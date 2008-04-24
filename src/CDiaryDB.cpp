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

#include "CDiaryDB.h"
#include "CDiaryEditWidget.h"
#include "CTabWidget.h"
#include "CQlb.h"
#include "CGpx.h"

#include <QtGui>

CDiaryDB * CDiaryDB::m_self = 0;

CDiaryDB::CDiaryDB(QTabWidget * tb, QObject * parent)
    : IDB(tb, parent)
    , diary(this)
{
    m_self = this;
}

CDiaryDB::~CDiaryDB()
{

}

void CDiaryDB::openEditWidget()
{

    CTabWidget * tb = dynamic_cast<CTabWidget*>(tabbar);
    if(tb == 0) return;

    if(editWidget.isNull()){
        editWidget = new CDiaryEditWidget(diary.text(), tabbar);
        tb->addTab(editWidget,tr("Diary"));

    }
    else{
        diary.setText(editWidget->textEdit->toHtml());
        tb->delTab(editWidget);

    }
}

void CDiaryDB::loadQLB(CQlb& qlb)
{
    QDataStream stream(&qlb.diary(),QIODevice::ReadOnly);
    stream >> diary;
    if(!editWidget.isNull()){
        editWidget->textEdit->setHtml(diary.text());
    }
    emit sigChanged();
}

void CDiaryDB::saveQLB(CQlb& qlb)
{
    if(!editWidget.isNull()){
        diary.setText(editWidget->textEdit->toHtml());
    }
    qlb << diary;
}

void CDiaryDB::clear()
{
    CTabWidget * tb = dynamic_cast<CTabWidget*>(tabbar);
    if(tb == 0) return;

    if(!editWidget.isNull()){
        tb->delTab(editWidget);
    }
    diary = CDiary(this);
    emit sigChanged();
}


const QString CDiaryDB::getDiary()
{
   if(!editWidget.isNull()){
        diary.setText(editWidget->textEdit->toHtml());
    }
    return diary.text();
}

void CDiaryDB::loadGPX(CGpx& gpx)
{
    const QDomNodeList& drys = gpx.elementsByTagName("diary");
    uint N = drys.count();
    for(uint n = 0; n < N; ++n) {
        const QDomNode& dry = drys.item(n);
        QString tmp = diary.text();
        tmp += dry.toElement().text();
        diary.setText(tmp);
        break; // only entry allowed sofar
    }

    emit sigChanged();
}

void CDiaryDB::saveGPX(CGpx& gpx)
{

    QDomElement root        = gpx.documentElement();
    QDomElement extensions  = gpx.createElement("extensions");
    QDomElement dry         = gpx.createElement("diary");
    QDomText text           = gpx.createTextNode(diary.text());

    root.appendChild(extensions);
    extensions.appendChild(dry);
    dry.appendChild(text);
}
