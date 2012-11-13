qt_signalwatcher
================

A replacement for `QCoreApplication::watchUnixSignal()` and `QCoreApplication::unixSignal()`.

**Build**

Assuming that the current directory is the one with `signalwatcher.pro`:

```bash
qmake signalwatcher.pro && make
```

Some systems have `qmake-qt4` instead of `qmake`.

The above command will generate `src/libsigwatch.a` (the library itself), `src/libsigwatch.prl` (this file is used by Qt to link the library properly) and `test/tst_signalwatchertest` (this binary can be used to verify that the code works as expected).

**Usage**

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
CONFIG += use_prl
LIBS   += -L/path/to/libsignalwatcher -lsignalwatcher
```
