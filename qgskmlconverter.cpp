#include <QFile>
#include <QMessageBox>

#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgslogger.h>
#include <qgsmapcanvas.h>
#include <qgsrenderer.h>
#include <qgssymbol.h>
#include <qgsvectorlayer.h>
#include <qgsuniquevaluerenderer.h>

#include "qgskmlconverter.h"

#define STYLEIDDELIMIT "."

const QString myPathToIcon = "http://maps.google.com/mapfiles/kml/shapes/donut.png";

QgsKmlConverter::QgsKmlConverter()
{
  QgsApplication::setOrganizationName( "gis-lab" );
  QgsApplication::setOrganizationDomain( "gis-lab.info" );
  QgsApplication::setApplicationName( "qgis2google2" );
}

QgsKmlConverter::~QgsKmlConverter()
{
  while ( mTempKmlFiles.count() )
  {
    QFile *file = mTempKmlFiles.takeFirst();
    file->remove();
    delete file;
  }
}

QString QgsKmlConverter::exportLayerToKmlFile( QgsVectorLayer *vlayer )
{
  if ( vlayer )
  {
    QgsFeatureList featureList;
    QgsRectangle rect = vlayer->extent();

    QgsApplication::setOverrideCursor( Qt::WaitCursor );
    vlayer->select( rect, false );
    featureList = vlayer->selectedFeatures();
    vlayer->invertSelection();
    QgsApplication::restoreOverrideCursor();

    return exportFeaturesToKmlFile( vlayer, featureList );
  }
  return QString();
}

QString QgsKmlConverter::exportFeaturesToKmlFile( QgsVectorLayer *vlayer, const QgsFeatureList &flist )
{
  QgsApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  QFile *tempFile = getTempFile();
  if ( !tempFile->exists() )
    return QString();

  // set codec for kml file
  QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
  QTextStream out( tempFile );

  out.setAutoDetectUnicode( false );
  out.setCodec( codec );

  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl
      << "<kml xmlns=\"http://earth.google.com/kml/2.2\"" << endl
      << "xmlns:gx=\"http://www.google.com/kml/ext/2.2\">" << endl;

  out << "<Document>" << endl
      << "<name>" << removeEscapeChars( vlayer->name() ) << "</name>" << endl;

  const QgsRenderer *renderer = vlayer->renderer();
  QList< QgsSymbol *> symbols = renderer->symbols();
  QString styleId = "styleOf-" + vlayer->name();
  bool bSingleSymbol = renderer->name() == "Single Symbol";
  bool bUniqueValue = renderer->name() == "Unique Value";

  QSettings settings;
  bool bOverideUniqueValue = settings.value( "/qgis2google/overridelayerstyle" ).toBool();
  if ( bOverideUniqueValue )
  {
    bSingleSymbol = true;
    bUniqueValue = false;
  }

  QString kmlStyle;
  if ( bSingleSymbol )
  {
    QList<QgsSymbol *> symbols = vlayer->renderer()->symbols();
    // read default values for kml from symbology
    QgsSymbol *symbol = symbols.first();
    // create style kml for one symbol
    if ( symbol )
      out << styleKmlSingleSymbol( vlayer->getTransparency(), symbol, styleId, vlayer->geometryType() ) << endl;
  }
  else if ( bUniqueValue )
  {
    // create style kml for many symbols
    out << styleKmlUniqueValue( vlayer->getTransparency(), styleId, symbols ) << endl;
  }
  else
  {
    // dont process other renderers symbols
  }

  // export eatch feature to kml format
  foreach ( QgsFeature feature, flist )
  {
    QgsGeometry *geometry = feature.geometry();
    if ( geometry )
    {
      out << "<Placemark>" << endl;
      // try to find name of feature from attribute table and set one for kml's placemark as html
      if ( bSingleSymbol )
      {
        out << placemarkNameKml( vlayer, feature.attributeMap() ) << endl;
      }
      else // Unique Value
      {
        const QgsUniqueValueRenderer *urenderer = dynamic_cast<const QgsUniqueValueRenderer *>( renderer );
        QgsSymbol *symbol = symbolForFeature( &feature, urenderer );
        if ( symbol )
          out << "<name>" + removeEscapeChars( symbol->lowerValue() ) + "</name>" << endl;
      }

      // try to find placemark description in attribute table (it should be in html format)
      out << placemarkDescriptionKml( vlayer, feature.attributeMap() ) << endl;

      if ( bUniqueValue )
      {
        const QgsUniqueValueRenderer *uniqValRenderer = dynamic_cast<const QgsUniqueValueRenderer *>( renderer );
        QgsSymbol *symbol = symbolForFeature( &feature, uniqValRenderer );
        QString uniqStyleId = featureStyleId( symbol, styleId );
        out << "<styleUrl>" << uniqStyleId << "</styleUrl>" << endl;
      }
      else
      {
        out << "<styleUrl>" << styleId << "</styleUrl>" << endl;
      }

      // convert wkt to kml and write to kml file
      out << convertWkbToKml(geometry) << endl;
      out << "</Placemark>" << endl;
    }
  }

  out << "</Document>" << endl
      << "</kml>" << endl;

  QgsApplication::restoreOverrideCursor();
  return tempFile->fileName();
}

