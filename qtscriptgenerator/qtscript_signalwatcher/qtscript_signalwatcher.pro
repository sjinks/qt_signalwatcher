TARGET   = qtscript_signalwatcher
QT      += script
QT      -= gui
CONFIG  += plugin release
TEMPLATE = lib

SOURCES += ../generated_cpp/signalwatcher/plugin.cpp
exists(../generated_cpp/signalwatcher/plugin.h) {
	HEADERS += ../generated_cpp/signalwatcher/plugin.h
}

INCLUDEPATH += ../../src/
LIBS        += -L../../lib -lsignalwatcher

include(../generated_cpp/signalwatcher/signalwatcher.pri)
