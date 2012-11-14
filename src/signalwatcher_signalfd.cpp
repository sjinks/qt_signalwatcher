#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/signalfd.h>
#include "signalwatcher.h"
#include "helpers_p.h"

class SignalWatcherPrivate {
public:
	~SignalWatcherPrivate(void);

private:
	static SignalWatcher* instance;
	static QSocketNotifier* notifier;
	static int fd;
	static bool map[NSIG];

	Q_DISABLE_COPY(SignalWatcherPrivate)
	Q_DECLARE_PUBLIC(SignalWatcher)
	SignalWatcher* const q_ptr;

	SignalWatcherPrivate(SignalWatcher* const w);
	bool watch(int sig);
	bool unwatch(int sig);
	void _q_handleSignal(void);

	static int signalfd_wrapper(int fd, const sigset_t* mask);
};

SignalWatcher* SignalWatcherPrivate::instance   = 0;
QSocketNotifier* SignalWatcherPrivate::notifier = 0;
int SignalWatcherPrivate::fd                    = -1;
bool SignalWatcherPrivate::map[NSIG];

SignalWatcherPrivate::SignalWatcherPrivate(SignalWatcher* const w)
	: q_ptr(w)
{
	memset(SignalWatcherPrivate::map, 0, sizeof(SignalWatcherPrivate::map));
}

SignalWatcherPrivate::~SignalWatcherPrivate(void)
{
	my_close(SignalWatcherPrivate::fd);
	SignalWatcherPrivate::notifier = 0;
}

#if defined(SFD_CLOEXEC) && defined(SFD_NONBLOCK)
#	define MY_SFD_CLOEXEC  SFD_CLOEXEC
#	define MY_SFD_NONBLOCK SFD_NONBLOCK
#else
#	define MY_SFD_CLOEXEC 0
#	define MY_SFD_NONBLOCK 0
#endif

int SignalWatcherPrivate::signalfd_wrapper(int fd, const sigset_t* mask)
{
	int flags = MY_SFD_CLOEXEC | MY_SFD_NONBLOCK;
	int res   = signalfd(fd, mask, flags);
	if (-1 == res) {
		if (EINVAL == errno && flags) {
			res = signalfd(fd, mask, 0);
		}
	}
	else {
		return res;
	}

	if (res == -1) {
		qErrnoWarning("%s: signalfd() failed", Q_FUNC_INFO);
		return -1;
	}

	fd  = res;
	if (-1 == set_cloexec(fd)) {
		qErrnoWarning("%s: Unable to set close on exec flag", Q_FUNC_INFO);
	}

	if (-1 == make_nonblocking(fd)) {
		qErrnoWarning("%s: Unable to make the descriptor non-blocking", Q_FUNC_INFO);
		return -1;
	}

	return fd;
}

#undef MY_SFD_CLOEXEC
#undef MY_SFD_NONBLOCK

bool SignalWatcherPrivate::watch(int sig)
{
	if (sig < NSIG) {
		if (map[sig]) {
			// We have already installed a handler for this signal
			return false;
		}

		sigset_t old;
		sigset_t mask;
		sigprocmask(SIG_BLOCK, 0, &mask);
		sigaddset(&mask, sig);
		sigprocmask(SIG_SETMASK, &mask, &old);
		int f = SignalWatcherPrivate::signalfd_wrapper(SignalWatcherPrivate::fd, &mask);
		if (-1 == f) {
			qErrnoWarning("%s: signalfd() failed", Q_FUNC_INFO);
			sigprocmask(SIG_SETMASK, &old, 0);
			return false;
		}

		SignalWatcherPrivate::map[sig] = true;

		if (-1 == SignalWatcherPrivate::fd) {
			Q_ASSERT(SignalWatcherPrivate::notifier == 0);
			Q_Q(SignalWatcher);

			SignalWatcherPrivate::fd = f;
			SignalWatcherPrivate::notifier = new QSocketNotifier(f, QSocketNotifier::Read, q);

			QObject::connect(SignalWatcherPrivate::notifier, SIGNAL(activated(int)), q, SLOT(_q_handleSignal()));
		}

		return true;
	}

	return false;
}

bool SignalWatcherPrivate::unwatch(int sig)
{
	if (sig < NSIG && -1 != SignalWatcherPrivate::fd) {
		Q_ASSERT(SignalWatcherPrivate::notifier != 0);

		sigset_t mask;
		sigprocmask(SIG_UNBLOCK, 0, &mask);
		sigdelset(&mask, sig);
		sigprocmask(SIG_SETMASK, &mask, 0);
		map[sig] = false;
		int f = SignalWatcherPrivate::signalfd_wrapper(SignalWatcherPrivate::fd, &mask);
		return f != -1;
	}

	return false;
}

void SignalWatcherPrivate::_q_handleSignal(void)
{
	Q_Q(SignalWatcher);
	QSocketNotifier* sn = qobject_cast<QSocketNotifier*>(q->sender());
	Q_ASSERT(sn != 0);

	sn->setEnabled(false);

	struct signalfd_siginfo info;
	if (safe_read(SignalWatcherPrivate::fd, &info, sizeof(info)) != sizeof(info)) {
		qErrnoWarning("%s: read() failed", Q_FUNC_INFO);
	}

	Q_EMIT q->unixSignal(info.ssi_signo);
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

const char* SignalWatcher::backend(void)
{
	return "signalfd";
}

#include "moc_signalwatcher.cpp"
