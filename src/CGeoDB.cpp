/**********************************************************************************************
    Copyright (C) 2010 Oliver Eichler oliver.eichler@gmx.de

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

#include "CGeoDB.h"
#include "config.h"
#include "WptIcons.h"

#include "CWptDB.h"
#include "CWpt.h"
#include "CTrackDB.h"
#include "CRouteDB.h"
#include "COverlayDB.h"
#include "CDlgSelGeoDBFolder.h"

#include "CQlb.h"

#include <QtGui>
#include <QSqlQuery>
#include <QSqlError>

class CGeoDBInternalEditLock
{
    public:
    CGeoDBInternalEditLock(CGeoDB * db) : db(db){db->isInternalEdit += 1;}
        ~CGeoDBInternalEditLock(){db->isInternalEdit -= 1;}
    private:
        CGeoDB * db;
};

CGeoDB::CGeoDB(QTabWidget * tb, QWidget * parent)
    : QWidget(parent)
    , tabbar(tb)
    , isInternalEdit(0)
{
    setupUi(this);
    setObjectName("GeoDB");

    tabbar->insertTab(eName,this, QIcon(":/icons/iconGeoDB16x16"),"");
    tabbar->setTabToolTip(tabbar->indexOf(this), tr("Manage your Geo Data Base"));

    itemWorkspace = new QTreeWidgetItem(treeWidget,eFolder);
    itemWorkspace->setText(eName, tr("Workspace"));
    itemWorkspace->setIcon(eName, QIcon(":/icons/iconGlobe16x16"));
    itemWorkspace->setToolTip(eName, tr("All items you see on the map."));
    itemWorkspace->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);

    itemWksWpt = new  QTreeWidgetItem(itemWorkspace, eTypFolder);
    itemWksWpt->setText(eName, tr("Waypoints"));
    itemWksWpt->setIcon(eName, QIcon(":/icons/iconWaypoint16x16"));
    itemWksWpt->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemWksWpt->setHidden(true);

    itemWksTrk = new  QTreeWidgetItem(itemWorkspace, eTypFolder);
    itemWksTrk->setText(eName, tr("Tracks"));
    itemWksTrk->setIcon(eName, QIcon(":/icons/iconTrack16x16"));
    itemWksTrk->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemWksTrk->setHidden(true);

    itemWksRte = new  QTreeWidgetItem(itemWorkspace, eTypFolder);
    itemWksRte->setText(eName, tr("Routes"));
    itemWksRte->setIcon(eName, QIcon(":/icons/iconRoute16x16"));
    itemWksRte->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemWksRte->setHidden(true);

    itemWksOvl = new  QTreeWidgetItem(itemWorkspace, eTypFolder);
    itemWksOvl->setText(eName, tr("Overlays"));
    itemWksOvl->setIcon(eName, QIcon(":/icons/iconOverlay16x16"));
    itemWksOvl->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemWksOvl->setHidden(true);

    itemPaperbin = new QTreeWidgetItem(treeWidget,eFolder);
    itemPaperbin->setText(eName, tr("Lost items"));
    itemPaperbin->setIcon(eName, QIcon(":/icons/iconDelete16x16"));
    itemPaperbin->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);

    itemDatabase = new QTreeWidgetItem(treeWidget,eFolder);
    itemDatabase->setText(eName, tr("Database"));
    itemDatabase->setIcon(eName, QIcon(":/icons/iconGeoDB16x16"));
    itemDatabase->setData(eName, eUserRoleDBKey, 1);
    itemDatabase->setFlags(itemWorkspace->flags() & ~Qt::ItemIsDragEnabled);
    itemDatabase->setToolTip(eName, tr("All your data grouped by folders."));

    db = QSqlDatabase::addDatabase("QSQLITE","qlandkarte");
    db.setDatabaseName(QDir::home().filePath(CONFIGDIR "qlgt.db"));
    db.open();

    QSqlQuery query(db);

    if(!query.exec("PRAGMA locking_mode=EXCLUSIVE")) {
      return;
    }

    if(!query.exec("PRAGMA temp_store=MEMORY")) {
      return;
    }

    if(!query.exec("PRAGMA default_cache_size=50")) {
      return;
    }

    if(!query.exec("PRAGMA page_size=8192")) {
      return;
    }


    if(!query.exec("SELECT version FROM versioninfo"))
    {
        initDB();
    }
    else if(query.next())
    {
        int version = query.value(0).toInt();
        if(version != DB_VERSION)
        {
            migrateDB(version);
        }
    }
    else
    {
        initDB();
    }

    contextMenuFolder = new QMenu(this);
    actEditDirComment = contextMenuFolder->addAction(QPixmap(":/icons/iconEdit16x16.png"),tr("Edit comment"),this,SLOT(slotEditDirComment()));
    actAddDir = contextMenuFolder->addAction(QPixmap(":/icons/iconAdd16x16.png"),tr("Add Folder"),this,SLOT(slotAddDir()));
    actDelDir = contextMenuFolder->addAction(QPixmap(":/icons/iconDelete16x16.png"),tr("Del. Folder"),this,SLOT(slotDelDir()));

    contextMenuWks = new QMenu(this);
    actAddToDB = contextMenuWks->addAction(QPixmap(":/icons/iconAdd16x16"), tr("Add to database..."), this, SLOT(slotAddItems()));

    setupTreeWidget();

    connect(treeWidget,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(slotContextMenu(const QPoint&)));
    connect(treeWidget,SIGNAL(itemExpanded(QTreeWidgetItem *)),this,SLOT(slotItemExpanded(QTreeWidgetItem *)));
    connect(treeWidget,SIGNAL(itemClicked(QTreeWidgetItem *, int)),this,SLOT(slotItemClicked(QTreeWidgetItem *, int)));
    connect(treeWidget,SIGNAL(itemChanged(QTreeWidgetItem *, int)),this,SLOT(slotItemChanged(QTreeWidgetItem *, int)));

    connect(&CWptDB::self(), SIGNAL(sigChanged()), this, SLOT(slotWptDBChanged()));
    connect(&CTrackDB::self(), SIGNAL(sigChanged()), this, SLOT(slotTrkDBChanged()));
    connect(&CRouteDB::self(), SIGNAL(sigChanged()), this, SLOT(slotRteDBChanged()));
    connect(&COverlayDB::self(), SIGNAL(sigChanged()), this, SLOT(slotOvlDBChanged()));

    slotWptDBChanged();
    slotTrkDBChanged();
    slotRteDBChanged();
    slotOvlDBChanged();

    itemWorkspace->setExpanded(true);
}

CGeoDB::~CGeoDB()
{

}


void CGeoDB::gainFocus()
{
    if(tabbar->currentWidget() != this)
    {
        tabbar->setCurrentWidget(this);
    }
}


void CGeoDB::initDB()
{
    qDebug() << "void CGeoDB::initDB()";
    QSqlQuery query(db);

    if(query.exec( "CREATE TABLE versioninfo ( version TEXT )"))
    {
        query.prepare( "INSERT INTO versioninfo (version) VALUES(:version)");
        query.bindValue(":version", DB_VERSION);
        if(!query.exec())
        {
            qDebug() << query.lastQuery();
            qDebug() << query.lastError();
        }
    }

    if(!query.exec( "CREATE TABLE folders ("
        "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "date           DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "icon           TEXT NOT NULL,"
        "name           TEXT NOT NULL,"
        "comment        TEXT"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }


    if(!query.exec( "CREATE TABLE items ("
        "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "type           INTEGER,"
        "key            TEXT NOT NULL,"
        "date           DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "icon           TEXT NOT NULL,"
        "name           TEXT NOT NULL,"
        "comment        TEXT,"
        "data           BLOB NOT NULL"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    if(!query.exec("INSERT INTO folders (icon, name, comment) VALUES ('', 'database', '')"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    if(!query.exec( "CREATE TABLE folder2folder ("
        "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "parent         INTEGER NOT NULL,"
        "child          INTEGER NOT NULL,"
        "FOREIGN KEY(parent) REFERENCES folders(id),"
        "FOREIGN KEY(child) REFERENCES folders(id)"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    if(!query.exec( "CREATE TABLE folder2item ("
        "id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "parent         INTEGER NOT NULL,"
        "child          INTEGER NOT NULL,"
        "FOREIGN KEY(parent) REFERENCES folders(id),"
        "FOREIGN KEY(child) REFERENCES items(id)"
    ")"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

}

void CGeoDB::migrateDB(int version)
{
    qDebug() << "void CGeoDB::migrateDB(int version)" << version;
    QSqlQuery query(db);

    query.prepare( "UPDATE versioninfo set version=:version");
    query.bindValue(":version", version);
    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }


}

void CGeoDB::setupTreeWidget()
{
    CGeoDBInternalEditLock lock(this);

    QList<QTreeWidgetItem*> children = itemDatabase->takeChildren();
    QTreeWidgetItem * child;
    foreach(child, children)
    {
        delete child;
    }

    queryChildrenFromDB(itemDatabase, 2);
    itemDatabase->setExpanded(true);
}

void CGeoDB::slotWptDBChanged()
{
    CGeoDBInternalEditLock lock(this);

    QSqlQuery query(db);
    qDeleteAll(itemWksWpt->takeChildren());

    CWptDB& wptdb = CWptDB::self();
    CWptDB::keys_t key;
    QList<CWptDB::keys_t> keys = wptdb.keys();

    foreach(key, keys)
    {

        CWpt * wpt = wptdb.getWptByKey(key.key);
        if(!wpt || wpt->sticky)
        {

            continue;
        }

        query.prepare("SELECT id FROM items WHERE key=:key");
        query.bindValue(":key", key.key);
        if(!query.exec())
        {
            qDebug() << query.lastQuery();
            qDebug() << query.lastError();
        }


        QTreeWidgetItem * item = new QTreeWidgetItem(itemWksWpt, eWpt);
        if(query.next())
        {
            item->setData(eName, eUserRoleDBKey, query.value(0));
            item->setIcon(eDBState, QIcon(":/icons/iconGeoDB16x16"));
        }
        item->setData(eName, eUserRoleQLKey, key.key);
        item->setIcon(eName, getWptIconByName(key.icon));
        item->setText(eName, key.name);
        item->setToolTip(eName, key.comment);
    }

    itemWksWpt->setHidden(itemWksWpt->childCount() == 0);

}

void CGeoDB::slotTrkDBChanged()
{
    CGeoDBInternalEditLock lock(this);
}

void CGeoDB::slotRteDBChanged()
{
    CGeoDBInternalEditLock lock(this);
}

void CGeoDB::slotOvlDBChanged()
{
    CGeoDBInternalEditLock lock(this);
}

void CGeoDB::slotAddDir()
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item = treeWidget->currentItem();
    if(item == 0)
    {
        return;
    }

    QString name = QInputDialog::getText(0, tr("Folder name..."), tr("Name of new folders"));
    if(name.isEmpty())
    {
        return;
    }

    QString comment = QInputDialog::getText(0, tr("Folder comment..."), tr("Would you like to add a comment?"));

    addFolder(item, name, comment);
}

void CGeoDB::slotDelDir()
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item = treeWidget->currentItem();
    QMessageBox::StandardButton but = QMessageBox::question(0, tr("Delete folders..."), tr("You are sure you want to delete '%1' and all items below?").arg(item->text(eName)), QMessageBox::Ok|QMessageBox::Abort);
    if(but == QMessageBox::Ok)
    {
        delFolder(item, true);
    }
}

void CGeoDB::slotEditDirComment()
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item = treeWidget->currentItem();
    quint64 itemId = item->data(eName, eUserRoleDBKey).toULongLong();

    QString comment = QInputDialog::getText(0, tr("Folder comment..."), tr("Edit comment?"), QLineEdit::Normal,item->toolTip(eName));
    if(comment.isEmpty())
    {
        return;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE folders SET comment=:comment WHERE id=:id");
    query.bindValue(":comment", comment);
    query.bindValue(":id", itemId);

    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
        return;
    }

    updateFolderById(itemId);
}

void CGeoDB::slotContextMenu(const QPoint& pos)
{
    QTreeWidgetItem * item = treeWidget->currentItem();
    QTreeWidgetItem * top  = item->parent();
    while(top && top->parent()) top = top->parent();
    if(top == 0) top = item;

    if(top == itemWorkspace)
    {
        QPoint p = treeWidget->mapToGlobal(pos);
        contextMenuWks->exec(p);
    }
    else if(top == itemPaperbin)
    {

    }
    else if(top == itemDatabase)
    {
        if(item->type() == eFolder)
        {
            if(item == itemDatabase)
            {
                actDelDir->setEnabled(false);
                actEditDirComment->setEnabled(false);
            }
            else
            {
                actDelDir->setEnabled(true);
                actEditDirComment->setEnabled(true);
            }

            QPoint p = treeWidget->mapToGlobal(pos);
            contextMenuFolder->exec(p);
        }
    }
}

void CGeoDB::slotItemExpanded(QTreeWidgetItem * item)
{
    CGeoDBInternalEditLock lock(this);

    const int size = item->childCount();
    if(size == 0)
    {
        queryChildrenFromDB(item, 2);
    }
    else
    {
        for(int i; i< size; i++)
        {
            QTreeWidgetItem  * child = item->child(i);
            if(child->type() == eFolder && child->childCount() == 0)
            {
                queryChildrenFromDB(child, 1);
            }
        }
    }
}

void CGeoDB::slotItemClicked(QTreeWidgetItem * item, int column)
{

}

void CGeoDB::slotItemChanged(QTreeWidgetItem * item, int column)
{
    if(isInternalEdit != 0)
    {
        return;
    }

    quint64 itemId = item->data(eName, eUserRoleDBKey).toULongLong();
    QString itemText = item->text(eName);

    if(itemText.isEmpty())
    {
        return;
    }

    if(column == eName)
    {
        QSqlQuery query(db);
        query.prepare("UPDATE folders SET name=:name WHERE id=:id");
        query.bindValue(":name", itemText);
        query.bindValue(":id", itemId);

        if(!query.exec())
        {
            qDebug() << query.lastQuery();
            qDebug() << query.lastError();
            return;
        }
    }

    updateFolderById(itemId);
}

void CGeoDB::queryChildrenFromDB(QTreeWidgetItem * parent, int levels)
{
    CGeoDBInternalEditLock lock(this);

    QSqlQuery query(db);
    const quint64 parentId = parent->data(eName, eUserRoleDBKey).toULongLong();

    if(parentId == 0)
    {
        return;
    }

    // folders 1st
    query.prepare("SELECT t1.child FROM folder2folder AS t1, folders AS t2 WHERE t1.parent = :id AND t2.id = t1.child ORDER BY t2.name");
    query.bindValue(":id", parentId);
    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
        return;
    }

    while(query.next())
    {
        quint64 childId = query.value(0).toULongLong();

        QSqlQuery query2(db);
        query2.prepare("SELECT icon, name, comment FROM folders WHERE id = :id");
        query2.bindValue(":id", childId);
        if(!query2.exec())
        {
            qDebug() << query2.lastQuery();
            qDebug() << query2.lastError();
            continue;
        }
        query2.next();

        QTreeWidgetItem * item = new QTreeWidgetItem(parent, eFolder);
        item->setData(eName, eUserRoleDBKey, childId);
        item->setIcon(eName, QIcon(query2.value(0).toString()));
        item->setText(eName, query2.value(1).toString());
        item->setToolTip(eName, query2.value(2).toString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        if(parentId != 1)
        {
            item->setCheckState(0, Qt::Unchecked);
        }

        if(levels)
        {
            queryChildrenFromDB(item, levels - 1);
        }
    }

    // items 2nd
    qDebug() << "parent id" << parentId;
    query.prepare("SELECT t1.child FROM folder2item AS t1, items AS t2 WHERE t1.parent = :id AND t2.id = t1.child ORDER BY t2.name");
    query.bindValue(":id", parentId);
    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
        return;
    }

    while(query.next())
    {
        quint64 childId = query.value(0).toULongLong();

        QSqlQuery query2(db);
        query2.prepare("SELECT type, icon, name, comment FROM items WHERE id = :id");
        query2.bindValue(":id", childId);
        if(!query2.exec())
        {
            qDebug() << query2.lastQuery();
            qDebug() << query2.lastError();
            continue;
        }
        query2.next();

        QTreeWidgetItem * item = new QTreeWidgetItem(parent, query2.value(0).toInt());
        item->setData(eName, eUserRoleDBKey, childId);
        if(query2.value(0).toInt() == eWpt)
        {
            item->setIcon(eName, getWptIconByName(query2.value(1).toString()));
        }
        else
        {
            item->setIcon(eName, QIcon(query2.value(1).toString()));
        }
        item->setText(eName, query2.value(2).toString());
        item->setToolTip(eName, query2.value(3).toString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        if(parentId != 1)
        {
            item->setCheckState(0, Qt::Unchecked);
        }
    }

    treeWidget->header()->setResizeMode(eName,QHeaderView::ResizeToContents);
}

void CGeoDB::addFolder(QTreeWidgetItem * parent, const QString& name, const QString& comment)
{
    CGeoDBInternalEditLock lock(this);

    QSqlQuery query(db);
    quint64 parentId = parent->data(eName, eUserRoleDBKey).toULongLong();
    quint64 childId = 0;

    query.prepare("INSERT INTO folders (icon, name, comment) VALUES (:icon, :name, :comment)");
    if(parentId == 1)
    {
        query.bindValue(":icon", ":/icons/iconFolderBlue16x16");
    }
    else
    {
        query.bindValue(":icon", ":/icons/iconFolderGreen16x16");
    }
    query.bindValue(":name", name);
    query.bindValue(":comment", comment);

    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
        return;
    }

    if(!query.exec("SELECT last_insert_rowid() from folders"))
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
        return;
    }
    query.next();
    childId = query.value(0).toULongLong();
    if(childId == 0)
    {
        qDebug() << "childId equals 0. bad.";
        return;
    }

    query.prepare("INSERT INTO folder2folder (parent, child) VALUES (:parent, :child)");
    query.bindValue(":parent", parentId);
    query.bindValue(":child", childId);

    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
        return;
    }

    QTreeWidgetItem * item = new QTreeWidgetItem(eFolder);
    item->setData(eName, eUserRoleDBKey, childId);
    if(parentId == 1)
    {
        item->setIcon(eName, QIcon(":/icons/iconFolderBlue16x16"));
    }
    else
    {
        item->setIcon(eName, QIcon(":/icons/iconFolderGreen16x16"));
        item->setCheckState(0, Qt::Unchecked);
    }
    item->setText(eName, name);
    item->setToolTip(eName, comment);
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    addFolderById(parentId, item);

    // delete item as it will be cloned by addFolderById() and never used directly
    delete item;
}

void CGeoDB::delFolder(QTreeWidgetItem * item, bool isTopLevel)
{
    CGeoDBInternalEditLock lock(this);

    int i;
    const int size   = item->childCount();
    quint64 itemId   = item->data(eName, eUserRoleDBKey).toULongLong();
    quint64 parentId = item->parent()->data(eName, eUserRoleDBKey).toULongLong();

    QSqlQuery query(db);
    // delete this particular relation first
    query.prepare("DELETE FROM folder2folder WHERE parent=:parent AND child=:child");
    query.bindValue(":parent", parentId);
    query.bindValue(":child", itemId);
    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    // next query if the item is used as child in any other relation
    query.prepare("SELECT * FROM folder2folder WHERE child=:id");
    query.bindValue(":id", itemId);
    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }

    // if there is no other relation delete the children, too.
    if(!query.next())
    {
        for(i = 0; i < size; i++)
        {
            delFolder(item->child(i), false);
        }

        // remove the child items relations
        query.prepare("DELETE FROM folder2item WHERE parent=:id");
        query.bindValue(":id", itemId);
        if(!query.exec())
        {
            qDebug() << query.lastQuery();
            qDebug() << query.lastError();
        }


        // and remove the folder
        query.prepare("DELETE FROM folders WHERE id=:id");
        query.bindValue(":id", itemId);
        if(!query.exec())
        {
            qDebug() << query.lastQuery();
            qDebug() << query.lastError();
        }
    }

    if(isTopLevel)
    {
        delFolderById(parentId, itemId);
    }

}

void CGeoDB::addFolderById(quint64 parentId, QTreeWidgetItem * child)
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive, eName);

    foreach(item, items)
    {
        if(item->type() != eFolder)
        {
            continue;
        }

        if(item->data(eName, eUserRoleDBKey).toULongLong() == parentId)
        {
            QTreeWidgetItem * clone = new QTreeWidgetItem(child->type());
            clone->setIcon(eName, child->icon(eName));
            clone->setIcon(eName, child->icon(eName));
            clone->setData(eName, eUserRoleDBKey, child->data(eName, eUserRoleDBKey));
            clone->setData(eName, eUserRoleQLKey, child->data(eName, eUserRoleQLKey));
            clone->setText(eName, child->text(eName));
            clone->setToolTip(eName, child->toolTip(eName));

            if(parentId != 1)
            {
                clone->setCheckState(0, Qt::Unchecked);
            }

            item->addChild(clone);
        }
    }
}

void CGeoDB::delFolderById(quint64 parentId, quint64 childId)
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive, eName);
    QList<QTreeWidgetItem*> itemsToDelete;

    foreach(item, items)
    {
        if(item->type() != eFolder)
        {
            continue;
        }
        if(item->data(eName, eUserRoleDBKey).toULongLong() == parentId)
        {
            int i;
            const int size = item->childCount();
            for(i = 0; i < size; i++)
            {
                if(item->child(i)->data(eName, eUserRoleDBKey).toULongLong() == childId)
                {
                    // just collect the items, do not delete now to prevent crash
                    itemsToDelete << item->takeChild(i);
                    break;
                }
            }
        }
    }

    // now it's save to delete all items
    qDeleteAll(itemsToDelete);
}

void CGeoDB::updateFolderById(quint64 id)
{
    CGeoDBInternalEditLock lock(this);

    QTreeWidgetItem * item;
    QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive, eName);

    QSqlQuery query(db);
    query.prepare("SELECT icon, name, comment FROM folders WHERE id=:id");
    query.bindValue(":id", id);
    if(!query.exec())
    {
        qDebug() << query.lastQuery();
        qDebug() << query.lastError();
    }
    query.next();

    foreach(item, items)
    {
        if(item->type() != eFolder)
        {
            continue;
        }
        if(item->data(eName, eUserRoleDBKey).toULongLong() == id)
        {
            item->setIcon(eName, QIcon(query.value(0).toString()));
            item->setText(eName, query.value(1).toString());
            item->setToolTip(eName, query.value(2).toString());
        }
    }
}


void CGeoDB::slotAddItems()
{
    CGeoDBInternalEditLock lock(this);

    QSqlQuery query(db);
    quint64 parentId, childId = 0;
    CDlgSelGeoDBFolder dlg(db, parentId);

    dlg.exec();

    if(parentId == 0)
    {
        return;
    }

    int i;
    int size;
    QTreeWidgetItem * item;

    size = itemWksWpt->childCount();
    for(i = 0; i < size; i++)
    {
        item = itemWksWpt->child(i);
        if(!item->isSelected())
        {
            continue;
        }

        // test for item with qlandkarte key
        QString key = item->data(eName, eUserRoleQLKey).toString();
        query.prepare("SELECT id FROM items WHERE key=:key");
        query.bindValue(":key", key);
        if(!query.exec())
        {
            qDebug() << query.lastQuery();
            qDebug() << query.lastError();
        }
        if(query.next())
        {
            childId = query.value(0).toULongLong();
        }
        else
        {
            // insert item
            QString key = item->data(eName, eUserRoleQLKey).toString();
            CWpt * wpt  = CWptDB::self().getWptByKey(key);
            QByteArray buffer;
            QDataStream stream(&buffer, QIODevice::WriteOnly);
            stream.setVersion(QDataStream::Qt_4_5);
            stream << *wpt;

            // add item to database
            query.prepare("INSERT INTO items (type, key, date, icon, name, comment, data) "
                          "VALUES (:type, :key, :date, :icon, :name, :comment, :data)");

            query.bindValue(":type", eWpt);
            query.bindValue(":key", wpt->key());
            query.bindValue(":date", QDateTime::fromTime_t(wpt->timestamp).toString("yyyy-MM-dd hh-mm-ss"));
            query.bindValue(":icon", wpt->icon);
            query.bindValue(":name", wpt->name);
            query.bindValue(":comment", wpt->comment);
            query.bindValue(":data", buffer);
            if(!query.exec())
            {
                qDebug() << query.lastQuery();
                qDebug() << query.lastError();
            }

            if(!query.exec("SELECT last_insert_rowid() from items"))
            {
                qDebug() << query.lastQuery();
                qDebug() << query.lastError();
                return;
            }
            query.next();
            childId = query.value(0).toULongLong();
            if(childId == 0)
            {
                qDebug() << "childId equals 0. bad.";
                return;
            }
        }
        item->setData(eName, eUserRoleDBKey, childId);
        item->setIcon(eDBState, QIcon(":/icons/iconGeoDB16x16"));
        // create link folder <-> item
        query.prepare("SELECT id FROM folder2item WHERE parent=:parent AND child=:child");
        query.bindValue(":parent", parentId);
        query.bindValue(":child", childId);
        if(!query.exec())
        {
            qDebug() << query.lastQuery();
            qDebug() << query.lastError();
        }


        if(!query.next())
        {
            query.prepare("INSERT INTO folder2item (parent, child) VALUES (:parent, :child)");
            query.bindValue(":parent", parentId);
            query.bindValue(":child", childId);
            if(!query.exec())
            {
                qDebug() << query.lastQuery();
                qDebug() << query.lastError();
                return;
            }
            // update tree widget
            addFolderById(parentId, item);
        }
    }
}