// try to find feature's name in attribute table
int QgsKmlConverter::attributeNameIndex( QgsVectorLayer *vlayer )
{
  QgsAttributeList attributeList = vlayer->pendingAllAttributesList();

  for( int i = 0; i < attributeList.count(); i++ )
  {
    QString attrDisplayName = vlayer->attributeDisplayName( attributeList.at( i ) );
    if ( attrDisplayName.compare( "name", Qt::CaseInsensitive ) == 0)
    {
      return attributeList.at( i );
    }
  }
  return -1;
}

// create kml string with feature's name from attribute table
QString QgsKmlConverter::placemarkNameKml( QgsVectorLayer *vlayer, QgsAttributeMap attrMap )
{
  QString result;
  QTextStream out( &result );

  int index = attributeNameIndex( vlayer );

  if ( index > -1 )
  {
    QString name = attrMap.value( index ).toString();
    if ( !name.isEmpty() )
    {
      out << "<name>" << name << "</name>";
    }
  }
  return removeEscapeChars(result);
}

// try to find feature's description in attribute table
int QgsKmlConverter::attributeDescriprionIndex( QgsVectorLayer *vlayer )
{
  QgsAttributeList attributeList = vlayer->pendingAllAttributesList();

  for( int i = 0; i < attributeList.count(); i++ )
  {
    QString attrDisplayName = vlayer->attributeDisplayName( attributeList.at( i ) );
    if ( attrDisplayName.startsWith( "descr", Qt::CaseInsensitive ) )
    {
      return attributeList.at( i );
    }
  }
  return -1;
}

// create kml string with feature's description from attribute table
QString QgsKmlConverter::placemarkDescriptionKml( QgsVectorLayer *vlayer, QgsAttributeMap attrMap )
{
  QString result;
  QTextStream out( &result );

  int index = attributeDescriprionIndex( vlayer );

  if ( index > -1 )
  {
    QString description = attrMap.value( index ).toString();
    if ( !description.isEmpty() )
    {
      out << "<description>" << description << "</description>";
    }
  }
  return removeEscapeChars(result);
}

// in kml instead argb is abgr color modele
QRgb QgsKmlConverter::rgba2abgr( QColor color )
{
  qreal redF = color.redF();
  qreal blueF = color.blueF();
  color.setRedF( blueF );
  color.setBlueF( redF );

  return color.rgba();
}

// find symbol (that contains color, size, fill settings) wich feature will be draw
QgsSymbol *QgsKmlConverter::symbolForFeature( QgsFeature *feature, const QgsUniqueValueRenderer *urenderer )
{
  //first find out the value
  const QgsAttributeMap& attrs = feature->attributeMap();
  QString value = attrs[ urenderer->classificationField() ].toString();

  QList<QgsSymbol *> symbols = urenderer->symbols();
  foreach( QgsSymbol *symbol, symbols )
  {
    if ( symbol->lowerValue() == value )
      return symbol;
  }
  return NULL;
}

// create kml style identificator for feature
QString QgsKmlConverter::featureStyleId( QgsSymbol *symbol, QString styleId )
{
  if ( symbol && !symbol->lowerValue().isEmpty() )
    return removeEscapeChars(styleId + STYLEIDDELIMIT + symbol->lowerValue());
  else
    return "";
}

