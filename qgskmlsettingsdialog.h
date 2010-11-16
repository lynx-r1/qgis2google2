#ifndef QGSKMLSETTINGSDIALOG_H
#define QGSKMLSETTINGSDIALOG_H

#include <QtGui/QDialog>
#include <QSettings>

#include <qgis.h>

class QComboBox;
class QLabel;
class QSpinBox;

namespace Ui {
  class QgsKmlSettingsDialog;
}

class QgsKmlSettingsDialog : public QDialog {
  Q_OBJECT
public:
  QgsKmlSettingsDialog(QWidget *parent = 0, QGis::GeometryType typeOfFeature = QGis::Point );
  ~QgsKmlSettingsDialog();

protected:
  void changeEvent(QEvent *e);

private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();

  void on_pbIconColor_clicked();
  void on_pbLabelColor_clicked();
  void on_pbLineColor_clicked();
  void on_pbPolyColor_clicked();

  void on_cbPointExtrude_currentIndexChanged( const QString &text );
  void on_cbLineExtrude_currentIndexChanged( const QString &text );
  void on_cbPolyExtrude_currentIndexChanged( const QString &text );

  void on_cbLineTessellate_currentIndexChanged( const QString &text );
  void on_cbPolyTessellate_currentIndexChanged( const QString &text );

  void on_cbPointAltitudeMode_currentIndexChanged( const QString & );
  void on_cbLineAltitudeMode_currentIndexChanged( const QString & );
  void on_cbPolyAltitudeMode_currentIndexChanged( const QString & );

  void on_sbxLabelOpacity_valueChanged( int value );
  void on_sbxIconOpacity_valueChanged( int value );
  void on_sbxLineOpacity_valueChanged( int value );
  void on_sbxPolyOpacity_valueChanged( int value );

  void on_chbOverrideLayerStyle_toggled( bool checked );

private:
  void initComboBoxes();
  void setTypeOfTab( QGis::GeometryType typeOfFeature );

  void readSettings();
  void writeSettings();

  void enableAltitudeControl( QLabel *label, QSpinBox *spinBox, bool enable );
  void altitudeControl( QString str, bool enable );

  int qstringToBool( QString boolStr );
  QString boolToQString ( int boolVal );

  QColor getButtonColor( QPushButton *button );
  void setButtonColor( QPushButton *button, QColor &color );

  void extrudeIndexChange( QComboBox *comboBox, QString text );
  void tessellateIndexChange( QComboBox *comboBox, QString text );

  void setAltitudeItemsData( QComboBox *comboBox );
  void setColorModeItemsData( QComboBox *comboBox );

  void setAltitudeModeToolTip( QComboBox *comboBox );

  Ui::QgsKmlSettingsDialog *m_ui;
};

#endif // QGSKMLSETTINGSDIALOG_H
