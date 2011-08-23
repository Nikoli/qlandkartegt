/**********************************************************************************************
    Copyright (C) 2011 Jan Rheinländer jrheinlaender@users.sourceforge.net

    Based on CPowerDB.cpp by Oliver Eichler.

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

#include "CPowerDB.h"
#include "config.h"
//#include "WptIcons.h"

#include "CWptDB.h"
#include "CWpt.h"
//#include "CTextEditWidget.h"
//#include "CResources.h"
//#include "CMainWindow.h"
//#include "CCanvas.h"
//#include "IMap.h"
//#include "CDlgEditFolder.h"

//#include "CQlb.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant> // required for query code to work!

#define QUERY_EXEC(cmd) \
if(!query.exec())\
{\
    qDebug() << query.lastQuery();\
    qDebug() << query.lastError();\
    cmd;\
}\

#define PROGRESS_SETUP(lbl, max) \
QProgressDialog progress(lbl, "Abort", 0, max, 0);\

#define PROGRESS(x, cmd) \
progress.setValue(x); \
if (progress.wasCanceled()) cmd; \
qApp->processEvents(QEventLoop::ExcludeUserInputEvents);\


class CPowerDBInternalEditLock
{
    public:
        CPowerDBInternalEditLock(CPowerDB * db) : db(db){db->isInternalEdit += 1;}
        ~CPowerDBInternalEditLock(){db->isInternalEdit -= 1;}
    private:
        CPowerDB * db;
};

CPowerDB * CPowerDB::m_self = 0;

CPowerDB::CPowerDB(QObject * parent)
    : isInternalEdit(0)
{
    m_self = this;

    setObjectName("PowerDB");

    connect(&CWptDB::self(), SIGNAL(sigChanged()), this, SLOT(slotWptDBChanged()));
    connect(&CWptDB::self(), SIGNAL(sigModified(const QString&)), this, SLOT(slotWptDBKeyModified(const QString&)));
}

CPowerDB::~CPowerDB()
{
}

void CPowerDB::initDB()
{
    //qDebug() << "void CPowerDB::initDB()";
    QSqlQuery query(*db);

    if(query.exec( "CREATE TABLE versioninfo ( version TEXT )"))
    {
        query.prepare( "INSERT INTO versioninfo (version) VALUES(:version)");
        query.bindValue(":version", POWERDB_VERSION);
        QUERY_EXEC(;);
    }

    if(!query.exec( "CREATE TABLE wpts ("
        "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "key            VARCHAR NOT NULL,"
        "sticky         INTEGER UNSIGNED DEFAULT 0,"
        "timestamp      DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "icon           VARCHAR,"
        "name           VARCHAR,"
        "comment        VARCHAR,"
        "lat            FLOAT NOT NULL,"
        "lon            FLOAT NOT NULL,"
        "altitude       FLOAT,"
        "proximity      FLOAT"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    if(!query.exec( "CREATE TABLE images ("
        "id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "info      VARCHAR,"
        "image     BLOB"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    if(!query.exec( "CREATE TABLE image2wpt ("
        "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "wpt            INTEGER NOT NULL,"
        "image          INTEGER NOT NULL,"
        "FOREIGN KEY(wpt)   REFERENCES wpts(id),"
        "FOREIGN KEY(image) REFERENCES images(id)"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }
  
    if(!query.exec( "CREATE TABLE wpts_eElectric ("
        "wpt_id        INTEGER,"
        "consumers     FLOAT,"
        "load          FLOAT,"
        "FOREIGN KEY(wpt_id) REFERENCES wpts(id)"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }
}

void CPowerDB::migrateDB(int version)
{
    qDebug() << "void CPowerDB::migrateDB(int version)" << version;
    QSqlQuery query(*db);

    for(version++; version <= POWERDB_VERSION; version++)
    {

        switch(version)
        {
            case 1:
                break;

            case 2:
            {
                /*if(!query.exec( "CREATE TABLE workspace ("
                    "id             INTEGER PRIMARY KEY NOT NULL,"
                    "changed        BOOLEAN DEFAULT FALSE,"
                    "data           BLOB NOT NULL"
                ")"))
                {
                    qDebug() << query.lastQuery();
                    qDebug() << query.lastError();
                    return;
                }*/

                break;
            }

        }
    }
    query.prepare( "UPDATE versioninfo set version=:version");
    query.bindValue(":version", version - 1);
    QUERY_EXEC(;);
}

void CPowerDB::clear()
{
  if (db == NULL) return; // No database loaded at all

  // This deletes the connection, not the database file itself
  delete(db);
  QSqlDatabase::removeDatabase("qlpowerdb");
}

