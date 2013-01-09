TEMPLATE = subdirs
CONFIG  += ordered

SUBDIRS += \
    src \
    tests

src.file   = src/signalwatcher.pro
tests.file = tests/signalwatcher-test.pro
