#include <QtCore/QCoreApplication>
#include "signalwatcher.h"

class SignalWatcherPrivate {}

static SignalWatcher* g_instance = 0;

SignalWatcher::SignalWatcher(void)
	: QObject(qApp)
{
}

bool SignalWatcher::watch(int sig)
{
	Q_UNUSED(sig)
	return false;
}

bool SignalWatcher::unwatch(int sig)
{
	Q_UNUSED(sig)
	return false;
}

SignalWatcher::~SignalWatcher(void)
{
	g_instance = 0;
}

SignalWatcher* SignalWatcher::instance(void)
{
	if (!g_instance) {
		g_instance = new SignalWatcher();
	}

	return g_instance;
}
