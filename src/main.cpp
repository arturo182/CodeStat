#include "mainwindow.h"

#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	//cheat to get portable settings
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, qApp->applicationDirPath());
	QSettings::setDefaultFormat(QSettings::IniFormat);
	QApplication::setOrganizationName("xxx/../");
	QApplication::setApplicationName("settings");

	MainWindow wnd;
	wnd.show();

	return app.exec();
}
