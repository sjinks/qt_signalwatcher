QT      += testlib
QT      -= gui
TARGET   = tst_signalwatchertest
CONFIG  += console
CONFIG  -= app_bundle
TEMPLATE = app

HEADERS += \
	signalwatchertest.h \

SOURCES += \
	signalwatchertest.cpp \
	main.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += ../src
LIBS        += -L../lib -lsignalwatcher
