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
#ifndef CGEODB_H
#define CGEODB_H

#include <QWidget>
#include <QSqlDatabase>
#include "ui_IGeoToolWidget.h"

class QTabWidget;
class QTreeWidgetItem;
class QMenu;

class CGeoDB : public QWidget, private Ui::IGeoToolWidget
{
    Q_OBJECT;
    public:
        CGeoDB(QTabWidget * tb, QWidget * parent);
        virtual ~CGeoDB();

        /// switch tabbar if project manager gains focus
        void gainFocus();

    private slots:
        /// save all elements in the workspace branch to be restored in next application start
        void saveWorkspace();
        /// initialize database from scratch
        void initDB();
        /// move database from one version to most resent version
        void migrateDB(int version);


    private:
        friend class CGeoDBInternalEditLock;
        friend class CDlgSelGeoDBFolder;
        friend class CDlgEditFolder;

        enum EntryType_e {
            eWpt        = QTreeWidgetItem::UserType + 3,
            eTrk        = QTreeWidgetItem::UserType + 4,
            eRte        = QTreeWidgetItem::UserType + 5,
            eOvl        = QTreeWidgetItem::UserType + 6,

            eFolder0    = QTreeWidgetItem::UserType + 100,
            eFolderT    = QTreeWidgetItem::UserType + 101,
            eFolder1    = QTreeWidgetItem::UserType + 102,
            eFolder2    = QTreeWidgetItem::UserType + 103,
            eFolderN    = QTreeWidgetItem::UserType + 104,
        };

        enum ColumnType_e {
            eCoName       = 0,
            eCoState    = 1,
        };
        enum UserRoles_e {
            eUrDBKey  = Qt::UserRole,
            eUrQLKey  = Qt::UserRole + 1,
            eUrFolder = Qt::UserRole + 2
        };


        /// left hand tool widget tabbar
        QTabWidget * tabbar;
        /// a counter that is used by CGeoDBInternalEditLock to block slotItemChanged() on internal item edits
        quint32 isInternalEdit;
        /// a copy from CResources::saveOnExit;
        bool saveOnExit;
        /// the database object
        QSqlDatabase db;

        QTreeWidgetItem * itemDatabase;
        QTreeWidgetItem * itemLostFound;
        QTreeWidgetItem * itemWorkspace;

