/***************************************************************************
    qgis2google.h
    -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id: plugin.h 9138 2008-08-23 21:37:31Z jef $ */
/***************************************************************************
 *   QGIS Programming conventions:
 *
 *   mVariableName - a class level member variable
 *   sVariableName - a static class level member variable
 *   variableName() - accessor for a class member (no 'get' in front of name)
 *   setVariableName() - mutator for a class member (prefix with 'set')
 *
 *   Additional useful conventions:
 *
 *   theVariableName - a method parameter (prefix with 'the')
 *   myVariableName - a locally declared variable within a method ('my' prefix)
 *
 *   DO: Use mixed case variable names - myVariableName
 *   DON'T: separate variable names using underscores: my_variable_name (NO!)
 *
 * **************************************************************************/
#ifndef qgis2google_H
#define qgis2google_H

//QT4 includes
#include <QObject>

//QGIS includes
#include "../qgisplugin.h"

//forward declarations
class QAction;
class QToolBar;

class QgisInterface;
class QgsGoogleEarthTool;
class QgsMapLayer;
class QgsMapTool;

/**
* \class Plugin
* \brief [name] plugin for QGIS
* [description]
*/
class qgis2google: public QObject, public QgisPlugin
{
  Q_OBJECT
public:
  /**
    * Constructor for a plugin. The QgisInterface pointer is passed by
    * QGIS when it attempts to instantiate the plugin.
    * @param theInterface Pointer to the QgisInterface object.
     */
  qgis2google( QgisInterface  *theInterface );
  //! Destructor
  virtual ~qgis2google();

public slots:
  //! init the gui
  virtual void initGui();
  //! unload the plugin
  void unload();
  //! show the help document
  void help();

private slots:
  void setToolToEarth();
  //! tune plugin
  void settings();
  void setDefaultSettings( QgsMapLayer *layer );
  void about();

private:
  int mPluginType;
  //! Pointer to the QGIS interface object
  QgisInterface *mQGisIface;

  QgsGoogleEarthTool *mSendToEarthTool;

  QAction *mFeatureToEarthAction;
  QAction *mLayerToEarthAction;
  QAction *mSettingsAction;
  QAction *mInfoAction;

  QToolBar *mToolsToolBar;
  QgsMapTool *mPrevMapTool;

  QString mPluginName;
};

#endif //qgis2google_H
