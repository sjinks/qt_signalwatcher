QT      -= gui
TARGET   = signalwatcher
TEMPLATE = lib
CONFIG  += staticlib create_prl release
DESTDIR  = ../lib

HEADERS += \
	signalwatcher.h \
	helpers_p.h \
	qt4compat.h

headers.files = signalwatcher.h

unix:!symbian {
	system('cc -E $$PWD/conftests/signalfd.h -o /dev/null 2> /dev/null'):     SOURCES += signalwatcher_signalfd.cpp
	else:system('cc -E $$PWD/conftests/eventfd.h -o /dev/null 2> /dev/null'): SOURCES += signalwatcher_eventfd.cpp
	else:                                                                     SOURCES += signalwatcher_pipe.cpp
}
else {
	SOURCES += signalwatcher_none.cpp
}

unix {
	CONFIG += create_pc
	headers.path = /usr/include

	QMAKE_PKGCONFIG_NAME        = signalwatcher
	QMAKE_PKGCONFIG_DESCRIPTION = "Signal Watcher for Qt"
	QMAKE_PKGCONFIG_LIBDIR      = $$target.path
	QMAKE_PKGCONFIG_INCDIR      = $$headers.path
	QMAKE_PKGCONFIG_DESTDIR     = pkgconfig
}
else {
	headers.path = $$DESTDIR
}

unix:!symbian {
	maemo5 {
		target.path = /opt/usr/lib
	}
	else {
		target.path = /usr/lib
	}
}

INSTALLS += target headers
