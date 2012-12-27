QT      -= gui
TARGET   = signalwatcher
TEMPLATE = lib
CONFIG  += staticlib create_prl release
DESTDIR  = ../lib

HEADERS += \
	signalwatcher.h \
	helpers_p.h

headers.files = signalwatcher.h

unix:!symbian {
	exists(/usr/include/sys/signalfd.h):     SOURCES += signalwatcher_signalfd.cpp
	else:exists(/usr/include/sys/eventfd.h): SOURCES += signalwatcher_eventfd.cpp
	else:                                    SOURCES += signalwatcher_pipe.cpp
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
