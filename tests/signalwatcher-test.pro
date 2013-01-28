QT      += testlib
QT      -= gui
TARGET   = tst_signalwatchertest
CONFIG  += console
CONFIG  -= app_bundle
TEMPLATE = app

lessThan(QT_MAJOR_VERSION, 5): CONFIG += qtestlib

HEADERS += \
	signalwatchertest.h \

SOURCES += \
	signalwatchertest.cpp \
	main.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += ../src
LIBS        += -L../lib -lsignalwatcher
