TEMPLATE = subdirs
CONFIG  += ordered

SUBDIRS += \
    src \
    test

src.file  = src/signalwatcher.pro
test.file = test/signalwatcher-test.pro
