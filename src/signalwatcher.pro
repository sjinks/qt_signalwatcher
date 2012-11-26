QT      -= gui
TARGET   = signalwatcher
TEMPLATE = lib
CONFIG  += staticlib release

HEADERS += \
	signalwatcher.h \
	helpers_p.h

unix:!symbian {
	maemo5 {
		target.path = /opt/usr/lib
	} else {
		target.path = /usr/lib
	}
	INSTALLS += target
}

unix:!symbian {
	exists(/usr/include/sys/signalfd.h):     SOURCES += signalwatcher_signalfd.cpp
	else:exists(/usr/include/sys/eventfd.h): SOURCES += signalwatcher_eventfd.cpp
	else:                                    SOURCES += signalwatcher_pipe.cpp
}
else {
	SOURCES += signalwatcher_none.cpp
}