        QTreeWidgetItem * itemWksWpt;
        QTreeWidgetItem * itemWksTrk;
        QTreeWidgetItem * itemWksRte;
        QTreeWidgetItem * itemWksOvl;


//    private slots:
//        void slotAddFolder();
//        void slotDelFolder();
//        void slotEditFolder();
//        void slotMoveFolder();
//        void slotCopyFolder();

//        void slotAddItems();
//        void slotDelItems();
//        void slotMoveItems();
//        void slotCopyItems();
//        void slotSaveItems();
//        void slotHardCopyItem();

//        void slotContextMenu(const QPoint&);
//        void slotItemExpanded(QTreeWidgetItem * item);
//        void slotItemChanged(QTreeWidgetItem * item, int column);
//        void slotItemDoubleClicked(QTreeWidgetItem * item, int column);

//        void slotWptDBChanged();
//        void slotTrkDBChanged();
//        void slotRteDBChanged();
//        void slotOvlDBChanged();

//        void slotMoveLost();
//        void slotDelLost();

//        void slotModifiedWpt(const QString&);
//        void slotModifiedTrk(const QString&);
//        void slotModifiedRte(const QString&);
//        void slotModifiedOvl(const QString&);

//    private:
//        friend class CGeoDBInternalEditLock;
//        friend class CDlgSelGeoDBFolder;
//        friend class CDlgEditFolder;
//        friend bool sortItemsLessThan(QTreeWidgetItem * item1, QTreeWidgetItem * item2);

//        enum EntryType_e {
//            //eFolder     = QTreeWidgetItem::UserType + 1,
//            //eTypFolder  = QTreeWidgetItem::UserType + 2,
//            eWpt        = QTreeWidgetItem::UserType + 3,
//            eTrk        = QTreeWidgetItem::UserType + 4,
//            eRte        = QTreeWidgetItem::UserType + 5,
//            eOvl        = QTreeWidgetItem::UserType + 6,

//            eFolder0    = QTreeWidgetItem::UserType + 100,
//            eFolderT    = QTreeWidgetItem::UserType + 101,
//            eFolder1    = QTreeWidgetItem::UserType + 102,
//            eFolder2    = QTreeWidgetItem::UserType + 103,
//            eFolderN    = QTreeWidgetItem::UserType + 104,
//        };

//        enum ColumnType_e {
//            eName       = 0,
//            eDBState    = 1,
//        };
//        enum UserRoles_e {
//            eUserRoleDBKey  = Qt::UserRole,
//            eUserRoleQLKey  = Qt::UserRole + 1,
//            eUserRoleFolder = Qt::UserRole + 2
//        };

//        void initDB();
//        void migrateDB(int version);
//        void queryChildrenFromDB(QTreeWidgetItem * parent, int levels);

//        void setupTreeWidget();
//        void addFolder(QTreeWidgetItem * parent, const QString& name, const QString& comment, qint32 type);
//        void delFolder(QTreeWidgetItem * item, bool isTopLevel);

//        /// search treeWidget for items with id and update their content from database
//        void updateFolderById(quint64 id);
//        /// search treeWidget for items with id and update their content from database
//        void updateItemById(quint64 id);
//        /// search treeWidget for items with id  and add copy of item as child
//        void addFolderById(quint64 parentId, QTreeWidgetItem * child);
//        /// search treeWidget for folders with parentId and delete items including all their children with childId
//        void delFolderById(quint64 parentId, quint64 childId);
//        /// search treeWidget for items with parentId and delete items
//        void delItemById(quint64 parentId, quint64 childId);

//        void updateLostFound();
//        void updateModifyMarker();
//        void updateModifyMarker(QTreeWidgetItem * item, QSet<QString>& keys, const QString& label);
//        void updateCheckmarks();
//        void updateCheckmarks(QTreeWidgetItem * item);

//        void moveChildrenToWks(quint64 parentId);
//        void delChildrenFromWks(quint64 parentId);

//        void addWptToDB(quint64 parentId, QTreeWidgetItem * item);
//        void addTrkToDB(quint64 parentId, QTreeWidgetItem * item);
////        void addRteToDB(quint64 parentId, QTreeWidgetItem * item);
//        void addOvlToDB(quint64 parentId, QTreeWidgetItem * item);

//        void saveWorkspace();
//        void loadWorkspace();
//        void updateWorkspace();

//        void sortItems(QTreeWidgetItem * item);

//        QTabWidget * tabbar;
//        QTreeWidgetItem * itemDatabase;
//        QTreeWidgetItem * itemLostFound;
//        QTreeWidgetItem * itemWorkspace;

//        QTreeWidgetItem * itemWksWpt;
//        QTreeWidgetItem * itemWksTrk;
//        QTreeWidgetItem * itemWksRte;
//        QTreeWidgetItem * itemWksOvl;

//        QSqlDatabase db;

//        QMenu * contextMenuFolder;
//        QAction * actAddDir;
//        QAction * actDelDir;
//        QAction * actEditDirComment;
//        QAction * actMoveDir;
//        QAction * actCopyDir;


//        QMenu * contextMenuItem;
//        QAction * actMoveItem;
//        QAction * actCopyItem;
//        QAction * actDelItem;

//        QMenu * contextMenuWks;
//        QAction * actAddToDB;
//        QAction * actSaveToDB;
//        QAction * actHardCopy;

//        QMenu * contextMenuLost;
//        QAction * actMoveLost;
//        QAction * actDelLost;

//        quint32 isInternalEdit;

//        QSet<QString> keysWptModified;
//        QSet<QString> keysTrkModified;
//        QSet<QString> keysRteModified;
//        QSet<QString> keysOvlModified;

//        QSet<quint64> keysWksWpt;
//        QSet<quint64> keysWksTrk;
//        QSet<quint64> keysWksRte;
//        QSet<quint64> keysWksOvl;

//        bool saveOnExit;
};

#endif //CGEODB_H