// create string with kml style description section, all values takes from settings
QString QgsKmlConverter::styleKmlSingleSymbol( int transp, QgsSymbol *symbol, QString styleId, QGis::GeometryType typeOfFeature )
{
  double scale = 1.0;
  QSettings settings;
  QString result, colorMode("normal");
  QTextStream out( &result );
  QColor color, fillColor;
  bool bOverrideLayerStyle = settings.value("/qgis2google/overridelayerstyle").toBool();

  out << "<Style id=\"" << styleId << "\">" << endl;

  color = symbol->color();
  color.setAlpha( transp );
  fillColor = symbol->fillColor();
  fillColor.setAlpha( transp );
  double lineWidth = symbol->lineWidth();
  int bPolyStyle = symbol->brush().style() != Qt::NoBrush;
  int fill = bPolyStyle;
  bPolyStyle = symbol->pen().style() != Qt::NoPen;
  int outline = bPolyStyle;

  if (bOverrideLayerStyle)
  {
    color = settings.value( "/qgis2google/label/color" ).value<QColor>();
    colorMode = settings.value( "/qgis2google/label/colormode" ).toString();
    scale = settings.value( "/qgis2google/label/scale" ).toDouble();
  }
  out << "<LabelStyle>" << endl
      << "<color>" << hex << rgba2abgr( color ) << dec << "</color>" << endl
      << "<colorMode>" << colorMode << "</colorMode>" << endl
      << "<scale>" << scale << "</scale>" << endl
      << "</LabelStyle>" << endl;

  if (bOverrideLayerStyle)
  {
    fillColor = settings.value( "/qgis2google/icon/color" ).value<QColor>();
    colorMode = settings.value( "/qgis2google/icon/colormode" ).toString();
    scale = settings.value( "/qgis2google/icon/scale" ).toDouble();
  }
  out << "<IconStyle>" << endl
      << "<color>" << hex << rgba2abgr( fillColor ) << dec << "</color>" << endl
      << "<colorMode>" << colorMode << "</colorMode>" << endl
      << "<scale>" << scale << "</scale>" << endl
      << "<Icon>" << endl << "<href>" << myPathToIcon << "</href>" << endl << "</Icon>" << endl
      << "</IconStyle>" << endl;

  if (bOverrideLayerStyle)
  {
    color = settings.value( "/qgis2google/line/color" ).value<QColor>();
    colorMode = settings.value( "/qgis2google/line/colormode" ).toString();
    lineWidth = settings.value( "/qgis2google/line/width" ).toDouble();
  }
  out << "<LineStyle>" << endl
      << "<color>" << hex << rgba2abgr( color ) << dec << "</color>" << endl
      << "<colorMode>" << colorMode << "</colorMode>" << endl
      << "<width>" << lineWidth << "</width>" << endl
      << "</LineStyle>" << endl;

  if (bOverrideLayerStyle)
  {
    fillColor = settings.value( "/qgis2google/poly/color" ).value<QColor>();
    colorMode = settings.value( "/qgis2google/poly/colormode" ).toString();
    fill = settings.value( "/qgis2google/poly/fill" ).toInt();
    outline = settings.value( "/qgis2google/poly/outline" ).toInt();
  }
  out << "<PolyStyle>" << endl
      << "<color>" << hex << rgba2abgr( fillColor ) << dec << "</color>" << endl
      << "<colorMode>" << colorMode << "</colorMode>" << endl
      << "<fill>" << fill << "</fill>" << endl
      << "<outline>" << outline << "</outline>" << endl
      << "</PolyStyle>" << endl;

  out << "</Style>";

  return result;
}

