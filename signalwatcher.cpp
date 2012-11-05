#include "signalwatcher.h"

#if defined(Q_OS_UNIX) && !defined(Q_OS_SYMBIAN)

#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <QtCore/QVariant>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>

class SignalWatcherPrivate {
public:
	~SignalWatcherPrivate(void);

private:
	struct sigdata_t {
		QSocketNotifier* sn;
		int fd[2];
	};

	static SignalWatcher* instance;
	static SignalWatcherPrivate::sigdata_t signal_data[NSIG];

	Q_DISABLE_COPY(SignalWatcherPrivate)
	Q_DECLARE_PUBLIC(SignalWatcher)
	SignalWatcher* const q_ptr;

	SignalWatcherPrivate(SignalWatcher* const w);
	static void signalHandler(int sig);
	bool watch(int sig);
	bool unwatch(int sig);
	void _q_handleSignal(void);
};

SignalWatcher* SignalWatcherPrivate::instance = 0;
SignalWatcherPrivate::sigdata_t SignalWatcherPrivate::signal_data[NSIG];

SignalWatcherPrivate::SignalWatcherPrivate(SignalWatcher* const w)
	: q_ptr(w)
{
	memset(SignalWatcherPrivate::signal_data, 0, sizeof(SignalWatcherPrivate::signal_data));
}

SignalWatcherPrivate::~SignalWatcherPrivate(void)
{
	for (int i=0; i<NSIG; ++i) {
		if (SignalWatcherPrivate::signal_data[i].sn) {
			this->unwatch(i);
		}
	}
}

bool SignalWatcherPrivate::watch(int sig)
{
	if (sig < NSIG) {
		if (SignalWatcherPrivate::signal_data[sig].sn) {
			// We have already installed a handler for this signal
			return false;
		}

		if (::socketpair(AF_UNIX, SOCK_STREAM, 0, SignalWatcherPrivate::signal_data[sig].fd)) {
			qCritical("Failed to create a socket pair: %s", strerror(errno));
			return false;
		}

		Q_Q(SignalWatcher);
		QSocketNotifier* n = new QSocketNotifier(SignalWatcherPrivate::signal_data[sig].fd[1], QSocketNotifier::Read, q);
		n->setProperty("sig", QVariant::fromValue(sig));
		QObject::connect(n, SIGNAL(activated(int)), q, SLOT(_q_handleSignal()));
		SignalWatcherPrivate::signal_data[sig].sn = n;

		struct sigaction sa;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags   = SA_RESTART;
		sa.sa_handler = SignalWatcherPrivate::signalHandler;
		sigaction(sig, &sa, 0);
		return true;
	}

	return false;
}

bool SignalWatcherPrivate::unwatch(int sig)
{
	if (sig < NSIG) {
		struct sigaction sa;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags   = 0;
		sa.sa_handler = SIG_DFL;

		if (SignalWatcherPrivate::signal_data[sig].sn) {
			delete SignalWatcherPrivate::signal_data[sig].sn;
			SignalWatcherPrivate::signal_data[sig].sn = 0;
			::close(SignalWatcherPrivate::signal_data[sig].fd[0]);
			::close(SignalWatcherPrivate::signal_data[sig].fd[1]);
		}

		sigaction(sig, &sa, 0);
		return true;
	}

	return false;
}

void SignalWatcherPrivate::signalHandler(int sig)
{
	Q_ASSERT(sn < NSIG);
	if (SignalWatcherPrivate::signal_data[sig].sn) {
		char a = 1;
		if (sizeof(a) != ::write(SignalWatcherPrivate::signal_data[sig].fd[0], &a, sizeof(a))) {
			Q_ASSERT(false);
		}
	}
}

void SignalWatcherPrivate::_q_handleSignal(void)
{
	Q_Q(SignalWatcher);
	QSocketNotifier* sn = qobject_cast<QSocketNotifier*>(q->sender());
	Q_ASSERT(sn != 0);

	sn->setEnabled(false);

	QVariant sig = sn->property("sig");
	Q_ASSERT(sig.isValid());

	int signo = sig.toInt();
	Q_ASSERT(signo >= 0 && signo < NSIG);
	Q_ASSERT(SignalWatcherPrivate::signal_data[signo].sn == sn);

	char tmp;
	if (sizeof(tmp) != ::read(SignalWatcherPrivate::signal_data[signo].fd[1], &tmp, sizeof(tmp))) {
		Q_ASSERT(false);
	}

	Q_EMIT q->unixSignal(signo);
	sn->setEnabled(true);
}

SignalWatcher::SignalWatcher(void)
	: QObject(qApp), d_ptr(new SignalWatcherPrivate(this))
{
}

bool SignalWatcher::watch(int sig)
{
	Q_D(SignalWatcher);
	return d->watch(sig);
}

bool SignalWatcher::unwatch(int sig)
{
	Q_D(SignalWatcher);
	return d->unwatch(sig);
}

#else

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

#endif // defined(Q_OS_UNIX) && !defined(Q_OS_SYMBIAN)

SignalWatcher::~SignalWatcher(void)
{
	SignalWatcherPrivate::instance = 0;
}

SignalWatcher* SignalWatcher::instance(void)
{
	if (!SignalWatcherPrivate::instance) {
		SignalWatcherPrivate::instance = new SignalWatcher();
	}

	return SignalWatcherPrivate::instance;
}

#if defined(Q_OS_UNIX) && !defined(Q_OS_SYMBIAN)
#	include "moc_signalwatcher.cpp"
#endif
