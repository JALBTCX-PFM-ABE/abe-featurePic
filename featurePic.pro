contains(QT_CONFIG, opengl): QT += opengl
QT += 
RESOURCES = icons.qrc
INCLUDEPATH += /c/PFM_ABEv7.0.0_Win64/include
LIBS += -L /c/PFM_ABEv7.0.0_Win64/lib -lBinaryFeatureData -lnvutility -lgdal -lxml2 -lpoppler -ljpeg -liconv
DEFINES += WIN32 NVWIN3X
CONFIG += console
CONFIG += static

#
# The following line is included so that the contents of acknowledgments.hpp will be available for translation
#

HEADERS += /c/PFM_ABEv7.0.0_Win64/include/acknowledgments.hpp

QMAKE_LFLAGS += 
######################################################################
# Automatically generated by qmake (2.01a) Wed Jan 22 14:11:07 2020
######################################################################

TEMPLATE = app
TARGET = featurePic
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += featurePic.hpp featurePicHelp.hpp version.hpp
SOURCES += featurePic.cpp main.cpp
RESOURCES += icons.qrc
TRANSLATIONS += featurePic_xx.ts
