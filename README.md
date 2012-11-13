qt_signalwatcher
================

A replacement for QCoreApplication::watchUnixSignal() and
QCoreApplication::unixSignal().

**Usage**

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

