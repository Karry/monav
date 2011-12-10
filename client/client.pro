# -------------------------------------------------
# Project created by QtCreator 2010-06-15T15:30:10
# -------------------------------------------------

TEMPLATE = app
QT += svg xml
DESTDIR = ../bin
TARGET = monav

# Activate this to run tests
DEFINES += CPPUNITLITE

# Enable this to compile on desktop machines without QtMobility installed
# DEFINES += NOQTMOBILE

TRANSLATIONS += ../translations/de_DE.ts
RESOURCES += translations.qrc
RESOURCES += images.qrc
RESOURCES += audio.qrc

# Required by osmrendererclient
QT += network
QT += multimedia
CONFIG += mobility
MOBILITY += location
# Required to get a non-debug build (at least on Windows)
# CONFIG += release

# Required to get qDebug() output on the Mac
# CONFIG += console

INCLUDEPATH += ..

SOURCES += main.cpp \
	paintwidget.cpp \
	addressdialog.cpp \
	bookmarksdialog.cpp \
	routedescriptiondialog.cpp \
	mapdata.cpp \
	routinglogic.cpp \
	instructiongenerator.cpp \
	overlaywidget.cpp \
	scrollarea.cpp \
	gpsdialog.cpp \
	generalsettingsdialog.cpp \
	logger.cpp \
	../utils/directoryunpacker.cpp \
	../utils/lzma/LzmaDec.c \
	mappackageswidget.cpp \
	mainwindow.cpp \
	mapmoduleswidget.cpp \
	placechooser.cpp \
	globalsettings.cpp \
	streetchooser.cpp \
	 gpsdpositioninfosource.cpp \
	 json.cpp \
	tripinfodialog.cpp \
	worldmapchooser.cpp \
	audio.cpp

HEADERS += \
	paintwidget.h \
	../utils/coordinates.h \
	../utils/config.h \
	../interfaces/irenderer.h \
	../interfaces/iaddresslookup.h \
	addressdialog.h \
	../interfaces/igpslookup.h \
	../interfaces/irouter.h \
	bookmarksdialog.h \
	routedescriptiondialog.h \
	descriptiongenerator.h \
	mapdata.h \
	routinglogic.h \
	instructiongenerator.h \
	fullscreenexitbutton.h \
	overlaywidget.h \
	scrollarea.h \
	gpsdialog.h \
	generalsettingsdialog.h \
	logger.h \
	../utils/directoryunpacker.h \
	../utils/lzma/LzmaDec.h \
	mappackageswidget.h \
	mainwindow.h \
	mapmoduleswidget.h \
	placechooser.h \
	globalsettings.h \
	streetchooser.h \
	json.h \
	gpsdpositioninfosource.h \
	tripinfodialog.h \
	worldmapchooser.h \
	audio.h

FORMS += \
	paintwidget.ui \
	addressdialog.ui \
	bookmarksdialog.ui \
	routedescriptiondialog.ui \
	gpsdialog.ui \
	generalsettingsdialog.ui \
	mappackageswidget.ui \
	mainwindow.ui \
	mapmoduleswidget.ui \
	placechooser.ui \
	streetchooser.ui \
	tripinfodialog.ui

# TDD stuff
	SOURCES += \
	../CppUnitLite/TestResult.cpp \
	../CppUnitLite/TestRegistry.cpp \
	../CppUnitLite/TestHarness.cpp \
	../CppUnitLite/Test.cpp \
	../CppUnitLite/SimpleString.cpp \
	../CppUnitLite/Failure.cpp \
	../CppUnitLite/CppUnitLite.cpp

HEADERS  += \
	../CppUnitLite/TestResult.h \
	../CppUnitLite/TestRegistry.h \
	../CppUnitLite/TestHarness.h \
	../CppUnitLite/Test.h \
	../CppUnitLite/SimpleString.h \
	../CppUnitLite/Failure.h \
	../CppUnitLite/CppUnitLite.h

LIBS += -L../bin/plugins_client -lmapnikrendererclient -lcontractionhierarchiesclient -lgpsgridclient -losmrendererclient -lunicodetournamenttrieclient -lqtilerendererclient

unix {
	QMAKE_CXXFLAGS_RELEASE -= -O2
	QMAKE_CXXFLAGS_RELEASE += -O3 \
		-Wno-unused-function
	QMAKE_CXXFLAGS_DEBUG += -Wno-unused-function
}

maemo5 {
	QT += maemo5
}

unix:!symbian {
	maemo5 {
		target.path = /opt/usr/bin
	} else {
		target.path = /usr/local/bin
		DEFINES += NOQTMOBILE
	}
	INSTALLS += target
}

macx {
	ICON = ../images/AppIcons.icns
	DEFINES += NOQTMOBILE
}

win32 {
	RC_FILE = ../images/WindowsResources.rc
	DEFINES += NOQTMOBILE
}
