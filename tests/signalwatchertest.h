#ifndef SIGNALWATCHERTEST_H
#define SIGNALWATCHERTEST_H

#include <QtCore/QObject>

class SignalWatcherTest : public QObject {
	Q_OBJECT
public:
	explicit SignalWatcherTest(QObject* parent = 0);

private Q_SLOTS:
	void initTestCase(void);
	void testInterface(void);
	void testImplementation(void);
	void testBug1(void);
};

#endif // SIGNALWATCHERTEST_H
