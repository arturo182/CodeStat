#ifndef WNDMAIN_H
#define WNDMAIN_H

#include <QtGui/QMainWindow>

class QLabel;
class QTreeWidgetItem;

class wndFilters;

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

    QStringList fileList(const QString &fileDir, const QStringList &filters, const QStringList &excludes);
    QString calcFileSize(qint64 bytes);

  private slots:
    void slotResizeStats(int index, int oldSize, int newSize);
    void on_btnDirectoriesAdd_clicked();
    void on_btnStart_clicked();
    void on_twDirectories_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_btnDirectoriesRemove_clicked();
    void on_twStats_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_btnExportSave_clicked();
    void on_btnFiltersNew_clicked();
    void on_twFilters_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_btnFiltersEdit_clicked();

    void on_twFilters_itemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    Ui::wndMain *m_ui;
    wndFilters *m_wndFilters;
};

#endif // WNDMAIN_H
