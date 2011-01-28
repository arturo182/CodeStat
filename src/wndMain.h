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

    QStringList fileList(const QString &fileDir, const QStringList &filters);
    QString calcFileSize(qint64 bytes);

  private slots:
    void on_btnDirectoriesAdd_clicked();
    void on_btnStart_clicked();

private:
    Ui::wndMain *m_ui;

};

#endif // WNDMAIN_H
