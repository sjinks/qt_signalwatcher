#ifndef SIGNALWATCHER_H
#define SIGNALWATCHER_H

#include <QtCore/QObject>

class SignalWatcherPrivate;

class SignalWatcher : public QObject {
	Q_OBJECT
public:
	static SignalWatcher* instance(void);
	virtual ~SignalWatcher(void);
	bool watch(int sig);
	bool unwatch(int sig);
	bool watch(int sig, bool watch);

	static const char* backend(void);

Q_SIGNALS:
	void unixSignal(int);

private:
	SignalWatcher(void);
	Q_DISABLE_COPY(SignalWatcher)

#if defined(Q_OS_UNIX) && !defined(Q_OS_SYMBIAN)
	Q_DECLARE_PRIVATE(SignalWatcher)
#if QT_VERSION >= 0x040600
	QScopedPointer<SignalWatcherPrivate> d_ptr;
#else
	SignalWatcherPrivate* d_ptr;
#endif
	Q_PRIVATE_SLOT(d_func(), void _q_handleSignal(void))
#endif
};

#endif // SIGNALWATCHER_H
