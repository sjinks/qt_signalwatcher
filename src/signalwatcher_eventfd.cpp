#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <QtCore/QVariant>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/eventfd.h>
#include "signalwatcher.h"
#include "helpers_p.h"

#if defined(EFD_CLOEXEC) && defined(EFD_NONBLOCK)
#	define MY_EFD_CLOEXEC EFD_CLOEXEC
#	define MY_EFD_NONBLOCK EFD_NONBLOCK
#else
#	define MY_EFD_CLOEXEC 0
#	define MY_EFD_NONBLOCK 0
#endif

class SignalWatcherPrivate {
public:
	~SignalWatcherPrivate(void);

private:
	struct sigdata_t {
		QSocketNotifier* sn;
		int fd;
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

static int create_comm_obj(int& fd)
{
	int flags = MY_EFD_CLOEXEC | MY_EFD_NONBLOCK;
	int res   = eventfd(0, flags);
	fd        = -1;

	if (-1 == res) {
		if (EINVAL == errno && flags) {
			res = eventfd(0, 0);
		}
	}
	else {
		fd = res;
		return 0;
	}

	if (res == -1) {
		qErrnoWarning("%s: eventfd() failed", Q_FUNC_INFO);
		return -1;
	}

	fd = res;
	if (-1 == set_cloexec(fd)) {
		qErrnoWarning("%s: Unable to set close on exec flag", Q_FUNC_INFO);
	}

	if (-1 == make_nonblocking(fd)) {
		qErrnoWarning("%s: Unable to make the descriptor non-blocking", Q_FUNC_INFO);
		return -1;
	}

	return 0;
}

#undef MY_EFD_CLOEXEC
#undef MY_EFD_NONBLOCK

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

		if (create_comm_obj(SignalWatcherPrivate::signal_data[sig].fd)) {
			my_close(SignalWatcherPrivate::signal_data[sig].fd);
			return false;
		}

		Q_Q(SignalWatcher);
		QSocketNotifier* n = new QSocketNotifier(SignalWatcherPrivate::signal_data[sig].fd, QSocketNotifier::Read, q);
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
			my_close(SignalWatcherPrivate::signal_data[sig].fd);
		}

		sigaction(sig, &sa, 0);
		return true;
	}

	return false;
}

void SignalWatcherPrivate::signalHandler(int sig)
{
	Q_ASSERT(sig < NSIG);
	if (SignalWatcherPrivate::signal_data[sig].sn) {
		eventfd_t value = 1;
		if (safe_write(SignalWatcherPrivate::signal_data[sig].fd, &value, sizeof(value)) != sizeof(value)) {
			qErrnoWarning("%s: write() failed", Q_FUNC_INFO);
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

	eventfd_t value;
	if (safe_read(SignalWatcherPrivate::signal_data[signo].fd, &value, sizeof(value)) != sizeof(value)) {
		qErrnoWarning("%s: read() failed", Q_FUNC_INFO);
	}

	Q_EMIT q->unixSignal(signo);
	sn->setEnabled(true);
}

SignalWatcher::SignalWatcher(void)
	: QObject(QCoreApplication::instance()), d_ptr(new SignalWatcherPrivate(this))
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

SignalWatcher::~SignalWatcher(void)
{
	SignalWatcherPrivate::instance = 0;

#if QT_VERSION < 0x040600
	delete this->d_ptr;
	this->d_ptr = 0;
#endif
}

SignalWatcher* SignalWatcher::instance(void)
{
	if (!SignalWatcherPrivate::instance) {
		SignalWatcherPrivate::instance = new SignalWatcher();
	}

	return SignalWatcherPrivate::instance;
}

const char* SignalWatcher::backend(void)
{
	return "eventfd";
}

#include "moc_signalwatcher.cpp"
