//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- Copyright (c) 2009 Marc Feld
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C-  ------------------------------------------------------------------


#include "CActions.h"
#include <QAction>
#include <QDebug>
#include "CMainWindow.h"

CActions::CActions(QObject *parent) : QObject(parent), parent(parent)
{

  createAction(tr("F1"),":/icons/iconMap16x16",tr("&Map ..."),"aSwitchToMap",tr("Manage maps."));
  createAction(tr("F2"),":/icons/iconWaypoint16x16",tr("&Waypoint ..."),"aSwitchToWpt",tr("Manage waypoints."));
  createAction(tr("F3"),":/icons/iconTrack16x16",tr("&Track ..."),"aSwitchToTrack",tr("Manage tracks."));
  createAction(tr("F4"),":/icons/iconRoute16x16",tr("&Route ..."),"aSwitchToRoute",tr(""));
  createAction(tr("F5"),":/icons/iconLiveLog16x16",tr("Live &Log ..."),"aSwitchToLiveLog",tr("Toggle live log recording."));
  createAction(tr("F6"),":/icons/iconOverlay16x16",tr("&Overlay ..."),"aSwitchToOverlay",tr("Manage overlays, such as textboxes"));
  createAction(tr("F7"),":/icons/iconGlobe+16x16",tr("Mor&e ..."),"aSwitchToMainMore",tr("Extended functions."));
  createAction(tr("F8"),":/icons/iconClear16x16",tr("&Clear all"),"aClearAll",tr("Remove all waypoints, tracks, ..."));
  createAction(tr("F9"),":/icons/iconUpload16x16",tr("&Upload all"),"aUploadAll",tr("Upload all data to device."));
  createAction(tr("F10"),":/icons/iconDownload16x16",tr("&Download all"),"aDownloadAll",tr("Download all data from device."));

  createAction(tr("ESC"),":/icons/iconBack16x16",tr("&Back"),"aSwitchToMain",tr("Go back to main menu."));
  createAction(tr("F1"),":/icons/iconMoveMap16x16",tr("Mo&ve Map"),"aMoveArea",tr("Move the map. Press down the left mouse button and move the mouse."));
  createAction(tr("F2"),":/icons/iconZoomArea16x16",tr("&Zoom Map"),"aZoomArea",tr("Select area for zoom."));
  createAction(tr("F3"),":/icons/iconCenter16x16",tr("&Center Map"),"aCenterMap",tr("Find your map by jumping to it's center."));

  createAction(tr("F5"),":/icons/iconSelect16x16",tr("Select &Sub Map"),"aSelectArea",tr("Select area of map to export. Select area by pressing down the left mouse button and move the mouse."));
  createAction(tr("F6"),":/icons/iconEdit16x16",tr("&Edit / Create Map"),"aEditMap",tr(""));
  createAction(tr("F7"),":/icons/iconFind16x16",tr("&Search Map"),"aSearchMap",tr("Find symbols on a map via image recognition."));
#ifdef PLOT_3D
  createAction(tr("F8"),":/icons/icon3D16x16.png",tr("3&D Map..."), "aSwitchToMap3D", tr("Show 3D map"));
#endif
  createAction(tr("F9"),":/icons/iconUpload16x16",tr("Upload"),"aUploadMap",tr("Upload map selection to device."));

#ifdef PLOT_3D
  createAction(tr("F0"),":/icons/iconBack16x16",tr("Close"),"aCloseMap3D",tr("Close 3D view."));
  createAction(tr("F1"),0,tr("Flat / 3D Mode"),"aMap3DMode",tr("Toggle between 3D track only and full map surface model."));
  createAction(tr("F2"),":/icons/iconInc16x16",tr("Inc. Elevation"),"aMap3DZoomPlus",tr("Make elevations on the map higher as they are."));
  createAction(tr("F3"),":/icons/iconDec16x16",tr("Dec. Elevation"),"aMap3DZoomMinus",tr("Make elevations on the map lower as they are."));
  createAction(tr("F4"),":/icons/iconLight16x16",tr("Lighting On/Off"), "aMap3DLighting",tr("Turn on/off lighting."));
#endif
//
  createAction(tr("F5"),":/icons/iconAdd16x16",tr("New Waypoint"),"aNewWpt",tr("Create a new user waypoint. The default position will be the current cursor position."));
  createAction(tr("F6"),":/icons/iconEdit16x16",tr("Edit Waypoint"),"aEditWpt",tr("Switch cursor to 'Edit Waypoint' mode. Point-n-click to edit a waypoint."));
  createAction(tr("F7"),":/icons/iconWptMove16x16",tr("Move Waypoint"),"aMoveWpt",tr("Switch cursor to 'Move Waypoint' mode. Point-click-move-click to move a waypoint. Use the right mouse button to abort. It is ok to leave 'Move Waypoint' mode and to resume."));
#ifdef HAS_EXIF
  createAction(tr("F8"),":/icons/iconImage16x16",tr("From Images..."),"aImageWpt",tr("Create waypoints from geo-referenced images in a path."));
#endif
  createAction(tr("F9"),":/icons/iconUpload16x16",tr("Upload"),"aUploadWpt",tr("Upload waypoints to device."));
  createAction(tr("F10"),":/icons/iconDownload16x16",tr("Download"),"aDownloadWpt",tr("Download waypoints from device."));
//
  createAction(tr("F5"),":/icons/iconAdd16x16",tr("Combine Tracks"),"aCombineTrack",tr("Combine multiple selected tracks to one."));
  createAction(tr("F6"),":/icons/iconEdit16x16",tr("Edit Track"),"aEditTrack",tr("Toggle track edit dialog."));
  createAction(tr("F7"),":/icons/iconEditCut16x16",tr("Cut Tracks"),"aCutTrack",tr("Cut a track into pieces."));
  createAction(tr("F8"),":/icons/iconSelect16x16",tr("Select Points"),"aSelTrack",tr("Select track points by rectangle."));
  createAction(tr("F9"),":/icons/iconUpload16x16",tr("Upload"),"aUploadTrack",tr("Upload tracks to device."));
  createAction(tr("F10"),":/icons/iconDownload16x16",tr("Download"),"aDownloadTrack",tr("Download tracks from device."));
//
  createAction(tr("F5"),":/icons/iconPlayPause16x16",tr("Start / Stop"),"aLiveLog",tr("Start / stop live log recording."));
  createAction(tr("F6"),":/icons/iconLock16x16",tr("Move Map to Pos."),"aLockMap",tr("Move the map to keep the positon cursor centered."));
  createAction(tr("F7"),":/icons/iconAdd16x16",tr("Add Waypoint"),"aAddWpt",tr("Add a waypoint at current position."));
//

  createAction(tr("F5"),":/icons/iconText16x16",tr("Add Static Text Box"),"aText",tr("Add text on the map."));
  createAction(tr("F6"),":/icons/iconTextBox16x16",tr("Add Geo-Ref. Text Box"),"aTextBox",tr("Add a textbox on the map."));
  createAction(tr("F7"),":/icons/iconDistance16x16",tr("Add Distance Polyline"),"aDistance",tr("Add a polyline to measure distances."));
//
  createAction(tr("F5"),":/icons/iconDiary16x16",tr("Diary"),"aDiary",tr("Add / edit diary data"));
  createAction(tr("F6"),":/icons/iconColorChooser16x16",tr("Pick Color"),"aColorPicker",tr("test only"));
  createAction(tr("F7"),0,tr("Create World Basemap"),"aWorldBasemap",tr("Create a world basemap from OSM tiles to be used by QLandkarte M"));

//
  createAction(tr("F9"),":/icons/iconUpload16x16",tr("Upload"),"aUploadRoute",tr("Upload routes to device."));
  createAction(tr("F10"),":/icons/iconDownload16x16",tr("Download"),"aDownloadRoute",tr("Download routes from device."));
//



}

CActions::~CActions()
{

}

void CActions::createAction(const QString& shortCut, const char * icon,const QString& name, const QString& actionName, const QString& toolTip )
{
  if (findChild<QAction *>(actionName))
  {
    qDebug() << tr("Action with the name '%1' already registered. Please choose an other name.").arg(actionName);
    return;
  }

  QAction *tmpAction = new QAction(QIcon(icon), name,this);
  tmpAction->setObjectName(actionName);
  tmpAction->setShortcut(shortCut);
  tmpAction->setToolTip(toolTip);
 // theMainWindow->addAction(tmpAction);
}


QAction *CActions::getAction(const QString& actionObjectName)
{
  QAction *tmpAction = findChild<QAction *>(actionObjectName);
  if (tmpAction)
    return tmpAction;
  else
  {
    qDebug() << tr("Action with name '%1' not found. %2").arg(actionObjectName).arg(Q_FUNC_INFO);
    return new QAction(this);
  }
}