// create string with kml style description section for each symbols, all values takes from settings
QString QgsKmlConverter::styleKmlUniqueValue( int transp, QString styleId, QList<QgsSymbol *> symbols )
{
  double scale = 1.0;
  QString result, colorMode( "normal" );
  QTextStream out( &result );
  QColor color, fillColor;

  foreach( QgsSymbol *symbol, symbols )
  {
    out << endl << "<Style id=\"" << featureStyleId( symbol, styleId ) << "\">" << endl;

    color = symbol->color();
    color.setAlpha( transp );
    fillColor = symbol->fillColor();
    fillColor.setAlpha( transp );

    out << "<LabelStyle>" << endl
        << "<color>" << hex << rgba2abgr( color ) << dec << "</color>" << endl
        << "<colorMode>" << colorMode << "</colorMode>" << endl
        << "<scale>" << scale << "</scale>" << endl
        << "</LabelStyle>" << endl;

    out << "<IconStyle>" << endl
        << "<color>" << hex << rgba2abgr( fillColor ) << dec << "</color>" << endl
        << "<colorMode>" << colorMode << "</colorMode>" << endl
        << "<scale>" << scale << "</scale>" << endl
        << "<Icon>" << endl << "<href>" << myPathToIcon << "</href>" << endl << "</Icon>" << endl
        << "</IconStyle>" << endl;

    double lineWidth = symbol->lineWidth();
    out << "<LineStyle>" << endl
        << "<color>" << hex << rgba2abgr( color ) << dec << "</color>" << endl
        << "<colorMode>" << colorMode << "</colorMode>" << endl
        << "<width>" << lineWidth << "</width>" << endl
        << "</LineStyle>" << endl;

    int bPolyStyle = symbol->brush().style() != Qt::NoBrush;
    int fill = bPolyStyle;
    bPolyStyle = symbol->pen().style() != Qt::NoPen;
    int outline = bPolyStyle;
    out << "<PolyStyle>" << endl
        << "<color>" << hex << rgba2abgr( fillColor ) << dec << "</color>" << endl
        << "<colorMode>" << colorMode << "</colorMode>" << endl
        << "<fill>" << fill << "</fill>" << endl
        << "<outline>" << outline << "</outline>" << endl
        << "</PolyStyle>" << endl;

    out << "</Style>";
  }
  return result;
}

