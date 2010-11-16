#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
#include <QMouseEvent>
#include <QRubberBand>
#include <QUrl>

#include <qgisinterface.h>
#include <qgsapplication.h>
#include <qgslogger.h>
#include <qgsmapcanvas.h>
#include <qgsmaptopixel.h>
#include <qgsvectorlayer.h>

#include "qgsgoogleearthtool.h"
#include "qgskmlconverter.h"

QgsGoogleEarthTool::QgsGoogleEarthTool( QgsMapCanvas *canvas )
    : QgsMapTool( canvas ), mDragging( false ), mRubberBand( NULL ),
    kmlConverter( new QgsKmlConverter )
{
  mCursor = QCursor( Qt::PointingHandCursor );
}

QgsGoogleEarthTool::~QgsGoogleEarthTool()
{
  delete kmlConverter;
}

void QgsGoogleEarthTool::canvasMoveEvent( QMouseEvent *e )
{
  if ( !( e->buttons() & Qt::LeftButton ) )
    return;

  if ( !mDragging )
  {
    mDragging = true;
    mRubberBand = new QRubberBand( QRubberBand::Rectangle, mCanvas );
    mGERect.setTopLeft( e->pos() );
  }

  mGERect.setBottomRight( e->pos() );
  mRubberBand->setGeometry( mGERect.normalized() );
  mRubberBand->show();
}

void QgsGoogleEarthTool::canvasPressEvent( QMouseEvent *e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  mGERect.setRect( 0, 0, 0, 0 );
}

void QgsGoogleEarthTool::canvasReleaseEvent( QMouseEvent *e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  QgsVectorLayer *vlayer = dynamic_cast<QgsVectorLayer*>( mCanvas->currentLayer() );
  if ( vlayer )
  {
    QgsFeatureList featureList;
    if ( mDragging )
    {
      mDragging = false;
      delete mRubberBand;
      mRubberBand = NULL;

      // store the rectangle
      mGERect.setRight( e->pos().x() );
      mGERect.setBottom( e->pos().y() );

      // get features list selected by rectangle
      featureList = selecteManyFeatures( vlayer, mGERect );
    }
    else
    {
      // get selected feature
      featureList = selectOneFeature( vlayer, e->pos() );
    }

    // export selected features to kml
    QString tempFileName = kmlConverter->exportFeaturesToKmlFile( vlayer, featureList );

    // open kml in Google Earth
    if ( !tempFileName.isEmpty() && QFileInfo( tempFileName ).exists() )
      QDesktopServices::openUrl( QUrl::fromLocalFile( tempFileName ) );
  }
  else
  {
    mDragging = false;
    if ( mRubberBand )
    {
      delete mRubberBand;
      mRubberBand = NULL;
    }
  }
}

void QgsGoogleEarthTool::exportLayerToKml()
{
  QgsVectorLayer *vlayer = dynamic_cast<QgsVectorLayer*>( mCanvas->currentLayer() );
  if ( vlayer )
  {
    // export active layer to kml
    QString tempFileName = kmlConverter->exportLayerToKmlFile( vlayer );
    // open kml in Google Earth
    QDesktopServices::openUrl( QUrl( "file:///" + tempFileName ) );
  }
}

QgsFeatureList QgsGoogleEarthTool::selectOneFeature( QgsVectorLayer *vlayer, const QPoint &pos )
{
    // copy|past from QgsMapToolSelect
    QRect select_rect;
    int boxSize = 0;
    if ( vlayer->geometryType() != QGis::Polygon )
    {
        //if point or line use an artificial bounding box of 10x10 pixels
        //to aid the user to click on a feature accurately
        boxSize = 5;
    }
    else
    {
        //otherwise just use the click point for polys
        boxSize = 1;
    }

    select_rect.setLeft( pos.x() - boxSize );
    select_rect.setRight( pos.x() + boxSize );
    select_rect.setTop( pos.y() - boxSize );
    select_rect.setBottom( pos.y() + boxSize );

    // transform rectangle to map coordinate
    const QgsMapToPixel *transform = mCanvas->getCoordinateTransform();
    QgsPoint ll = transform->toMapCoordinates( select_rect.left(), select_rect.bottom() );
    QgsPoint ur = transform->toMapCoordinates( select_rect.right(), select_rect.top() );
    QgsRectangle searchRect( ll.x(), ll.y(), ur.x(), ur.y() );

    selecteFeatures( vlayer, searchRect );
    return vlayer->selectedFeatures();
}

QgsFeatureList QgsGoogleEarthTool::selecteManyFeatures( QgsVectorLayer *vlayer, const QRect &rect )
{
  const QgsMapToPixel* coordinateTransform = mCanvas->getCoordinateTransform();

  // set the extent to the Google Earth select
  QgsPoint ll = coordinateTransform->toMapCoordinates( rect.left(), rect.bottom() );
  QgsPoint ur = coordinateTransform->toMapCoordinates( rect.right(), rect.top() );

  QgsRectangle searchRect;
  searchRect.setXMinimum( ll.x() );
  searchRect.setYMinimum( ll.y() );
  searchRect.setXMaximum( ur.x() );
  searchRect.setYMaximum( ur.y() );
  searchRect.normalize();

  selecteFeatures( vlayer, searchRect );
  return vlayer->selectedFeatures();
}

void QgsGoogleEarthTool::selecteFeatures( QgsVectorLayer *vlayer, const QgsRectangle &rect )
{
  // prevent selecting to an empty extent
  if ( rect.width() == 0 || rect.height() == 0 )
  {
    return;
  }

  QgsRectangle searchRect;
  try
  {
    searchRect = toLayerCoordinates( vlayer, rect );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    // catch exception for 'invalid' rectangle and leave existing selection unchanged
    QgsLogger::warning( "Caught CRS exception " + QString( __FILE__ ) + ": " + QString::number( __LINE__ ) );
    QMessageBox::warning( mCanvas, QObject::tr( "CRS Exception" ),
                          QObject::tr( "Selection extends beyond layer's coordinate system." ) );
    return;
  }

  QgsApplication::setOverrideCursor( Qt::WaitCursor );
  vlayer->select( searchRect, false );
  QgsApplication::restoreOverrideCursor();
}