// Load the given database
void CPowerDB::load(const QString& filename)
{
  db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE","qlpowerdb"));
  db->setDatabaseName(filename);
  db->open(); // Note: Don't try to close() this or removeDatabase() it, that will just give an error

  QSqlQuery query(*db);

  if(!query.exec("PRAGMA locking_mode=EXCLUSIVE")) {
    return;
  }

  if(!query.exec("PRAGMA synchronous=OFF")) {
    return;
  }

  if(!query.exec("SELECT version FROM versioninfo"))
  {
      initDB();
  }
  else if(query.next())
  {
      int version = query.value(0).toInt();
      if(version != POWERDB_VERSION)
      {
          migrateDB(version);
      }
  }
  else
  {
      initDB();
  }

  // Add whatever waypoints are currently loaded from CWptDB to the DB
  slotWptDBChanged();

  // Find out which new keys must be  added to CWptDB
  query.exec("SELECT key from wpts");

  while (query.next()) {
    //qDebug() << "query: " << query.value(0).toString();
    QString key = query.value(0).toString();

    if (!CWptDB::self().contains(key)) {
      QSqlQuery query2(*db);
      query2.prepare("SELECT sticky, icon, name, comment, lat, lon, altitude, proximity FROM wpts WHERE key=:key");
      query2.bindValue(":key", key);
      query2.exec();
      
      if (query2.next())
      {
        CWpt* wp = new CWpt(this);
        wp->setKey(key);
        wp->sticky = query2.value(0).toInt();
        wp->setIcon(query2.value(1).toString());
        wp->setName(query2.value(2).toString());
        wp->setComment(query2.value(3).toString());
        wp->lat = query2.value(4).toFloat();
        wp->lon = query2.value(5).toFloat();
        wp->ele = query2.value(6).toFloat();
        wp->prx = query2.value(7).toFloat();
        // TODO: Retrieve rest of waypoint information from other tables

        CWptDB::self().addWpt(wp, true);
      }
    }
  }

  emit sigModified();
  emit sigChanged();
}

// Save the database to the given file
void CPowerDB::save(const QString& filename) 
{
  // If the database is already open, then it is automatically saved anyway
  if (db != NULL) return;

  // Remove an old file by the same name (the UI already warned the user that this would happen)
  QFileInfo fileInfo(filename);
  if (fileInfo.exists()) {
    QFile::remove(filename);
  }
    
  // Create a new database
  load(filename); // This will automatically put all the current waypoints in CWptDB into the DB
}

// Propagate any modifications of existing waypoints into the DB
void CPowerDB::slotWptDBKeyModified(const QString& key) {
  if (db == NULL) return; // No database loaded

  QSqlQuery query(*db);
  query.prepare("SELECT name FROM wpts WHERE key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return);

  if (query.next()) {
    qDebug() << "Existing key '" << key << "' modified: " << query.value(0).toString();

    CWpt* wpt = CWptDB::self().getWptByKey(key);
    
    QSqlQuery query2(*db);
    query2.prepare("UPDATE wpts SET sticky=:sticky, timestamp=:timestamp, icon=:icon, name=:name, comment=:comment, "\
                                  "lat=:lat, lon=:lon, altitude=:altitude, proximity=:proximity WHERE key=:key");
    query2.bindValue(":key", key);
    query2.bindValue(":sticky", wpt->sticky);
    query2.bindValue(":timestamp", wpt->getTimestamp());
    query2.bindValue(":icon", wpt->getIconString());
    query2.bindValue(":name", wpt->getName());
    query2.bindValue(":comment", wpt->getComment());
    query2.bindValue(":lat", wpt->lat);
    query2.bindValue(":lon", wpt->lon);
    query2.bindValue(":altitude", wpt->ele);
    query2.bindValue(":proximity", wpt->prx);
    query2.exec();
  } else {
    qDebug() << "slotWptDBKeyModified called with unknown key!";
  }

  // Tell CMainWindow that the DB was modified so that it can be saved on exit
  // emit sigModified(); // Not necessary!
} // slotWptDBKeyModified()


void CPowerDB::slotWptDBChanged()
{
  if (db == NULL) return; // No database loaded

  QSqlQuery query(*db);
  QSet<QString> keysDB;

  // Found out which waypoints we already have in the DB
  query.prepare("SELECT key FROM wpts");
  QUERY_EXEC(return);

  while (query.next())
    keysDB.insert(query.value(0).toString());

  qDebug() << "Existing waypoints in DB: " << keysDB;

  // Find out which new keys have appeared in CWptDB
  QMap<QString,CWpt*> keysWptDB = CWptDB::self().getWpts();

  for (QMap<QString,CWpt*>::const_iterator i = keysWptDB.constBegin(); i != keysWptDB.constEnd(); i++) 
    if (!keysDB.contains(i.key())) {
      qDebug() << "Found new waypoint: " << i.value()->getInfo();
      query.prepare("INSERT INTO wpts (key, sticky, timestamp, icon, name, comment, lat, lon, altitude, proximity) "\
                    "VALUES(:key, :sticky, :timestamp, :icon, :name, :comment, :lat, :lon, :altitude, :proximity)");
      query.bindValue(":key", i.key());
      query.bindValue(":sticky", i.value()->sticky);
      query.bindValue(":timestamp", i.value()->getTimestamp());
      query.bindValue(":icon", i.value()->getIconString());
      query.bindValue(":name", i.value()->getName());
      query.bindValue(":comment", i.value()->getComment());
      query.bindValue(":lat", i.value()->lat);
      query.bindValue(":lon", i.value()->lon);
      query.bindValue(":altitude", i.value()->ele);
      query.bindValue(":proximity", i.value()->prx);
      QUERY_EXEC(return);
      
      // TODO: We should to insert the rest of the waypoint data, too
    }
  
  // Tell CMainWindow that the DB was modified so that it can be saved on exit
  // emit sigModified(); // Not necessary!
}


