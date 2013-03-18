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
#include "CPowerNW.h"
#include "CDlgEditPowerLine.h"
#include "CDlgEditPowerNW.h"
#include "CResources.h"
#include "CMainWindow.h"
#include "CPowerToolWidget.h"
#include "CMapDB.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant> // required for query code to work!
#include <QtGui>
#include "CUndoStackModel.h"

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

QSqlQuery CPowerDB::getCustomQuery()
{
    return QSqlQuery(*db);
}

CPowerDB::CPowerDB(QTabWidget * tb, QObject * parent)
    : tabbar(tb)
    , isInternalEdit(0)
    , db(NULL)
    , cnt(0)
    , showBullets(true)
    , printView(0)
{
    m_self = this;

    setObjectName("PowerDB");
    
    QSettings cfg;
    showBullets = cfg.value("PowerNW/showBullets", showBullets).toBool();
    toolview    = new CPowerToolWidget(tb);
    undoStack   = CUndoStackModel::getInstance();

    db = createDB(":memory:");

    // Add whatever waypoints are currently loaded from CWptDB to the DB
    // At program startup, these are the "sticky" waypoints
    QMap<QString,CWpt*> wpts = CWptDB::self().getWpts();

    for (QMap<QString,CWpt*>::const_iterator i = wpts.constBegin(); i != wpts.constEnd(); i++)
      addWpt(i.value());

    connect(&CWptDB::self(), SIGNAL(sigChanged()), this, SLOT(slotWptDBChanged()));
    connect(&CWptDB::self(), SIGNAL(sigModified(const QString&)), this, SLOT(slotWptDBKeyModified(const QString&)));    
}

CPowerDB::~CPowerDB()
{
    db->close();
    QSqlDatabase::removeDatabase("qlpowerdb_:memory:");
    delete db;
}

void CPowerDB::gainFocus()
{
    if(toolview && tabbar->currentWidget() != toolview)
    {
        tabbar->setCurrentWidget(toolview);
    }
}

CPowerToolWidget * CPowerDB::getToolWidget()
{
    return qobject_cast<CPowerToolWidget*>(toolview);
}

QSqlDatabase* CPowerDB::createDB(const QString& dbname)
{
    //qDebug() << "void CPowerDB::createDB() " << dbname;
    QSqlDatabase* thedb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE","qlpowerdb_"+dbname));
    thedb->setDatabaseName(dbname);

    if (!thedb->open()) {
      QMessageBox::critical(0, "Could not open database", "Internal error opening database");
      return NULL;
    }

    QSqlQuery query(*thedb);

    // Note:
    // ON DELETE CASCADE actually seems to have no effect in the QSQLITE database
    if(query.exec( "CREATE TABLE versioninfo ( version TEXT )"))
    {
        query.prepare( "INSERT INTO versioninfo (version) VALUES(:version)");
        query.bindValue(":version", POWERDB_VERSION);
        QUERY_EXEC(;);
    }

    query.prepare("CREATE TABLE wpts ("
        "key            VARCHAR PRIMARY KEY,"
        "sticky         INTEGER UNSIGNED DEFAULT 0,"
        "timestamp      DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "icon           VARCHAR,"
        "name           VARCHAR,"
        "comment        VARCHAR,"
        "lat            FLOAT NOT NULL,"
        "lon            FLOAT NOT NULL,"
        "altitude       FLOAT,"
        "proximity      FLOAT"
    ")");
    QUERY_EXEC(;);

    query.prepare( "CREATE TABLE images ("
        "id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "info      VARCHAR,"
        "image     BLOB"
    ")");
    QUERY_EXEC(;);

    query.prepare( "CREATE TABLE image2wpt ("
        "id                 INTEGER PRIMARY KEY AUTOINCREMENT,"
        "wpt_key            VARCHAR NOT NULL,"
        "image_id           INTEGER NOT NULL,"
        "FOREIGN KEY(wpt_key)   REFERENCES wpts(key) ON DELETE CASCADE,"
        "FOREIGN KEY(image_id) REFERENCES images(id) ON DELETE CASCADE"
    ")");
    QUERY_EXEC(;);
  
    query.prepare( "CREATE TABLE wpts_eElectric ("
        "wpt_key            VARCHAR,"
        "consumers          FLOAT,"
        "consumers1         FLOAT,"
        "consumers2         FLOAT,"
        "consumers3         FLOAT,"
        "load               FLOAT,"
        "totalload          FLOAT,"
        "loadphase1         FLOAT,"
        "loadphase2         FLOAT,"
        "loadphase3         FLOAT,"
        "resistance         FLOAT,"
        "totalresistance    FLOAT,"
        "voltage            FLOAT,"
        "powerfactor        FLOAT NOT NULL DEFAULT 0.75,"
        "FOREIGN KEY(wpt_key) REFERENCES wpts(key) ON DELETE CASCADE"
    ")");
    QUERY_EXEC(;);
    
    query.prepare( "CREATE TABLE lines ("
        "key                  VARCHAR PRIMARY KEY,"
        "timestamp            DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "icon                 VARCHAR,"
        "name                 VARCHAR,"  
        "comment              VARCHAR,"
        "nw_key               VARCHAR,"
        "first_key            VARCHAR NOT NULL,"
        "second_key           VARCHAR NOT NULL,"
        "cross_section        FLOAT NOT NULL DEFAULT 4.0,"
        "phases               INTEGER NOT NULL DEFAULT 7,"
        "length               FLOAT NOT NULL DEFAULT 0.0,"
        "conductivity         FLOAT NOT NULL DEFAULT 30.0,"
        "resistance           FLOAT,"
        "current              FLOAT,"
        "voltage_drop         FLOAT,"
        "highlighted          INTEGER UNSIGNED DEFAULT 0,"
        "FOREIGN KEY(first_key)  REFERENCES wpts(key) ON DELETE CASCADE,"
        "FOREIGN KEY(second_key) REFERENCES wpts(key) ON DELETE CASCADE,"
        "FOREIGN KEY(nw_key)     REFERENCES wpts(key) ON DELETE CASCADE"
    ")");
    QUERY_EXEC(;);
    
    if (query.exec( "CREATE TABLE networks ("
        "key                VARCHAR PRIMARY KEY,"
        "timestamp          DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "icon               VARCHAR,"
        "name               VARCHAR,"  
        "comment            VARCHAR,"
        "powerhouse_key     VARCHAR,"
        "powerhouse_voltage FLOAT NOT NULL DEFAULT 240,"
        "watts_per_consumer FLOAT NOT NULL DEFAULT 60,"
        "line_conductivity  FLOAT NOT NULL DEFAULT 30,"
        "power_factor       FLOAT NOT NULL DEFAULT 0.75,"
        "ratedvoltage       FLOAT NOT NULL DEFAULT 220,"
        "minimalvoltage     FLOAT NOT NULL DEFAULT 200,"
        "highlighted        INTEGER UNSIGNED DEFAULT 0,"
        "FOREIGN KEY(powerhouse_key) REFERENCES wpts(key) ON DELETE CASCADE"
    ")"))
    {
        query.prepare("INSERT INTO networks (key, name, powerhouse_key) "
                      "VALUES ('unassigned_power_lines', 'unassigned power lines', '')");
        QUERY_EXEC(;);
    }
    else
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    //qDebug() << "Created database " << dbname;
    return thedb;
}

