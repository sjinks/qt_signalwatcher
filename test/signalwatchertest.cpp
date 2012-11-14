#include <QtTest/QtTest>
#include <signal.h>
#include "signalwatcher.h"
#include "signalwatchertest.h"

SignalWatcherTest::SignalWatcherTest(QObject *parent) :
	QObject(parent)
{
}

void SignalWatcherTest::initTestCase(void)
{
	SignalWatcher* w = SignalWatcher::instance();
	if (!w) {
		QSKIP("Failed to instantiate SignalWatcher", SkipAll);
	}

	if (!qstrcmp("stub", w->backend())) {
		QSKIP("SignalWatcher is a stub", SkipAll);
	}

	qDebug("SignalWatcher uses '%s' backend", w->backend());
}

void SignalWatcherTest::testInterface(void)
{
	SignalWatcher* w = SignalWatcher::instance();
	QVERIFY(w != 0);

	QCOMPARE(w->watch(NSIG),      false);
	QCOMPARE(w->watch(SIGUSR1),   true);
	QCOMPARE(w->watch(SIGUSR1),   false);
	QCOMPARE(w->unwatch(SIGUSR1), true);
	QCOMPARE(w->unwatch(SIGUSR1), true);
	QCOMPARE(w->unwatch(SIGUSR2), true);
	QCOMPARE(w->unwatch(NSIG),    false);
}

void SignalWatcherTest::testImplementation(void)
{
	SignalWatcher* w = SignalWatcher::instance();
	QVERIFY(w != 0);

	QSignalSpy spy(w, SIGNAL(unixSignal(int)));

	// SIGCHLD is ignored by default
	QCOMPARE(::kill(getpid(), SIGCHLD), 0);
	QCoreApplication::processEvents();
	QCOMPARE(spy.count(), 0);

	QCOMPARE(w->watch(SIGCHLD), true);
	QCOMPARE(::kill(getpid(), SIGCHLD), 0);
	QCoreApplication::processEvents();
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.takeFirst().at(0).toInt(), SIGCHLD);

	QCOMPARE(w->unwatch(SIGCHLD), true);
	QCOMPARE(::kill(getpid(), SIGCHLD), 0);
	QCoreApplication::processEvents();
	QCOMPARE(spy.count(), 0);

	// SIGWINCH is ignored by default
	QCOMPARE(::kill(getpid(), SIGWINCH), 0);
	QCoreApplication::processEvents();
	QCOMPARE(spy.count(), 0);
}

void SignalWatcherTest::testBug1(void)
{
	SignalWatcher* w = SignalWatcher::instance();
	QVERIFY(w != 0);

	QSignalSpy spy(w, SIGNAL(unixSignal(int)));

	QCOMPARE(w->watch(SIGCHLD), true);
	QCOMPARE(w->watch(SIGWINCH), true);
	QCOMPARE(::kill(getpid(), SIGCHLD), 0);
	QCoreApplication::processEvents();
	QCOMPARE(spy.count(), 1);
	spy.clear();

	QCOMPARE(w->unwatch(SIGCHLD), true);
	QCOMPARE(::kill(getpid(), SIGWINCH), 0);
	QCoreApplication::processEvents();
	QCOMPARE(spy.count(), 1);

	QCOMPARE(w->unwatch(SIGWINCH), true);
}
