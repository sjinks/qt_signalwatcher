# qt_signalwatcher [![Build Status](https://secure.travis-ci.org/sjinks/qt_signalwatcher.png)](http://travis-ci.org/sjinks/qt_signalwatcher)

A replacement for `QCoreApplication::watchUnixSignal()` and `QCoreApplication::unixSignal()`.

## Requirements

* Qt >= 4.2

## Build

Assuming that the current directory is the one with `signalwatcher.pro`:

```bash
qmake signalwatcher.pro && make
```

Some systems have `qmake-qt4` instead of `qmake`.

The above commands will generate the static library and `.prl` file in `../lib` directory and `test/tst_signalwatchertest` (this binary can be used to verify that the code works as expected).

## Install

After completing Build step run

*nix:
```
sudo make install
```

Windows:
```
nmake install
```

For Windows this will copy `signalwatcher.h` to `../lib` directory.
For *nix this will install `signalwatcher.h` to `/usr/include`, `libsignalwatcher.a` and `libsignalwatcher.prl` to `/usr/lib`, `libsignalwatcher.pc` to `/usr/lib/pkgconfig`.


## Usage

```c++
#include "signalwatcher.h"

/* ... */

SignalWatcher* w = SignalWatcher::instance();
QObject::connect(w, SIGNAL(unixSignal(int)), this, SLOT(mySigTermHandler()));
w->watch(SIGTERM);

/* ... */

void mySigTermHandler(void)
{
    SignalWatcher* w = SignalWatcher::instance();
    w->unwatch(SIGTERM);
    QCoreApllication::quit();
}
```

Add these lines to the project file:

```
CONFIG += link_prl
LIBS   += -L/path/to/libsignalwatcher -lsignalwatcher
```

On *nix you can use this alternative (provided that the library was properly installed):

```
CONFIG    += link_pkgconfig
PKGCONFIG += signalwatcher
```