void CPowerDB::migrateDB(QSqlDatabase* thedb, int version)
{
    qDebug() << "void CPowerDB::migrateDB(int version)" << version;
    QSqlQuery query(*thedb);

    for(version++; version <= POWERDB_VERSION; version++)
    {

        switch(version)
        {
            case 1:
                // This applies to migration from version 0 to 1, because of the version++
                break;

            case 2:
            {
            if (!query.exec( "ALTER TABLE networks ADD COLUMN minimalvoltage FLOAT NOT NULL DEFAULT 200"))
                {
                    qDebug() << query.lastQuery();
                    qDebug() << query.lastError();
                    return;
                }
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
  //qDebug() << "CPowerDB::clear()";

  // This deletes the connection, not the database file itself
  db->close();
  QSqlDatabase::removeDatabase("qlpowerdb_:memory:");
  delete db;
  db = createDB(":memory:");
}

// Load the given database into the memory database (adding to existing content if there is any)
void CPowerDB::load(const QString& filename)
{
  qDebug() << "CPowerDB::load() " << filename;

  QSqlDatabase* newdb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE","qlpowerdbnew"));
  newdb->setDatabaseName(filename);

  if (!newdb->open()) {
    QMessageBox::critical(0, "Could not open database", "Please make sure the file exists");
    return;
  }

  bool changed = false;

  {
      QSqlQuery query(*newdb); // Put this in a separate scopy so that it is destroyed afterwards

      if(!query.exec("PRAGMA locking_mode=EXCLUSIVE")) {
        QMessageBox::critical(0, "Could not open database", "Database is locked by other process");
        return;
      }

      if(!query.exec("PRAGMA synchronous=OFF")) {
        QMessageBox::critical(0, "Could not open database", "Database synchronous mode could not be disabled");
        return;
      }

      if(!query.exec("SELECT version FROM versioninfo"))
      {
          QMessageBox::critical(0, "Could not open database", "Empty database or not QLandkarteGT database format");
          return;
      }
      else if(query.next())
      {
          int version = query.value(0).toInt();
          if(version != POWERDB_VERSION)
          {
              migrateDB(newdb, version);
          }
      }

      // Clear the highlighted line and network (leftover from last run)
      query.exec( "UPDATE lines SET highlighted=0");
      query.exec( "UPDATE networks SET highlighted=0");

      // Find out which new keys must be added to CWptDB
      query.exec("SELECT key from wpts");

      while (query.next()) {
        QString key = query.value(0).toString();

        if (!CWptDB::self().contains(key)) {
          //qDebug() << "Adding new key " << key << " to CWptDB";
          QSqlQuery query2(*newdb);
          query2.prepare("SELECT sticky, icon, name, comment, lat, lon, altitude, proximity FROM wpts WHERE key=:key");
          query2.bindValue(":key", key);
          query2.exec();

          if (query2.next())
          {
            //qDebug() << "Found data";
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

            CWptDB::self().addWpt(wp, true); // Must be silent because otherwise slotWptDBChanged() is called!
            changed = true;
          }
        }
      }
  }

  // Copy data from file to memory database
  newdb->close();
  QSqlDatabase::removeDatabase("qlpowerdbnew");
  delete newdb;

  QSqlQuery query(*db);

  query.prepare("ATTACH DATABASE '" + filename + "' AS newdb ");
  QUERY_EXEC(;);

  query.exec("INSERT INTO wpts_eElectric SELECT * FROM newdb.wpts_eElectric");
  query.exec("INSERT INTO images SELECT * FROM newdb.images");
  query.exec("INSERT INTO image2wpt SELECT * FROM newdb.image2wpt");
  query.exec("INSERT INTO networks SELECT * FROM newdb.networks WHERE newdb.networks.key != 'unassigned_power_lines'");
  query.exec("INSERT INTO lines SELECT * FROM newdb.lines");
  query.prepare("DETACH DATABASE newdb");
  QUERY_EXEC(;);
  
  if (changed) {
    emit CWptDB::self().sigChanged();
    emit CWptDB::self().sigModified();
  }

  emit sigModified();
  emit sigChanged();
}

// Save the memory database to the given file
void CPowerDB::save(const QString& filename) 
{
  //qDebug() << "CPowerDB::save " << filename;

  // Remove an old file by the same name (the UI already warned the user that this would happen)
  QFileInfo fileInfo(filename);
  if (fileInfo.exists()) {
    QFile::remove(filename);
  }

  QSqlDatabase* newdb = createDB(filename); // Create table structure in the new DB
  newdb->close();
  QSqlDatabase::removeDatabase("qlpowerdb_" + filename);
  delete newdb;

  QSqlQuery query(*db);

  query.prepare("ATTACH DATABASE '" + filename + "' AS newdb ");
  QUERY_EXEC(;);

  query.prepare("INSERT INTO newdb.wpts SELECT * FROM wpts");
  QUERY_EXEC(;);
  query.exec("INSERT INTO newdb.wpts_eElectric SELECT * FROM wpts_eElectric");
  query.exec("INSERT INTO newdb.images SELECT * FROM images");
  query.exec("INSERT INTO newdb.image2wpt SELECT * FROM image2wpt");
  query.exec("INSERT INTO newdb.networks SELECT * FROM networks WHERE key != 'unassigned_power_lines'");
  query.exec("INSERT INTO newdb.lines SELECT * FROM lines");
  query.prepare("DETACH DATABASE newdb");
  QUERY_EXEC(;);
}

// Propagate any modifications of existing waypoints into the DB
void CPowerDB::slotWptDBKeyModified(const QString& key) {
  QSqlQuery query(*db);
  // Discover whether wpt with this key exists
  query.prepare("SELECT name FROM wpts WHERE key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return);

  if (query.next()) {
    //qDebug() << "Existing key '" << key << "' modified: " << query.value(0).toString();

    CWpt* wpt = CWptDB::self().getWptByKey(key);
    if (wpt == NULL) qDebug() << "NULL key in slotWptDBKeyModified()";
    
    QSqlQuery query2(*db);
    query2.prepare("UPDATE wpts SET sticky=:sticky, timestamp=:timestamp, icon=:icon, "
                                   "name=:name, comment=:comment,lat=:lat,lon=:lon, "
                                   "altitude=:altitude, proximity=:proximity "
                                   "WHERE key=:key");
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
    //addWpt(CWptDB::self().getWptByKey(key)); //Doesn't work because the wpt hasn't been added to CWptDB yet (NULL key is returned)
    //qDebug() << "slotWptDBKeyModified called with unknown key!";
  }

  // Tell CMainWindow that the DB was modified so that it can be saved on exit
  // emit sigModified(); // Not necessary!
} // slotWptDBKeyModified()

void CPowerDB::slotWptDBChanged()
{
  qDebug() << "CPowerDB::slotWptDBChanged()";

  QSqlQuery query(*db);
  QSet<QString> setDB;

  // Find out which waypoints we already have in the DB
  query.prepare("SELECT key FROM wpts");
  QUERY_EXEC(return);

  while (query.next())
    setDB.insert(query.value(0).toString());

  //qDebug() << "Existing waypoints in DB: " << setDB;

  // Find out which new keys have appeared in CWptDB
  QMap<QString,CWpt*> keysWptDB = CWptDB::self().getWpts();
  QSet<QString> setWptDB;

  for (QMap<QString,CWpt*>::const_iterator i = keysWptDB.constBegin(); i != keysWptDB.constEnd(); i++) 
    setWptDB.insert(i.key());

  // Add waypoints that are new in the CWptDB
  QSet<QString> setNew = setWptDB - setDB;

  for (QSet<QString>::const_iterator i = setNew.constBegin(); i != setNew.constEnd(); i++) {
    addWpt(keysWptDB[*i]);
    //setDB.insert(*i); // Otherwise it will get deleted again in the next step :-(
  }
  
  // Delete waypoints which have been deleted from CWptDB
  QSet<QString> setDel = setDB - setWptDB;

  for (QSet<QString>::const_iterator i = setDel.constBegin(); i != setDel.constEnd(); i++)
    delWpt(*i);
  
  // Tell CMainWindow that the DB was modified so that it can be saved on exit
  // emit sigModified(); // Not necessary!
}

void CPowerDB::addWpt(CWpt* wpt) {
  if (wpt == NULL) {
    qDebug() << "addWpt() called with NULL key";
    return;
  }

  QSqlQuery query(*db);

  //qDebug() << "Adding waypoint: " << wpt->getInfo();
  query.prepare("INSERT INTO wpts (key, sticky, timestamp, icon, name, comment, lat, lon, altitude, proximity) "\
                "VALUES(:key, :sticky, :timestamp, :icon, :name, :comment, :lat, :lon, :altitude, :proximity)");
  query.bindValue(":key", wpt->getKey());
  query.bindValue(":sticky", wpt->sticky);
  query.bindValue(":timestamp", wpt->getTimestamp());
  query.bindValue(":icon", wpt->getIconString());
  query.bindValue(":name", wpt->getName());
  query.bindValue(":comment", wpt->getComment());
  query.bindValue(":lat", wpt->lat);
  query.bindValue(":lon", wpt->lon);
  query.bindValue(":altitude", wpt->ele);
  query.bindValue(":proximity", wpt->prx);
  QUERY_EXEC(return);

  // TODO: Make provision for duplicate primary key errors?
  // TODO: We should insert the rest of the waypoint data, too
} // addWpt()

void CPowerDB::delWpt(const QString& key) {
  QSqlQuery query(*db);

  qDebug() << "Deleting waypoint: " << key;
  query.prepare("DELETE FROM wpts WHERE key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return);
  query.prepare("DELETE FROM wpts_eElectric WHERE wpt_key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return);
  query.prepare("DELETE FROM image2wpt WHERE wpt_key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return);
  query.prepare("DELETE FROM lines WHERE (first_key=:key) OR (second_key=:key_again)");
  query.bindValue(":key", key);
  query.bindValue(":key_again", key);
  QUERY_EXEC(return);
} // delWpt()

const CPowerDB::wpt_eElectric CPowerDB::getElectricData(const QString& key) const 
{
  QSqlQuery query(*db);
  
  query.prepare("SELECT consumers,consumers1,consumers2,consumers3,"
                "load,totalload,loadphase1,loadphase2,loadphase3,"
                "resistance,totalresistance,voltage,powerfactor "
                "FROM wpts_eElectric WHERE wpt_key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return(wpt_eElectric()););

  struct wpt_eElectric result;

  if (query.next()) {
    result.consumers = query.value(0).toDouble();
    result.consumers1 = query.value(1).toDouble();
    result.consumers2 = query.value(2).toDouble();
    result.consumers3 = query.value(3).toDouble();
    result.load = query.value(4).toDouble();
    result.totalload = query.value(5).toDouble();
    result.loadphase1 = query.value(6).toDouble();
    result.loadphase2 = query.value(7).toDouble();
    result.loadphase3 = query.value(8).toDouble();
    result.resistance = query.value(9).toDouble();
    result.totalresistance = query.value(10).toDouble();
    if (query.value(11).toDouble() > 0.01) // Is always ZERO in some buggy situations!!!
        result.voltage = query.value(11).toDouble();
    if (query.value(12).toDouble() > 0.01) // Is always ZERO in some buggy situations!!!
        result.powerfactor = query.value(12).toDouble();
    result.is_new = false;
  } else {
    // Waypoint has no electric data associated yet
    result.consumers = 0.0;
    result.consumers1 = 0.0;
    result.consumers2 = 0.0;
    result.consumers3 = 0.0;
    result.load = 0.0;
    result.totalload = 0.0;
    result.loadphase1 = 0.0;
    result.loadphase2 = 0.0;
    result.loadphase3 = 0.0;
    result.resistance = 0.0;
    result.totalresistance = 0.0;
    result.voltage = 230.0;
    result.powerfactor = 0.75;
    result.is_new = true;
  }

  return result;
} // getElectricData()

const QString CPowerDB::getElectricInfo(const QString& key) const {
    wpt_eElectric data = getElectricData(key);
    QString result;
    result = (data.consumers == 1 ? tr("%1 consumer") : tr("%1 consumers")).arg(data.consumers) +
            (data.load > 0.01 ? tr(" plus %1W load").arg(data.load,0,'f',0) : "") +
            "\n" + tr("%1V voltage").arg(data.voltage,0,'f',0) +
            //tr(", %1Ohm resistance").arg(data.resistance,0,'f',0) +
            /*"\n"*/ ", " + tr("%1W total load").arg(data.totalload,0,'f',0); //+
            //tr(", %1Ohm total resistance").arg(data.totalresistance,0,'f',0);
    return result;
}

const bool CPowerDB::hasElectricData() {
    QSqlQuery query(*db);

    query.prepare("SELECT consumers FROM wpts_eElectric");
    QUERY_EXEC(return false;);

    if (query.next())
        return true;
    else
        return false;
}

void CPowerDB::setElectricData(const QString& key, const CPowerDB::wpt_eElectric& e)
{
  QSqlQuery query(*db);
  
  // Is there any old data already in the table?
  query.prepare("SELECT consumers FROM wpts_eElectric WHERE wpt_key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return);
  
  if (!query.next()) {
    // Insert new data
    //qDebug() << "Inserting new record into wpts_eElectric=" << key << ", voltage=" << e.voltage <<
    //           ", pf=" << e.powerfactor;
    query.prepare("INSERT INTO wpts_eElectric (wpt_key,consumers,consumers1,consumers2,consumers3,"
                  "load,totalload,loadphase1,loadphase2,loadphase3,"
                  "resistance,totalresistance,voltage,powerfactor) "
                  "VALUES(:key, :consumers, :consumers1, :consumers2, :consumers3,"
                  ":load, :totalload, :loadphase1, :loadphase2, :loadphase3,"
                  ":resistance, :totalresistance, :voltage, :powerfactor)");
  } else {
    // Update existing data
    //qDebug() << "Updating existing record into wpts_eElectric=" << key << ", voltage=" << e.voltage <<
    //            ", pf=" << e.powerfactor;
    query.prepare("UPDATE wpts_eElectric "
                  "SET consumers=:consumers,consumers1=:consumers1,consumers2=:consumers2,consumers3=:consumers3,"
                  "load=:load,totalload=:totalload,"
                  "loadphase1=:loadphase1,loadphase2=:loadphase2,loadphase3=:loadphase3,"
                  "resistance=:resistance,totalresistance=:totalresistance,"
                  "voltage=:voltage,powerfactor=:powerfactor "
                  "WHERE wpt_key=:key");
  }

  query.bindValue(":key", key);
  query.bindValue(":consumers", e.consumers);
  query.bindValue(":consumers1", e.consumers1);
  query.bindValue(":consumers2", e.consumers2);
  query.bindValue(":consumers3", e.consumers3);
  query.bindValue(":load", e.load);
  query.bindValue(":totalload", e.totalload);
  query.bindValue(":loadphase1", e.loadphase1);
  query.bindValue(":loadphase2", e.loadphase2);
  query.bindValue(":loadphase3", e.loadphase3);
  query.bindValue(":resistance", e.resistance);
  query.bindValue(":totalresistance", e.totalresistance);
  query.bindValue(":voltage", e.voltage);
  query.bindValue(":powerfactor", e.powerfactor);
  QUERY_EXEC(return);

  emit sigChanged();
  emit sigModified();
} // setElectricData()

const bool CPowerDB::containsPowerLine(const QString& key) const {
    QSqlQuery query(*db);
    
    query.prepare("SELECT first_key FROM lines WHERE key=:key");
    query.bindValue(":key", key);
    QUERY_EXEC(return false);
    
    return query.next();
}

CPowerLine* CPowerDB::getPowerLineByKey(const QString& key) 
{
  QSqlQuery query(*db);
  
  query.prepare("SELECT timestamp,name,comment,icon,nw_key,first_key,second_key,"
                "cross_section,phases,length,conductivity,resistance,current,voltage_drop,"
                "highlighted "
                "FROM lines WHERE key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return NULL;);

  CPowerLine* result = new CPowerLine(this);
  
  if (query.next()) {
    //result->setTimestamp(query.value(0).toTimestamp());
    result->setKey(key);
    result->setName(query.value(1).toString());
    result->setComment(query.value(2).toString());
    result->setIcon(query.value(3).toString());
    result->keyNetwork= query.value(4).toString();
    result->keyFirst  = query.value(5).toString();
    result->keySecond = query.value(6).toString();
    result->setCrossSection(query.value(7).toDouble());
    result->setPhases(query.value(8).toUInt());
    result->setLength(query.value(9).toDouble());
    result->setConductivity(query.value(10).toDouble());
    result->setResistance(query.value(11).toDouble());
    result->setCurrent(query.value(12).toDouble());
    result->setDrop(query.value(13).toDouble());
    result->highlighted=query.value(14).toUInt();
  } else {
    // No such power line
    result->setKey(key);
    result->setName("");
    result->setComment("");
    result->setIcon("");
    result->keyFirst  = "";
    result->keySecond = "";
    result->setCrossSection(0.0);
    result->setPhases(7);
    result->setLength(0.0);
    result->setConductivity(30.0);
    result->setResistance(0.0);
    result->setCurrent(0.0);
    result->setDrop(0.0);
    result->highlighted=0;
  }
  
  return result;
} // getPowerLineByKey()

const QStringList CPowerDB::getPowerLines() const
{
  QSqlQuery query(*db);
  QStringList result;
  
  query.prepare("SELECT key FROM lines");
  QUERY_EXEC(return result;);

  while (query.next()) 
    result.append(query.value(0).toString());
  
  return result;
} // getPowerLines()

const bool CPowerDB::hasPowerLines()
{
    return (getPowerLines().size() > 0);
}
   
CPowerLine* CPowerDB::newPowerLine(const QString& first, const QString& second, const bool silent)
{
    CPowerLine * l  = new CPowerLine(this);
    l->setName("New Power line");
    l->keyFirst  = first;
    l->keySecond = second;
    l->setLength(l->getDistance());

    // Find out if a network is selected, then add the new power line to it
    QString nw_key = getToolWidget()->getFirstSelectedNetwork();

    // Set default data of this network for the power line
    // and for the two waypoints of the power line (if no previous data exists)
    if (nw_key != "")
    {
        CPowerNW * nw = CPowerDB::self().getPowerNWByKey(nw_key);
        l->setConductivity(nw->conductivity);

        wpt_eElectric wpt_electric = getElectricData(first);
        if (wpt_electric.is_new) {
            wpt_electric.powerfactor = nw->powerfactor;
            setElectricData(first, wpt_electric);
        }
        wpt_electric = getElectricData(second);
        if (wpt_electric.is_new) {
            wpt_electric.powerfactor = nw->powerfactor;
            setElectricData(second, wpt_electric);
        }
    }
    else
    {
        nw_key = "unassigned_power_lines";
    }

    if (!silent) {
        CDlgEditPowerLine dlg(*l,theMainWindow->getCanvas());
        if(dlg.exec() == QDialog::Rejected)
        {
            delete l;
            return NULL;
        }
    }

    QSqlQuery query(*db);
    
    query.prepare("INSERT INTO lines (key, timestamp, name, comment, icon, nw_key, first_key, second_key, "
                  "cross_section, phases, length, conductivity, resistance, current, voltage_drop) "
                  "VALUES(:key, :timestamp, :name, :comment, :icon, :nw, :first, :second, "
                  ":section, :phases, :length, :conductivity, :resistance, :current, :voltage_drop)");
    query.bindValue(":key", l->getKey());
    query.bindValue(":timestamp", l->getTimestamp());
    query.bindValue(":name", l->getName());
    query.bindValue(":comment", l->getComment());
    query.bindValue(":icon", l->getIconString());
    query.bindValue(":nw", nw_key);
    query.bindValue(":first", l->keyFirst);
    query.bindValue(":second", l->keySecond);
    query.bindValue(":section", l->getCrossSection());
    query.bindValue(":phases", l->getPhases());
    query.bindValue(":length", l->getLength());
    query.bindValue(":conductivity", l->getConductivity());
    query.bindValue(":resistance", l->getResistance());
    query.bindValue(":current", l->getCurrent());
    query.bindValue(":voltage_drop", l->getDrop());
    QUERY_EXEC(return l);
    
    if (getToolWidget()->checkLoop(l->keyFirst, nw_key, l->getKey(), l->keySecond))
    {
        QMessageBox::warning(0, tr("Closed loop"), tr("Distribution networks with closed loops are not allowed"), 
            QMessageBox::Abort, QMessageBox::Abort);
        delPowerLine(l->getKey()); 
    }
    
    emit sigChanged();
    emit sigModified();

    return l;
} //newPowerLine()

void CPowerDB::delPowerLine(const QString& key) {
  QSqlQuery query(*db);

  qDebug() << "Deleting power line: " << key;
  query.prepare("DELETE FROM lines WHERE key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return);
} // delPowerLine()

void CPowerDB::delPowerLines(const QStringList& keys) {
    //undoStack->beginMacro("delPowerLines");
    foreach(QString key,keys)
    {
        delPowerLine(key);
    }
    //undoStack->endMacro();
    if(!keys.isEmpty())
    {
        emit sigChanged();
        emit sigModified();
    }  
} // delPowerLines()

void CPowerDB::splitPowerLine(const QString& key, const QString& wpt_key)
{
    CPowerLine* old_line = getPowerLineByKey(key);
    CPowerLine* line1 = newPowerLine(old_line->keyFirst, wpt_key, true);
    line1->keyNetwork = old_line->keyNetwork;
    line1->setCrossSection(old_line->getCrossSection());
    line1->setPhases(old_line->getPhases());
    line1->setConductivity(old_line->getConductivity());
    setPowerLineData(*line1);
    delPowerLine(key); // Avoid creating a loop in the network
    CPowerLine* line2 = newPowerLine(wpt_key, old_line->keySecond, true);
    line2->keyNetwork = line1->keyNetwork;
    line2->setCrossSection(line1->getCrossSection());
    line2->setPhases(line1->getPhases());
    line2->setConductivity(line1->getConductivity());
    setPowerLineData(*line2);

    // Set icon for intermediate waypoint
    QStringList wpts;
    wpts.push_back(wpt_key);
    CWptDB::self().setIcon(wpts, "Pin, Blue");
}

void CPowerDB::addPowerLine(CPowerLine * l, CPowerNW * nw, bool silent)
{
    if(containsPowerLine(l->getKey()))
    {
        delPowerLine(l->getKey());
    }
    newPowerLine(l->keyFirst, l->keySecond);
    setPowerLineData(*l);

    if(!silent)
    {
        emit sigChanged();
        emit sigModified();
    }
}

void CPowerDB::setPowerLineData(CPowerLine& l)
{
    //qDebug() << "CPowerDB::setPowerLineData() for " << l.getName();
    QSqlQuery query(*db);
    query.prepare("UPDATE lines SET timestamp=:timestamp, name=:name, comment=:comment, icon=:icon,"
                  "nw_key=:nw_key,"
                  "cross_section=:csection, phases=:phases, length=:length, conductivity=:conductivity, "
                  "resistance=:resistance, current=:current, voltage_drop=:voltage_drop "
                  "WHERE key=:key");
    query.bindValue(":timestamp", l.getTimestamp());
    query.bindValue(":name", l.getName());
    query.bindValue(":comment", l.getComment());
    query.bindValue(":icon", l.getIconString());
    query.bindValue(":key", l.getKey());
    query.bindValue(":nw_key", l.keyNetwork);
    query.bindValue(":csection", l.getCrossSection());
    query.bindValue(":phases", l.getPhases());
    query.bindValue(":length", l.getLength());
    query.bindValue(":conductivity", l.getConductivity());
    query.bindValue(":resistance", l.getResistance());
    query.bindValue(":current", l.getCurrent());
    query.bindValue(":voltage_drop", l.getDrop());
    QUERY_EXEC(return);
    //qDebug() << "Set current to " << l.getCurrent() << " in DB";

    emit sigChanged();
    emit sigModified();
}

void CPowerDB::unHighlightPowerLine(const QString& key) {

    //qDebug() << "CPowerDB::unHighlightPowerLine() for " << key;

    QSqlQuery query(*db);

    query.prepare("UPDATE lines SET highlighted=0 WHERE (highlighted=2) AND (key=:key)");
    query.bindValue(":key", key);
    QUERY_EXEC(return);
    query.prepare("UPDATE lines SET highlighted=1 WHERE (highlighted=3) AND (key=:key)");
    query.bindValue(":key", key);
    QUERY_EXEC(return);

    emit sigChanged(true);

}

void CPowerDB::unHighlightPowerLines() {
    QSqlQuery query(*db);

    query.prepare("UPDATE lines SET highlighted=0 WHERE (highlighted=2)");
    QUERY_EXEC(return);
    query.prepare("UPDATE lines SET highlighted=1 WHERE (highlighted=3)");
    QUERY_EXEC(return);

    emit sigChanged(true);
}

unsigned CPowerDB::isHighlightedPowerLine(const QString& key) {
    QSqlQuery query(*db);
    
    query.prepare("SELECT highlighted FROM lines WHERE key=:key");
    query.bindValue(":key", key);
    QUERY_EXEC(return false);
    
    if (query.next())
    {
      return query.value(0).toUInt();
    } else {
      return false;
    }
}

/*
  // Note: This should return ALL highlighted power lines, not just the first
CPowerLine* CPowerDB::highlightedPowerLine() {
    QSqlQuery query(*db);
    
    query.prepare("SELECT key FROM lines WHERE highlighted == 2 or highlighted == 3");
    QUERY_EXEC(return NULL);
    
    if (query.next())
    {
      return (getPowerLineByKey(query.value(0).toString()));
    } else {
      return NULL;
    }
}
*/

const bool CPowerDB::containsPowerNW(const QString& key) const {
    QSqlQuery query(*db);
    
    query.prepare("SELECT powerhouse_key FROM networks WHERE key=:key");
    query.bindValue(":key", key);
    QUERY_EXEC(return false);
    
    return query.next();
}

CPowerNW* CPowerDB::getPowerNWByKey(const QString& key) 
{
  QSqlQuery query(*db);
  
  query.prepare("SELECT timestamp,name,comment,icon,"
                "powerhouse_key,powerhouse_voltage,watts_per_consumer,line_conductivity,"
                "power_factor,ratedvoltage,minimalvoltage "
                "FROM networks WHERE key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return NULL;);

  CPowerNW* result = new CPowerNW(this);
  
  if (query.next()) {
    result->setKey(key);
    result->setName(query.value(1).toString());
    result->setComment(query.value(2).toString());
    result->setIcon(query.value(3).toString());
    result->ph      = query.value(4).toString();
    result->voltage = query.value(5).toDouble();
    result->watts   = query.value(6).toDouble();
    result->conductivity = query.value(7).toDouble();
    result->powerfactor = query.value(8).toDouble();
    if (query.value(9).toDouble() > 0.01) // Is always ZERO in some buggy situations!!!
        result->ratedVoltage = query.value(9).toDouble();
    result->minVoltage = query.value(10).toDouble();
  } else {
      qDebug() << "No network, using default values";
    // No such network
    result->setKey(key);
    result->setName("");
    result->setComment("");
    result->setIcon("");
    result->ph      = "";
    result->voltage = 240.0;
    result->watts   = 60.0;
    result->conductivity = 30.0;
    result->powerfactor = 0.75;
    result->ratedVoltage = 220;
    result->minVoltage = 200;
  }
  return result;
} // getPowerNWByKey

CPowerNW* CPowerDB::getPowerNWByName(const QString& name)
{
  //qDebug() << "CPowerDB::getPowerNWByName() for " << name;
  QSqlQuery query(*db);

  query.prepare("SELECT key FROM networks WHERE name=:name");
  query.bindValue(":name", name);
  QUERY_EXEC(return NULL;);

  if (query.next()) {
      qDebug() << "CPowerDB::getPowerNWByName() query successful, result " << query.value(0).toString();
      // TODO: Check for multiple results?
      return getPowerNWByKey(query.value(0).toString());
  } else {
      return NULL;
  }
} // getPowerNWByName

CPowerNW* CPowerDB::getPowerNWFromWpt(const QString& wpt_key)
{
    QSqlQuery query(*db);

    query.prepare("SELECT nw_key FROM lines WHERE first_key = :key OR second_key = :key");
    query.bindValue(":key", wpt_key);
    QUERY_EXEC(return NULL;);

    if (query.next()) {
        //qDebug() << "CPowerDB::getPowerNWByWpt() query successful, result " << query.value(0).toString();
        return getPowerNWByKey(query.value(0).toString());
    } else {
        return NULL;
    }
}

const QStringList CPowerDB::getPowerNWs() const
{
  QSqlQuery query(*db);
  
  query.prepare("SELECT key FROM networks");
  QUERY_EXEC(return QStringList(););

  QStringList result;
  while (query.next()) 
    result.append(query.value(0).toString());
  
  return result;
} // getPowerNWs()

const bool CPowerDB::hasPowerNWs()
{
    return (getPowerNWs().size() > 0);
}

CPowerNW* CPowerDB::newPowerNW(const QString& ph_key)
{
    CPowerNW * nw  = new CPowerNW(this);
    nw->setName("New Network");
    nw->ph  = ph_key;

    CDlgEditPowerNW dlg(*nw,theMainWindow->getCanvas());
    if(dlg.exec() == QDialog::Rejected)
    {
        delete nw;
        return NULL;
    }

    QSqlQuery query(*db);
    
    query.prepare("INSERT INTO networks (key, timestamp, name, comment, icon, "
                  "powerhouse_key, powerhouse_voltage, watts_per_consumer, line_conductivity,"
                  "power_factor, ratedvoltage, minimalvoltage) "
                  "VALUES(:key, :timestamp, :name, :comment, :icon, "
                  ":ph, :voltage, :watts, :conductivity, :pf, :ratedvoltage, :minimalvoltage)");
    query.bindValue(":key", nw->getKey());
    query.bindValue(":timestamp", nw->getTimestamp());
    query.bindValue(":name", nw->getName());
    query.bindValue(":comment", nw->getComment());
    query.bindValue(":icon", nw->getIconString());
    query.bindValue(":ph", nw->ph);
    query.bindValue(":voltage", nw->voltage);
    query.bindValue(":watts", nw->watts);
    query.bindValue(":conductivity", nw->conductivity);
    query.bindValue(":pf", nw->powerfactor);
    query.bindValue(":ratedvoltage", nw->ratedVoltage);
    query.bindValue(":minimalvoltage", nw->minVoltage);
    QUERY_EXEC(return nw);

    // propagate voltage to powerhouse waypoint
    wpt_eElectric ph_electric = getElectricData(nw->ph);
    ph_electric.voltage = nw->voltage;
    if (ph_electric.is_new)
        ph_electric.powerfactor = nw->powerfactor;
    setElectricData(nw->ph, ph_electric);

    highlightPowerNW(nw->getKey()); // So that new lines get added to this NW!
    emit sigChanged();
    emit sigModified();

    return nw;
}

void CPowerDB::delPowerNW(const QString& key) {
  if (key == "unassigned_power_lines") return; // This is not deleteable
  
  //undoStack->push(new CPowerUndoCommandDelete(this,key,silent));

  QSqlQuery query(*db);

  qDebug() << "Deleting power network: " << key;
  query.prepare("DELETE FROM networks WHERE key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return);

  // Clear nw key from all related lines
  query.prepare("UPDATE lines SET nw_key='unassigned_power_lines' WHERE nw_key=:key");
  query.bindValue(":key", key);
  QUERY_EXEC(return);
} // delPowerNW()

void CPowerDB::delPowerNWs(const QStringList& keys) {
    //undoStack->beginMacro("delPowerNWs");
    foreach(QString key,keys)
    {
        delPowerNW(key);
    }
    //undoStack->endMacro();
    if(!keys.isEmpty())
    {
        emit sigChanged();
        emit sigModified();
    }
} // delPowerNWs()

void CPowerDB::addPowerNW(CPowerNW * nw, bool silent)
{
    if(containsPowerNW(nw->getKey()))
    {
        delPowerNW(nw->getKey());
    }
    
    /*if(nw->getName().isEmpty())
    {
        nw->setName(tr("PowerNW%1").arg(cnt++));
    }*/
    newPowerNW(nw->ph);
    setPowerNWData(*nw);

    if(!silent)
    {
        emit sigChanged();
        emit sigModified();
    }
}


void CPowerDB::setPowerNWData(CPowerNW& nw)
{
    QSqlQuery query(*db);
    query.prepare("UPDATE networks SET timestamp=:timestamp,name=:name,comment=:comment,icon=:icon,"
                  "powerhouse_key=:ph,powerhouse_voltage=:voltage,watts_per_consumer=:watts,"
                  "line_conductivity=:conductivity,power_factor=:pf,ratedvoltage=:rv,minimalvoltage=:mv "
                  "WHERE key=:key");
    query.bindValue(":key", nw.getKey());
    query.bindValue(":timestamp", nw.getTimestamp());
    query.bindValue(":name", nw.getName());
    query.bindValue(":comment", nw.getComment());
    query.bindValue(":icon", nw.getIconString());
    query.bindValue(":ph", nw.ph);
    query.bindValue(":voltage", nw.voltage);
    query.bindValue(":watts", nw.watts);
    query.bindValue(":conductivity", nw.conductivity);
    query.bindValue(":pf", nw.powerfactor);
    query.bindValue(":rv",nw.ratedVoltage);
    query.bindValue(":mv", nw.minVoltage);
    QUERY_EXEC(return);
    
    emit sigChanged(); // notify the CPowerToolWidget that something has changed
}

/*void CPowerDB::setHighlightPowerNW(const QString& key, bool highlight) {

}*/

bool CPowerDB::isHighlightedPowerNW(const QString& key) {
    QSqlQuery query(*db);
    
    query.prepare("SELECT highlighted FROM networks WHERE key=:key");
    query.bindValue(":key", key);
    QUERY_EXEC(return false);
    
    if (query.next())
    {
      return query.value(0).toBool();
    } else {
      return false;
    }
}

CPowerNW* CPowerDB::highlightedPowerNW() {
    QSqlQuery query(*db);
    
    query.prepare("SELECT key FROM networks WHERE highlighted != 0");
    QUERY_EXEC(return NULL);
    
    if (query.next())
    {
      return (getPowerNWByKey(query.value(0).toString()));
    } else {
      return NULL;
    }
}

void CPowerDB::highlightPowerNW(const QString& key)
{
    qDebug() << "highlightPowerNW " << key;
    if(CPowerDB::self().isHighlightedPowerNW(key))
    {
        return;
    }

    QSqlQuery query(*db);

    // Mark the network as highlighted
    query.prepare("UPDATE networks SET highlighted=0 WHERE key!=:key");
    query.bindValue(":key", key);
    QUERY_EXEC(return);
    query.prepare("UPDATE networks SET highlighted=1 WHERE key=:key");
    query.bindValue(":key", key);
    QUERY_EXEC(return);
     
    // Highlight all the lines belonging to this network (and clear all other highlighting)
    query.prepare("UPDATE lines SET highlighted=0 WHERE nw_key!=:key");
    query.bindValue(":key", key);
    QUERY_EXEC(return);
    query.prepare("UPDATE lines SET highlighted=1 WHERE nw_key=:key");
    query.bindValue(":key", key);
    QUERY_EXEC(return);

    //emit sigHighlightPowerNW(key);
    //emit sigHighlightPowerNW(0);

    emit sigChanged(true);

}

void CPowerDB::highlightPowerLine(const QString& key, const bool single)
{
    qDebug() << "CPowerDB::highlightPowerLine() for " << key;

    if(CPowerDB::self().isHighlightedPowerLine(key) & 2)
    {
        return;
    }

    QSqlQuery query(*db);

    if (single) {
        // Unhighlight other highlighted lines
        query.prepare("UPDATE lines SET highlighted=0 WHERE (highlighted=2) AND (key!=:key)");
        query.bindValue(":key", key);
        QUERY_EXEC(return);
        query.prepare("UPDATE lines SET highlighted=1 WHERE (highlighted=3) AND (key!=:key)");
        query.bindValue(":key", key);
        QUERY_EXEC(return);
    }
    query.prepare("UPDATE lines SET highlighted=2 WHERE (highlighted=0) AND (key=:key)");
    query.bindValue(":key", key);
    QUERY_EXEC(return);
    query.prepare("UPDATE lines SET highlighted=3 WHERE (highlighted=1) AND (key=:key)");
    query.bindValue(":key", key);
    QUERY_EXEC(return);

    //emit sigHighlightPowerLine(key); // not required at this point
    //emit sigHighlightPowerLine(0); // if key doesn't exist

    emit sigChanged(true);

}

void CPowerDB::highlightPowerLines(const QStringList& keys)
{
    unHighlightPowerLines();

    foreach (QString key, keys) {
        highlightPowerLine(key, false);
    }
}

const QList<QPointF> getIntersectionPoints(const QRect& r, const QLine& l)
{
    QList<QPoint> points;
    points << r.topRight() << r.bottomRight() << r.bottomLeft() << r.topLeft();
    QList<QPointF> result;
    QPoint p1 = r.topLeft();

    for (QList<QPoint>::const_iterator pt = points.begin(); pt != points.end(); pt++) {
        QLineF line(p1, *pt);
        QPointF p_inter;
        if (line.intersect(l, &p_inter) == QLineF::BoundedIntersection)
            result << p_inter;
        p1 = *pt;
    }

    return result;
}

void CPowerDB::drawElectricText(QPainter& p, const CPowerLine* l, const QPoint &middle, const double angle)
{    
    QString str;
    str += trUtf8("%1m, %2mm²").arg(l->getLength(),0,'f',0).arg(l->getCrossSection(),0,'f',0);
    str += QString(", ") + (l->getPhases() & 1 ? "1" : "-") + (l->getPhases() & 2 ? "2" : "-") + (l->getPhases() & 4 ? "3" : "-") + tr("ph");

    p.save();
    p.translate(middle);
    double textAngle = angle + 90.0; // display at 90° angle to line
    if (textAngle > 90.0) textAngle -= 180.0;
    p.rotate(textAngle);

    QFont        f   = QFont("Arial",6);
    QFontMetrics fm(f);
    QRect        r1  = fm.boundingRect(QRect(0,0,300,0), Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, str);
    r1.moveTopLeft(QPoint(-r1.width()/2, -r1.height()/2));

    QRect r2 = r1;
    r2.setWidth(r1.width() + 4);
    r2.moveLeft(r1.left() - 2);
    r2.setHeight(r1.height() + 4);
    r2.moveTop(r1.top() - 2);

    p.setPen(QPen(Qt::black, 1));
    p.setBrush(CCanvas::brushBackWhite);
    p.drawRoundedRect(r2,2,2);

    p.setFont(f);
    p.drawText(r1, Qt::AlignJustify|Qt::AlignTop|Qt::TextWordWrap,str);
    p.restore();
}

void CPowerDB::drawLine(const QLine& qline, QPainter &p)
{
    QPoint ptt = qline.p1() - qline.p2();
    if(ptt.manhattanLength() < 5) return;

    p.drawLine(qline);
}

void CPowerDB::draw(QPainter& p, const QRect& rect, bool& needsRedraw)
{
    QStringList lines                 = CPowerDB::self().getPowerLines();
    QStringList::iterator lit         = lines.begin();
    QList< CPowerLine* > highlighted;

    while(lit != lines.end())
    {
        CPowerLine * l = CPowerDB::self().getPowerLineByKey(*lit);
        
        if(l->m_hide)
        {
            ++lit;
            continue;
        }

        QPoint middle;
        double angle;
        QLine qline = l->getLine(middle, angle);
        bool p1visible = rect.contains(qline.p1());
        bool p2visible = rect.contains(qline.p2());

        // Quick intersection test
        if (p1visible && p2visible) {
            // Line completely visible
        } else {
            // Exact intersection test
            QList<QPointF> points = getIntersectionPoints(rect, qline);

            if (points.empty()) {
                // Line not visible at all
                ++lit;
                continue;
            } else if (points.size() == 1) {
                if (p1visible)
                    points << qline.p1();
                else if (p2visible)
                    points << qline.p2();
                else {
                    // line touches at a corner
                    ++lit;
                    continue;
                }
            }

            // Line only partially visible
            QLineF visibleSegment(points.front(), points.back());
            if (visibleSegment.length() > 50.0)
                middle = ((visibleSegment.p1() + visibleSegment.p2())/2.0).toPoint();
        }

        if (l->highlighted > 0)
        {
            // store highlighted power line to draw it later
            // it must be drawn above all other power lines
            highlighted << l;
        }
        else
        {
            // draw normal power line
            QPen pen1(Qt::white,3);
            pen1.setCapStyle(Qt::RoundCap);
            pen1.setJoinStyle(Qt::RoundJoin);

            QPen pen2(l->getColor(),1);
            pen2.setCapStyle(Qt::RoundCap);
            pen2.setJoinStyle(Qt::RoundJoin);

            p.setPen(pen1);
            drawLine(qline, p);
            p.setPen(pen2);
            drawLine(qline, p);

            // Draw electric info on lines
            if (printView & 2)
            {
                drawElectricText(p, l, middle, angle);
            }
        }

        ++lit;
    }

    // if there are highlighted lines, draw them
    QList< CPowerLine* >::iterator hlit         = highlighted.begin();
    
    while (hlit != highlighted.end())
    {        
        QPoint middle;
        double angle;
        QLine qline = (*hlit)->getLine(middle, angle);

        // draw skunk line
        QPen pen1(((*hlit)->highlighted == 1 ? QColor(255,255,255,128) : QColor(0,0,128,128)),6);
        pen1.setCapStyle(Qt::RoundCap);
        pen1.setJoinStyle(Qt::RoundJoin);

        QColor color = (*hlit)->getColor();
        color.setAlpha(128);
        QPen pen2(color,4);
        pen2.setCapStyle(Qt::RoundCap);
        pen2.setJoinStyle(Qt::RoundJoin);

        p.setPen(pen1);
        drawLine(qline, p);
        p.setPen(pen2);
        drawLine(qline, p);

        // Draw electric info on lines
        if (printView & 2)
        {
            drawElectricText(p, (*hlit), middle, angle);
        }
        
        ++hlit;
    }
} // draw()

void CPowerDB::togglePrintView()
{
    printView++;
    if (printView > 3) printView = 0;
}

