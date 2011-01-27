#ifndef LCAPP_H
#define LCAPP_H

#include <QtGui/QApplication>

class LCApp: public QApplication
{
  Q_OBJECT

  public:
    LCApp(int &argc, char **argv);

  signals:

  public slots:

};

#endif // LCAPP_H