QString QgsKmlConverter::convertWkbToKml( QgsGeometry *geometry )
{
  QString result;
  QSettings settings;
  QTextStream out( &result );
  bool hasZValue;

  QGis::WkbType wkbType = geometry->wkbType();
  switch (wkbType)
  {
  case QGis::WKBPoint25D:
  case QGis::WKBPoint:
    {
      int altitudeVal = settings.value( "/qgis2google/point/altitudevalue" ).toInt();
      QString altitudeMode = settings.value( "/qgis2google/point/altitudemode" ).toString();
      hasZValue = altitudeMode != "clampToGround" && altitudeMode != "clampToSeaFloor";

      QgsPoint wkbPoint = geometry->asPoint();
      QString pointString = QString::number(wkbPoint.x(), 'f', 6) + "," +
                            QString::number(wkbPoint.y(), 'f', 6);
      if ( hasZValue )
        pointString.append(QString(",%1").arg(altitudeVal));

      int extrude = settings.value( "/qgis2google/point/extrude" ).toInt();
      out << "<Point>" << endl
          << "<extrude>" << extrude << "</extrude>" << endl
          << "<altitudeMode>" << altitudeMode << "</altitudeMode>" << endl
          << "<coordinates>" << pointString << "</coordinates>" << endl
          << "</Point>";

      return result;
    }
  case QGis::WKBLineString25D:
  case QGis::WKBLineString:
    {
      int altitudeVal = settings.value( "/qgis2google/line/altitudevalue" ).toInt();
      QString altitudeMode = settings.value( "/qgis2google/line/altitudemode" ).toString();
      hasZValue = altitudeMode != "clampToGround" && altitudeMode != "clampToSeaFloor" && altitudeVal != -1;

      QgsPolyline wkbPolyline = geometry->asPolyline();
      QString polylineString;
      foreach (QgsPoint pt, wkbPolyline)
      {
        polylineString += QString::number(pt.x(), 'f', 6) + "," +
                          QString::number(pt.y(), 'f', 6);
        if (hasZValue)
          polylineString += QString(",%1").arg(altitudeVal);

        polylineString += " ";
      }
      polylineString.chop(1);

      int extrude = settings.value( "/qgis2google/line/extrude" ).toInt();
      int tessellate = settings.value( "/qgis2google/line/tessellate" ).toInt();
      out << "<LineString>" << endl
          << "<extrude>" << extrude << "</extrude>" << endl
          << "<tessellate>" << tessellate << "</tessellate>" << endl
          << "<altitudeMode>" << altitudeMode << "</altitudeMode>" << endl
          << "<coordinates>" << polylineString << "</coordinates>" << endl
          << "</LineString>";

      return result;
    }
  case QGis::WKBPolygon25D:
  case QGis::WKBPolygon:
    {
      int altitudeVal = settings.value( "/qgis2google/poly/altitudevalue" ).toInt();
      QString altitudeMode = settings.value( "/qgis2google/poly/altitudemode" ).toString();
      hasZValue = altitudeMode != "clampToGround" && altitudeMode != "clampToSeaFloor" && altitudeVal != -1;

      QgsPolygon wkbPolygon = geometry->asPolygon();
      QStringList polylineStringList;
      foreach (QgsPolyline ln, wkbPolygon)
      {
        QString polylineString;
        foreach (QgsPoint pt, ln)
        {
          polylineString += QString::number(pt.x(), 'f', 6) + "," +
                            QString::number(pt.y(), 'f', 6);
          if (hasZValue)
            polylineString += QString(",%1").arg(altitudeVal);

          polylineString += " ";
        }
        polylineString.chop(1);
        polylineStringList.append(polylineString);
      }

      int extrude = settings.value( "/qgis2google/poly/extrude" ).toInt();
      int tessellate = settings.value( "/qgis2google/poly/tessellate" ).toInt();
      out << "<Polygon>" << endl
          << "<extrude>" << extrude << "</extrude>" << endl
          << "<tessellate>" << tessellate << "</tessellate>" << endl
          << "<gx:altitudeMode>" << altitudeMode << "</gx:altitudeMode>" << endl
          << "<outerBoundaryIs>" << endl << "<LinearRing>" << endl
          << "<coordinates>" << polylineStringList.at( 0 ) << "</coordinates>" << endl
          << "</LinearRing>" << endl << "</outerBoundaryIs>" << endl;

      for( int i = 1; i < polylineStringList.count(); i++ )
      {
        out << "<innerBoundaryIs>" << endl << "<LinearRing>" << endl
            << "<coordinates>" << polylineStringList.at( i ) << "</coordinates>" << endl
            << "</LinearRing>" << endl << "</innerBoundaryIs>" << endl;
      }
      out << "</Polygon>";

      return result;
    }
  case QGis::WKBMultiPoint25D:
  case QGis::WKBMultiPoint:
    {
      int altitudeVal = settings.value( "/qgis2google/point/altitudevalue" ).toInt();
      QString altitudeMode = settings.value( "/qgis2google/point/altitudemode" ).toString();
      hasZValue = altitudeMode != "clampToGround" && altitudeMode != "clampToSeaFloor" && altitudeVal != -1;

      out << "<MultiGeometry>" << endl;
      QgsMultiPoint wkbMultiPoint = geometry->asMultiPoint();
      foreach (QgsPoint pt, wkbMultiPoint)
      {
        QString pointString = QString::number(pt.x(), 'f', 6) + "," +
                              QString::number(pt.y(), 'f', 6);
        if ( hasZValue )
          pointString.append(QString(",%1").arg(altitudeVal));

        int extrude = settings.value( "/qgis2google/point/extrude" ).toInt();
        out << "<Point>" << endl
            << "<extrude>" << extrude << "</extrude>" << endl
            << "<altitudeMode>" << altitudeMode << "</altitudeMode>" << endl
            << "<coordinates>" << pointString << "</coordinates>" << endl
            << "</Point>" << endl;
      }
      out << "</MultiGeometry>";

      return result;
    }
  case QGis::WKBMultiLineString25D:
  case QGis::WKBMultiLineString:
    {
      int altitudeVal = settings.value( "/qgis2google/line/altitudevalue" ).toInt();
      QString altitudeMode = settings.value( "/qgis2google/line/altitudemode" ).toString();
      hasZValue = altitudeMode != "clampToGround" && altitudeMode != "clampToSeaFloor" && altitudeVal != -1;

      out << "<MultiGeometry>" << endl;
      QgsMultiPolyline wkbMultiPolyline = geometry->asMultiPolyline();
      foreach (QgsPolyline ln, wkbMultiPolyline)
      {
        QString polylineString;
        foreach (QgsPoint pt, ln)
        {
          polylineString += QString::number(pt.x(), 'f', 6) + "," +
                            QString::number(pt.y(), 'f', 6);
          if (hasZValue)
            polylineString += QString(",%1").arg(altitudeVal);

          polylineString += " ";
        }
        polylineString.chop(1);

        int extrude = settings.value( "/qgis2google/line/extrude" ).toInt();
        int tessellate = settings.value( "/qgis2google/line/tessellate" ).toInt();
        out << "<LineString>" << endl
            << "<extrude>" << extrude << "</extrude>" << endl
            << "<tessellate>" << tessellate << "</tessellate>" << endl
            << "<altitudeMode>" << altitudeMode << "</altitudeMode>" << endl
            << "<coordinates>" << polylineString << "</coordinates>" << endl
            << "</LineString>" << endl;
      }
      out << "</MultiGeometry>";

      return result;
    }
  case QGis::WKBMultiPolygon25D:
  case QGis::WKBMultiPolygon:
    {
      int altitudeVal = settings.value( "/qgis2google/poly/altitudevalue" ).toInt();
      QString altitudeMode = settings.value( "/qgis2google/poly/altitudemode" ).toString();
      hasZValue = altitudeMode != "clampToGround" && altitudeMode != "clampToSeaFloor" && altitudeVal != -1;

      out << "<MultiGeometry>" << endl;
      QgsMultiPolygon wkbMultiPolygon = geometry->asMultiPolygon();
      foreach (QgsPolygon pln, wkbMultiPolygon)
      {
        QStringList polylineStringList;
        foreach (QgsPolyline ln, pln)
        {
          QString polylineString;
          foreach (QgsPoint pt, ln)
          {
            polylineString += QString::number(pt.x(), 'f', 6) + "," +
                              QString::number(pt.y(), 'f', 6);
            if (hasZValue)
              polylineString += QString(",%1").arg(altitudeVal);

            polylineString += " ";
          }
          polylineString.chop(1);
          polylineStringList.append(polylineString);
        }

        int extrude = settings.value( "/qgis2google/poly/extrude" ).toInt();
        int tessellate = settings.value( "/qgis2google/poly/tessellate" ).toInt();
        out << "<Polygon>" << endl
            << "<extrude>" << extrude << "</extrude>" << endl
            << "<tessellate>" << tessellate << "</tessellate>" << endl
            << "<gx:altitudeMode>" << altitudeMode << "</gx:altitudeMode>" << endl
            << "<outerBoundaryIs>" << endl << "<LinearRing>" << endl
            << "<coordinates>" << polylineStringList.at( 0 ) << "</coordinates>" << endl
            << "</LinearRing>" << endl << "</outerBoundaryIs>" << endl;

        for( int i = 1; i < polylineStringList.count(); i++ )
        {
          out << "<innerBoundaryIs>" << endl << "<LinearRing>" << endl
              << "<coordinates>" << polylineStringList.at( i ) << "</coordinates>" << endl
              << "</LinearRing>" << endl << "</innerBoundaryIs>" << endl;
        }
        out << "</Polygon>" << endl;
      }
      out << "</MultiGeometry>";

      return result;
    }
  default:
    QgsDebugMsg( "error: mGeometry type not recognized" );
    return QString();
  }
}

