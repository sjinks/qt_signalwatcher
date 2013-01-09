#include <QtCore/QCoreApplication>
#include <QtTest/QtTest>
#include "signalwatchertest.h"

int main(int argc, char** argv)
{
	QCoreApplication app(argc, argv);
	SignalWatcherTest t;
	return QTest::qExec(&t, argc, argv);
}
