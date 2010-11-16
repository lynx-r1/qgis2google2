#ifndef QGSKMLCONVERTER_H
#define QGSKMLCONVERTER_H

#include <QColor>
#include <QTextStream>

#include <qgis.h>
#include <qgsfeature.h>

class QFile;

class QgsRenderer;
class QgsSymbol;
class QgsVectorLayer;
class QgsUniqueValueRenderer;

class QgsKmlConverter
{
  Q_DECLARE_TR_FUNCTIONS(QgsKmlConverter);

public:
  QgsKmlConverter();
  ~QgsKmlConverter();

  QString exportLayerToKmlFile( QgsVectorLayer *vlayer );
  QString exportFeaturesToKmlFile( QgsVectorLayer *vlayer, const QgsFeatureList &flist );

private:
  QString generateTempFileName();
  QFile *getTempFile();

  QString convertWkbToKml( QgsGeometry *geometry );

  int attributeNameIndex( QgsVectorLayer *vlayer);
  int attributeDescriprionIndex( QgsVectorLayer *vlayer);

  QString styleKmlSingleSymbol( int transp, QgsSymbol *symbol, QString styleId,
                                QGis::GeometryType typeOfFeature );
  QString styleKmlUniqueValue( int transp, QString styleId, QList<QgsSymbol *> symbols );
  QString placemarkNameKml( QgsVectorLayer *vlayer, QgsAttributeMap attrMap );
  QString placemarkDescriptionKml( QgsVectorLayer *vlayer, QgsAttributeMap attrMap );

  QString featureStyleId( QgsSymbol *symbol, QString styleId );
  QgsSymbol *symbolForFeature( QgsFeature *feature, const QgsUniqueValueRenderer *urenderer );

  QString removeEscapeChars( QString in );
  QRgb rgba2abgr( QColor color );

  QList<QFile *> mTempKmlFiles;
};

#endif // QGSKMLCONVERTER_H
