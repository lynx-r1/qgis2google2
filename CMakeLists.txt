
########################################################
# Files

SET (qgis2google_SRCS
     qgis2google.cpp
     qgsgoogleearthtool.cpp
     qgskmlconverter.cpp
     qgskmlsettingsdialog.cpp
)

SET (qgis2google_MOC_HDRS
     qgis2google.h
     qgsgoogleearthtool.h
     qgskmlconverter.h
     qgskmlsettingsdialog.h
)

SET (qgis2google_RCCS  qgis2google.qrc)

SET (qgis2google_UIS qgskmlsettingsdialogbase.ui )

FILE (GLOB TRANSLATIONS_FILES translations/*.ts)

########################################################
# Build

QT4_WRAP_UI (qgis2google_UIS_H  ${qgis2google_UIS})

QT4_WRAP_CPP (qgis2google_MOC_SRCS  ${qgis2google_MOC_HDRS})

QT4_ADD_RESOURCES (qgis2google_RCC_SRCS ${qgis2google_RCCS})

QT4_ADD_TRANSLATION (QM_FILE ${TRANSLATIONS_FILES})

ADD_LIBRARY (qgis2googleplugin MODULE
    ${qgis2google_SRCS} 
    ${qgis2google_MOC_SRCS} 
    ${qgis2google_RCC_SRCS} 
    ${qgis2google_UIS_H}
    ${qgis2google_TRANS_QMS}
)

ADD_CUSTOM_TARGET (qgis2google_TARGET ALL DEPENDS ${QM_FILES})

ADD_DEPENDENCIES (qgis2google_TARGET qgis2googleplugin)

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(qgis2googleplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS qgis2googleplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

