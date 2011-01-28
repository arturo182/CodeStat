#ifndef WNDMAIN_H
#define WNDMAIN_H

#include <QtGui/QMainWindow>

class QLabel;

namespace Ui
{
  class wndMain;
}

class wndMain: public QMainWindow
{
  Q_OBJECT

  public:
    explicit wndMain(QWidget *parent = 0);
    ~wndMain();

    void loadFilters();
    void loadExports();

    QStringList getFileList(const QString &fileDir, const QStringList &filters);

  private slots:
    void on_btnDirectoriesAdd_clicked();
    void on_btnStart_clicked();

private:
    Ui::wndMain *m_ui;

};

#endif // WNDMAIN_H
