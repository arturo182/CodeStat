#include "mainwindow.h"

#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
	QSettings::setDefaultFormat(QSettings::IniFormat);

	QApplication app(argc, argv);

	//cheat to get portable settings
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, qApp->applicationDirPath());
	QApplication::setOrganizationName("data");
	QApplication::setApplicationName("settings");

	MainWindow wnd;
	wnd.show();

	return app.exec();
}