// generate name for temporary file
QString QgsKmlConverter::generateTempFileName()
{
  QString tempFileName( "" );
  int currentIndex = 0;
  while ( tempFileName.isEmpty() )
  {
    QString testFileName = QDir::tempPath() + "/qgis2google" + QString::number( currentIndex ) + ".kml";
    if ( !QFile::exists( testFileName ) )
      tempFileName = testFileName;
    currentIndex++;
  }
  return tempFileName;
}

// open for write temporary file
QFile *QgsKmlConverter::getTempFile()
{
  QString tempFileName = generateTempFileName();
  if ( !tempFileName.isEmpty() )
  {
    QFile *tempFile = new QFile( tempFileName );
    if ( !tempFile->open( QIODevice::WriteOnly ) )
    {
      QMessageBox::critical( NULL, tr( "File open" ), tr( "Unable to open the temprory file %1" )
                             .arg( tempFile->fileName() ) );
      QgsLogger::debug( tr( "Unable to open the temprory file %1" ) );
      Q_ASSERT( 0 );
      return NULL;
    }
    mTempKmlFiles.push_back( tempFile );
    return tempFile;
  }
  return NULL;
}

// remove escape character from kml file string
QString QgsKmlConverter::removeEscapeChars( QString in )
{
  return in.replace( QRegExp( "&(?!amp;)" ), "&amp;" );
}
