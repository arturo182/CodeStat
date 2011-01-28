#include "LCApp.h"

#include "wndMain.h"

#include <QtCore/QSettings>
#include <QtCore/QTextCodec>

LCApp::LCApp(int &argc, char **argv): QApplication(argc, argv)
{
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, qApp->applicationDirPath());
  QCoreApplication::setOrganizationName("data/settings");

  connect(this, SIGNAL(aboutToQuit()), this, SLOT(slotAlmostQuit()));

  m_wndMain = new wndMain();
}

void LCApp::slotAlmostQuit()
{
  delete m_wndMain;
}

