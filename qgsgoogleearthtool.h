#include <QColor>
#include <QRect>
#include <QStringList>

#include <qgsfeature.h>
#include <qgsmaptool.h>

class QFile;
class QRubberBand;

class QgsVectorLayer;

class QgsKmlConverter;

class QgsGoogleEarthTool : public QgsMapTool
{
  Q_OBJECT
public:
  QgsGoogleEarthTool( QgsMapCanvas *canvas );
  ~QgsGoogleEarthTool( );

public slots:
  void exportLayerToKml();

protected:
  void canvasPressEvent( QMouseEvent *e );
  void canvasMoveEvent( QMouseEvent *e );
  void canvasReleaseEvent( QMouseEvent *e );

private:
  void selecteFeatures( QgsVectorLayer *vlayer, const QgsRectangle &rect );
  QgsFeatureList selecteManyFeatures( QgsVectorLayer *vlayer, const QRect &rect );
  QgsFeatureList selectOneFeature( QgsVectorLayer *vlayer, const QPoint &pos );

  QList<QFile *> mTempKmlFiles;
  //! stores Google Earth select rect
  QRect mGERect;

  //! Flag to indicate a map canvas drag operation is taking place
  bool mDragging;

  //! TODO: to be changed to a canvas item
  QRubberBand *mRubberBand;

  QgsKmlConverter *kmlConverter;
};
