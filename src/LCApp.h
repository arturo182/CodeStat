#ifndef LCAPP_H
#define LCAPP_H

#include <QtGui/QApplication>

class wndMain;

class LCApp: public QApplication
{
  Q_OBJECT

  public:
    LCApp(int &argc, char **argv);

  private slots:
    void slotAlmostQuit();

  private:
    wndMain *m_wndMain;
};

#endif // LCAPP_H
