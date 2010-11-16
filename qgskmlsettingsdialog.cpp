#include <QColorDialog>
#include <QDebug>

#include <qgsapplication.h>

#include "qgskmlsettingsdialog.h"
#include "ui_qgskmlsettingsdialogbase.h"

QgsKmlSettingsDialog::QgsKmlSettingsDialog(QWidget *parent, QGis::GeometryType typeOfFeature ) :
    QDialog(parent), m_ui(new Ui::QgsKmlSettingsDialog)
{
  m_ui->setupUi(this);

  initComboBoxes();

  QgsApplication::setOrganizationName( "gis-lab" );
  QgsApplication::setOrganizationDomain( "gis-lab.info" );
  QgsApplication::setApplicationName( "qgis2google2" );

  setTypeOfTab( typeOfFeature );
  readSettings();
}

QgsKmlSettingsDialog::~QgsKmlSettingsDialog()
{
  delete m_ui;
}

void QgsKmlSettingsDialog::changeEvent(QEvent *e)
{
  QDialog::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

void QgsKmlSettingsDialog::setTypeOfTab( QGis::GeometryType typeOfFeature )
{
  switch ( typeOfFeature )
  {
  case QGis::Point:
    m_ui->tabWidget->setCurrentIndex( 0 ); // Point
    break;
  case QGis::Line:
    m_ui->tabWidget->setCurrentIndex( 1 ); // Line
    break;
  case QGis::Polygon:
    m_ui->tabWidget->setCurrentIndex( 2 ); // Polygone
  case QGis::UnknownGeometry:
    break;
  }
}

void QgsKmlSettingsDialog::setAltitudeItemsData( QComboBox *comboBox )
{
  comboBox->setItemData( 0, "clampToGround" );
  comboBox->setItemData( 1, "clampToSeaFloor" );
  comboBox->setItemData( 2, "relativeToGround" );
  comboBox->setItemData( 3, "relativeToSeaFloor" );
  comboBox->setItemData( 4, "absolute" );
}

void QgsKmlSettingsDialog::setColorModeItemsData( QComboBox *comboBox )
{
  comboBox->setItemData( 0, "normal" );
  comboBox->setItemData( 1, "random" );
}

//void QgsKmlSettingsDialog::setUnitsData( QComboBox *comboBox )
//{
//  comboBox->setItemData( 0, "pixels" );
//  comboBox->setItemData( 1, "fraction" );
//  comboBox->setItemData( 2, "insetPixels" );
//}

void QgsKmlSettingsDialog::initComboBoxes()
{
  QStringList stringList;

  stringList << tr( "Normal" ) << tr( "Random" );

  m_ui->cbLabelColorMode->addItems( stringList );
  setColorModeItemsData( m_ui->cbLabelColorMode );
  m_ui->cbLineColorMode->addItems( stringList );
  setColorModeItemsData( m_ui->cbLineColorMode );
  m_ui->cbPolyColorMode->addItems( stringList );
  setColorModeItemsData( m_ui->cbPolyColorMode );
  m_ui->cbIconColorMode->addItems( stringList );
  setColorModeItemsData( m_ui->cbIconColorMode );

  stringList.clear();
  stringList << tr( "Clamp to ground" ) << tr( "Clamp to sea floor" )
      << tr( "Relative to ground" ) << tr( "Relative to sea floor" ) << tr( "Absolute" );

  m_ui->cbPointAltitudeMode->addItems( stringList );
  setAltitudeItemsData( m_ui->cbPointAltitudeMode );
  m_ui->cbLineAltitudeMode->addItems( stringList );
  setAltitudeItemsData( m_ui->cbLineAltitudeMode );
  m_ui->cbPolyAltitudeMode->addItems( stringList );
  setAltitudeItemsData( m_ui->cbPolyAltitudeMode );

  stringList.clear();
  stringList << tr( "Yes" ) << tr( "No" );

  m_ui->cbPolyFill->addItems( stringList );
  m_ui->cbPolyOutline->addItems( stringList );

  m_ui->cbPointExtrude->addItems( stringList );
  m_ui->cbLineExtrude->addItems( stringList );
  m_ui->cbPolyExtrude->addItems( stringList );

  m_ui->cbLineTessellate->addItems( stringList );
  m_ui->cbPolyTessellate->addItems( stringList );

//  stringList.clear();
//  stringList << tr( "Pixels" ) << tr( "Fractions" ) << tr( "Inset pixels" );
//
//  m_ui->cbXunits->addItems( stringList );
//  setUnitsData( m_ui->cbXunits );
//  m_ui->cbYunits->addItems( stringList );
//  setUnitsData( m_ui->cbYunits );
}

QColor QgsKmlSettingsDialog::getButtonColor( QPushButton *button )
{
  QPalette pallet = button->palette();
  QPalette::ColorRole colorRole = button->backgroundRole();

  return pallet.color( colorRole );
}

void QgsKmlSettingsDialog::setButtonColor( QPushButton *button, QColor &color )
{
  button->setPalette( QPalette( color ) );
}

int QgsKmlSettingsDialog::qstringToBool( QString boolStr )
{
  if ( boolStr == tr( "Yes" ) ) return 1;
  if ( boolStr == tr( "No" ) ) return 0;
  return -1;
}

QString QgsKmlSettingsDialog::boolToQString( int boolVal )
{
  if ( boolVal ) return tr( "Yes" );
  if ( !boolVal ) return tr( "No" );
  return "";
}

void QgsKmlSettingsDialog::readSettings()
{
  QSettings settings;
  QString tmpStr;
  QColor tmpColor;
  int tmpInt;
  double tmpDbl;

  move( settings.value( "/qgis2google/pos", QPoint( 0, 0 ) ).toPoint() );
  resize( settings.value( "/qgis2google/size" ).toSize() );

  tmpInt = settings.value( "/qgis2google/label/opacity", 100 ).toInt();
  m_ui->sbxLabelOpacity->setValue( tmpInt );
  tmpColor = settings.value( "/qgis2google/label/color", QColor( 255, 255, 255 ) ).value<QColor>();
  tmpInt = tmpInt / 100.0 * 255.0;
  tmpColor.setAlpha( tmpInt );
  setButtonColor( m_ui->pbLabelColor, tmpColor );
  tmpStr = settings.value( "/qgis2google/label/colormode", "normal" ).toString();
  m_ui->cbLabelColorMode->setCurrentIndex( m_ui->cbLabelColorMode->findData( tmpStr ) );
  tmpDbl = settings.value( "/qgis2google/label/scale", 1.0 ).toDouble();
  m_ui->dsbLabelScale->setValue( tmpDbl );

  //  tmpStr = settings.value( "/qgis2google/icon/location" ).toString(); // TODO
  tmpInt = settings.value( "/qgis2google/icon/opacity", 100 ).toInt();
  m_ui->sbxIconOpacity->setValue( tmpInt );
  tmpColor = settings.value( "/qgis2google/icon/color", QColor( 255, 255, 255 ) ).value<QColor>();
  tmpInt = tmpInt / 100.0 * 255.0;
  tmpColor.setAlpha( tmpInt );
  setButtonColor( m_ui->pbIconColor, tmpColor );
  tmpStr = settings.value( "/qgis2google/icon/colormode", "normal" ).toString();
  m_ui->cbIconColorMode->setCurrentIndex( m_ui->cbIconColorMode->findData( tmpStr ) );
  tmpDbl = settings.value( "/qgis2google/icon/scale", 1.0 ).toDouble();
  m_ui->dsbIconScale->setValue( tmpDbl );
  //  tmpInt = settings.value( "/qgis2google/icon/heading", 0 ).toInt(); // TODO
  //  tmpDbl = settings.value( "/qgis2google/icon/xunits", "fraction" ).toString();
  //  m_ui->cbXunits->setCurrentIndex( m_ui->cbXunits->findData( tmpDbl ) );
  //  tmpDbl = settings.value( "/qgis2google/icon/yunits", "fraction" ).toString();
  //  m_ui->cbYunits->setCurrentIndex( m_ui->cbYunits->findData( tmpDbl ) );
  //  tmpDbl = settings.value( "qgis2google/icon/x", 0.5 ).toDouble(); // TODO
  //  tmpDbl = settings.value( "qgis2google/icon/y", 0.5 ).toDouble(); // TODO

  tmpInt = settings.value( "/qgis2google/point/extrude", 0 ).toInt();
  m_ui->cbPointExtrude->setCurrentIndex( m_ui->cbPointExtrude->findText( boolToQString( tmpInt ) ) );
  tmpStr = settings.value( "/qgis2google/point/altitudemode", "clampToGround" ).toString();
  m_ui->cbPointAltitudeMode->setCurrentIndex( m_ui->cbPointAltitudeMode->findData( tmpStr ) );
  tmpInt = settings.value( "/qgis2google/point/altitudevalue", 0 ).toInt();
  m_ui->sbxPointAltitude->setValue( tmpInt );

  tmpInt = settings.value( "/qgis2google/line/opacity", 100 ).toInt();
  m_ui->sbxLineOpacity->setValue( tmpInt );
  tmpColor = settings.value( "/qgis2google/line/color", QColor( 0, 255, 255 ) ).value<QColor>();
  tmpInt = tmpInt / 100.0 * 255.0;
  tmpColor.setAlpha( tmpInt );
  setButtonColor( m_ui->pbLineColor, tmpColor );
  tmpStr = settings.value( "/qgis2google/line/colormode", "normal" ).toString();
  m_ui->cbLineColorMode->setCurrentIndex( m_ui->cbLineColorMode->findData( tmpStr ) );
  tmpInt = settings.value( "/qgis2google/line/width", 1.0 ).toInt();
  m_ui->dsbLineWidth->setValue( tmpInt );
  tmpInt = settings.value( "/qgis2google/line/extrude", 0 ).toInt();
  m_ui->cbLineExtrude->setCurrentIndex( m_ui->cbLineExtrude->findText( boolToQString( tmpInt ) ) );
  tmpInt = settings.value( "/qgis2google/line/tessellate", 0 ).toBool();
  m_ui->cbLineTessellate->setCurrentIndex( m_ui->cbLineTessellate->findText( boolToQString( tmpInt ) ) );
  tmpStr = settings.value( "/qgis2google/line/altitudemode", "clampToGround" ).toString();
  m_ui->cbLineAltitudeMode->setCurrentIndex( m_ui->cbLineAltitudeMode->findData( tmpStr ) );
  tmpInt = settings.value( "/qgis2google/line/altitudevalue", 0 ).toInt();
  m_ui->sbxLineAltitude->setValue( tmpInt );

  tmpInt = settings.value( "/qgis2google/poly/opacity", 100 ).toInt();
  m_ui->sbxPolyOpacity->setValue( tmpInt );
  tmpColor = settings.value( "/qgis2google/poly/color", QColor( 0, 0, 255 ) ).value<QColor>();
  tmpInt = tmpInt / 100.0 * 255.0;
  tmpColor.setAlpha( tmpInt );
  setButtonColor( m_ui->pbPolyColor, tmpColor );
  tmpStr = settings.value( "/qgis2google/poly/colormode", "normal" ).toString();
  m_ui->cbPolyColorMode->setCurrentIndex( m_ui->cbPolyColorMode->findData( tmpStr ) );
  tmpInt = settings.value( "/qgis2google/poly/extrude", 0 ).toInt();
  m_ui->cbPolyExtrude->setCurrentIndex( m_ui->cbPolyExtrude->findText( boolToQString( tmpInt ) ) );
  tmpInt = settings.value( "/qgis2google/poly/tessellate", 0 ).toBool();
  m_ui->cbPolyTessellate->setCurrentIndex( m_ui->cbPolyTessellate->findText( boolToQString( tmpInt ) ) );
  tmpStr = settings.value( "/qgis2google/poly/altitudemode", "clampToGround" ).toString();
  m_ui->cbPolyAltitudeMode->setCurrentIndex( m_ui->cbPolyAltitudeMode->findData( tmpStr ) );
  tmpInt = settings.value( "/qgis2google/poly/altitudevalue", 0 ).toInt();
  m_ui->sbxPolyAltitude->setValue( tmpInt );
  tmpInt = settings.value( "/qgis2google/poly/fill", 0 ).toBool();
  m_ui->cbPolyFill->setCurrentIndex( m_ui->cbPolyFill->findText( boolToQString( tmpInt ) ) );
  tmpInt = settings.value( "/qgis2google/poly/outline", 0 ).toBool();
  m_ui->cbPolyOutline->setCurrentIndex( m_ui->cbPolyOutline->findText( boolToQString( tmpInt ) ) );

  m_ui->tabWidget->setEnabled( false );
  tmpInt = settings.value( "/qgis2google/overridelayerstyle", 0 ).toBool();
  m_ui->chbOverrideLayerStyle->setChecked( tmpInt );
}

void QgsKmlSettingsDialog::writeSettings()
{
  QSettings settings;

  settings.setValue( "/qgis2google/size", size() );
  settings.setValue( "/qgis2google/pos", pos() );

  settings.setValue( "/qgis2google/label/color", getButtonColor( m_ui->pbLabelColor ) );
  settings.setValue( "/qgis2google/label/opacity", m_ui->sbxLabelOpacity->value() );
  settings.setValue( "/qgis2google/label/colormode", m_ui->cbLabelColorMode->itemData( m_ui->cbLabelColorMode->currentIndex() ) );
  settings.setValue( "/qgis2google/label/scale", m_ui->dsbLabelScale->value() );

  //  settings.setValue( "/qgis2google/icon/location", m_ui->cbTempFile->itemData( m_ui->cbTempFile->currentIndex() ) );
  settings.setValue( "/qgis2google/icon/color", getButtonColor( m_ui->pbIconColor ) );
  settings.setValue( "/qgis2google/icon/opacity", m_ui->sbxIconOpacity->value() );
  settings.setValue( "/qgis2google/icon/colormode", m_ui->cbIconColorMode->itemData( m_ui->cbIconColorMode->currentIndex() ) );
  settings.setValue( "/qgis2google/icon/scale", m_ui->dsbIconScale->value() );
  //  settings.setValue( "/qgis2google/icon/xunits", m_ui->cbXunits->itemData( m_ui->cbXunits->currentIndex() ) );
  //  settings.setValue( "/qgis2google/icon/yunits", m_ui->cbYunits->itemData( m_ui->cbYunits->currentIndex() ) );

  settings.setValue( "/qgis2google/point/extrude", qstringToBool( m_ui->cbPointExtrude->currentText() ) );
  settings.setValue( "/qgis2google/point/altitudemode", m_ui->cbPointAltitudeMode->itemData( m_ui->cbPointAltitudeMode->currentIndex() ) );
  settings.setValue( "/qgis2google/point/altitudevalue", m_ui->sbxPointAltitude->value() );

  settings.setValue( "/qgis2google/line/color", getButtonColor( m_ui->pbLineColor ) );
  settings.setValue( "/qgis2google/line/opacity", m_ui->sbxLineOpacity->value() );
  settings.setValue( "/qgis2google/line/colormode", m_ui->cbLineColorMode->itemData( m_ui->cbLineColorMode->currentIndex() ) );
  settings.setValue( "/qgis2google/line/width", m_ui->dsbLineWidth->value() );
  settings.setValue( "/qgis2google/line/extrude", qstringToBool( m_ui->cbLineExtrude->currentText() ) );
  settings.setValue( "/qgis2google/line/tessellate", qstringToBool( m_ui->cbLineTessellate->currentText() ) );
  settings.setValue( "/qgis2google/line/altitudemode", m_ui->cbLineAltitudeMode->itemData( m_ui->cbLineAltitudeMode->currentIndex() ) );
  settings.setValue( "/qgis2google/line/altitudevalue", m_ui->sbxLineAltitude->value() );

  settings.setValue( "/qgis2google/poly/color", getButtonColor( m_ui->pbPolyColor ) );
  settings.setValue( "/qgis2google/poly/opacity", m_ui->sbxPolyOpacity->value() );
  settings.setValue( "/qgis2google/poly/colormode", m_ui->cbPolyColorMode->itemData( m_ui->cbPolyColorMode->currentIndex() ) );
  settings.setValue( "/qgis2google/poly/extrude", qstringToBool( m_ui->cbPolyExtrude->currentText() ) );
  settings.setValue( "/qgis2google/poly/tessellate", qstringToBool( m_ui->cbPolyTessellate->currentText() ) );
  settings.setValue( "/qgis2google/poly/altitudemode", m_ui->cbPolyAltitudeMode->itemData( m_ui->cbPolyAltitudeMode->currentIndex() ) );
  settings.setValue( "/qgis2google/poly/altitudevalue", m_ui->sbxPolyAltitude->value() );
  settings.setValue( "/qgis2google/poly/fill", qstringToBool( m_ui->cbPolyFill->currentText() ) );
  settings.setValue( "/qgis2google/poly/outline", qstringToBool( m_ui->cbPolyOutline->currentText() ) );

  settings.setValue( "/qgis2google/overridelayerstyle", m_ui->chbOverrideLayerStyle->isChecked() );
}

void QgsKmlSettingsDialog::on_buttonBox_accepted()
{
  writeSettings();
}

void QgsKmlSettingsDialog::on_buttonBox_rejected()
{
  QSettings settings;
  settings.setValue( "/qgis2google/size", size() );
  settings.setValue( "/qgis2google/pos", pos() );
}

// Set color for features
void QgsKmlSettingsDialog::on_pbLabelColor_clicked()
{
  QColor color = QColorDialog::getColor( getButtonColor( m_ui->pbLabelColor ), this,
                                         tr( "Select color" ), QColorDialog::ShowAlphaChannel );
  if ( color.isValid() )
  {
    int iOpacityPers;

    setButtonColor( m_ui->pbLabelColor, color );
    iOpacityPers = color.alpha() * 100 / 255;
    m_ui->sbxLabelOpacity->setValue( iOpacityPers );
  }
}

void QgsKmlSettingsDialog::on_pbIconColor_clicked()
{
  QColor color = QColorDialog::getColor( getButtonColor( m_ui->pbIconColor ), this,
                                         tr( "Select color" ), QColorDialog::ShowAlphaChannel );
  if ( color.isValid() )
  {
    int iOpacityPers;

    setButtonColor( m_ui->pbIconColor, color );
    iOpacityPers = color.alpha() * 100 / 255;
    m_ui->sbxIconOpacity->setValue( iOpacityPers );
  }
}

void QgsKmlSettingsDialog::on_pbLineColor_clicked()
{
  QColor color = QColorDialog::getColor( getButtonColor( m_ui->pbLineColor ), this,
                                         tr( "Select color" ), QColorDialog::ShowAlphaChannel );
  if ( color.isValid() )
  {
    int iOpacityPers;

    setButtonColor( m_ui->pbLineColor, color );
    iOpacityPers = color.alpha() * 100 / 255;
    m_ui->sbxLineOpacity->setValue( iOpacityPers );
  }
}

void QgsKmlSettingsDialog::on_pbPolyColor_clicked()
{
  QColor color = QColorDialog::getColor( getButtonColor( m_ui->pbPolyColor ), this,
                                         tr( "Select color" ), QColorDialog::ShowAlphaChannel );
  if ( color.isValid() )
  {
    int iOpacityPers;

    setButtonColor( m_ui->pbPolyColor, color );
    iOpacityPers = color.alpha() * 100 / 255;
    m_ui->sbxPolyOpacity->setValue( iOpacityPers );
  }
}

void QgsKmlSettingsDialog::extrudeIndexChange( QComboBox *comboBox, QString text )
{
  if ( text == tr( "Yes" ) )
  {
    comboBox->removeItem( 0 );
    comboBox->removeItem( 0 );

    QString cbxName = comboBox->objectName();
    if ( comboBox->count() == 0 )
      altitudeControl( cbxName, false );
    else
      altitudeControl( cbxName, true );
  }
  else
  {
    if ( comboBox->findData( "clampToSeaFloor" ) == -1 )
    {
      comboBox->insertItem( 0, tr( "Clamp to sea floor" ) );
      comboBox->setItemData( 0, "clampToSeaFloor" );
    }
    if ( comboBox->findData( "clampToGround" ) == -1 )
    {
      comboBox->insertItem( 0, tr( "Clamp to ground" ) );
      comboBox->setItemData( 0, "clampToGround" );
    }
  }
}

void QgsKmlSettingsDialog::tessellateIndexChange( QComboBox *comboBox, QString text )
{
  if ( text == tr( "Yes" ) )
  {
    comboBox->removeItem( comboBox->findData( "relativeToGround" ) );
    comboBox->removeItem( comboBox->findData( "relativeToSeaFloor" ) );
    comboBox->removeItem( comboBox->findData( "absolute" ) );

    QString cbxName = comboBox->objectName();
    if ( comboBox->count() == 0 )
      altitudeControl( cbxName, false );
    else
      altitudeControl( cbxName, true );
  }
  else
  {
    if ( comboBox->findData( "relativeToGround" ) == -1 )
    {
      comboBox->addItem( tr( "Relative to ground" ) );
      comboBox->setItemData( comboBox->count() - 1, "relativeToGround" );
    }
    if ( comboBox->findData( "relativeToSeaFloor" ) == -1 )
    {
      comboBox->addItem( tr( "Relative to sea floor" ) );
      comboBox->setItemData( comboBox->count() - 1, "relativeToSeaFloor" );
    }
    if ( comboBox->findData( "absolute" ) == -1 )
    {
      comboBox->addItem( tr( "Absolute" ) );
      comboBox->setItemData( comboBox->count() - 1, "absolute" );
    }
  }
}

void QgsKmlSettingsDialog::on_cbPointExtrude_currentIndexChanged( const QString &text )
{
  extrudeIndexChange( m_ui->cbPointAltitudeMode, text );
}

void QgsKmlSettingsDialog::on_cbLineExtrude_currentIndexChanged( const QString &text )
{
  extrudeIndexChange( m_ui->cbLineAltitudeMode, text );
}

void QgsKmlSettingsDialog::on_cbPolyExtrude_currentIndexChanged( const QString &text )
{
  extrudeIndexChange( m_ui->cbPolyAltitudeMode, text );
}

void QgsKmlSettingsDialog::on_cbLineTessellate_currentIndexChanged( const QString &text )
{
  tessellateIndexChange( m_ui->cbLineAltitudeMode, text );
}

void QgsKmlSettingsDialog::on_cbPolyTessellate_currentIndexChanged( const QString &text )
{
  tessellateIndexChange( m_ui->cbPolyAltitudeMode, text );
}

void QgsKmlSettingsDialog::enableAltitudeControl( QLabel *label, QSpinBox *spinBox, bool enable )
{
  label->setEnabled( enable );
  spinBox->setEnabled( enable );
}

void QgsKmlSettingsDialog::altitudeControl( QString str, bool enable )
{
  if ( str == "cbPointAltitudeMode" )
    enableAltitudeControl( m_ui->lbPointAltitude, m_ui->sbxPointAltitude, enable );
  else if ( str == "cbLineAltitudeMode" )
    enableAltitudeControl( m_ui->lbLineAltitude, m_ui->sbxLineAltitude, enable );
  else if ( str == "cbPolyAltitudeMode" )
    enableAltitudeControl( m_ui->lbPolyAltitude, m_ui->sbxPolyAltitude, enable );
}

void QgsKmlSettingsDialog::setAltitudeModeToolTip( QComboBox *comboBox )
{
  if ( comboBox->currentText() == tr( "Clamp to ground" ) )
  {
    comboBox->setToolTip( tr( "Indicates to ignore an altitude specification" ) );
    QString cbxName = comboBox->objectName();
    altitudeControl( cbxName, false );
  }
  if ( comboBox->currentText() == tr( "Clamp to sea floor" ) )
  {
    comboBox->setToolTip( tr( "The altitude specification is ignored, and the point will be positioned on the sea floor. "
                              "If the point is on land rather than at sea, the point will be positioned on the ground" ) );
    QString cbxName = comboBox->objectName();
    altitudeControl( cbxName, false );
  }
  else if ( comboBox->currentText() == tr( "Relative to ground" ) )
  {
    comboBox->setToolTip( tr( "Sets the altitude of the element relative to the actual ground elevation of a particular location" ) );
    QString cbxName = comboBox->objectName();
    altitudeControl( cbxName, true );
  }
  else if ( comboBox->currentText() == tr( "Relative to sea floor" ) )
  {
    comboBox->setToolTip( tr( "Interprets the altitude as a value in meters above the sea floor. If the point is above land rather than sea,"
                              "the altitude will be interpreted as being above the ground" ) );
    QString cbxName = comboBox->objectName();
    altitudeControl( cbxName, true );
  }
  else if ( comboBox->currentText() == tr( "Absolute" ) )
  {
    comboBox->setToolTip( tr( "Sets the altitude of the coordinate relative to sea level, regardless of the actual elevation of the terrain beneath the element" ) );
    QString cbxName = comboBox->objectName();
    altitudeControl( cbxName, true );
  }
}

// Altitude mode
void QgsKmlSettingsDialog::on_cbPointAltitudeMode_currentIndexChanged( const QString & )
{
  setAltitudeModeToolTip( m_ui->cbPointAltitudeMode );
}

void QgsKmlSettingsDialog::on_cbLineAltitudeMode_currentIndexChanged( const QString & )
{
  setAltitudeModeToolTip( m_ui->cbLineAltitudeMode );
}

void QgsKmlSettingsDialog::on_cbPolyAltitudeMode_currentIndexChanged( const QString & )
{
  setAltitudeModeToolTip( m_ui->cbPolyAltitudeMode );
}

// Transparence - opacity
void QgsKmlSettingsDialog::on_sbxLabelOpacity_valueChanged( int value )
{
  int iOpacityPers;
  QColor btnColor;

  iOpacityPers = value / 100.0 * 255.0;
  btnColor = getButtonColor( m_ui->pbLabelColor );
  btnColor.setAlpha( iOpacityPers );
  setButtonColor( m_ui->pbLabelColor, btnColor );
}

void QgsKmlSettingsDialog::on_sbxIconOpacity_valueChanged( int value )
{
  int iOpacityPers;
  QColor btnColor;

  iOpacityPers = value / 100.0 * 255.0;
  btnColor = getButtonColor( m_ui->pbIconColor );
  btnColor.setAlpha( iOpacityPers );
  setButtonColor( m_ui->pbIconColor, btnColor );
}

void QgsKmlSettingsDialog::on_sbxLineOpacity_valueChanged( int value )
{
  int iOpacityPers;
  QColor btnColor;

  iOpacityPers = value / 100.0 * 255.0;
  btnColor = getButtonColor( m_ui->pbLineColor );
  btnColor.setAlpha( iOpacityPers );
  setButtonColor( m_ui->pbLineColor, btnColor );
}

void QgsKmlSettingsDialog::on_sbxPolyOpacity_valueChanged( int value )
{
  int iOpacityPers;
  QColor btnColor;

  iOpacityPers = value / 100.0 * 255.0;
  btnColor = getButtonColor( m_ui->pbPolyColor );
  btnColor.setAlpha( iOpacityPers );
  setButtonColor( m_ui->pbPolyColor, btnColor );
}

void QgsKmlSettingsDialog::on_chbOverrideLayerStyle_toggled( bool checked )
{
  m_ui->tabWidget->setEnabled( checked );
}
