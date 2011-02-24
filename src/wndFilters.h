#ifndef WNDFILTERS_H
#define WNDFILTERS_H

#include <QtGui/QDialog>

namespace Ui
{
  class wndFilters;
}

class wndFilters: public QDialog
{
  Q_OBJECT

  public:
    explicit wndFilters(QWidget *parent = 0);
    ~wndFilters();

    void setFileName(const QString &fileName) { m_fileName = fileName; }

  protected:
    void showEvent(QShowEvent *event);

  private slots:
    void on_btnOK_clicked();

  private:
    Ui::wndFilters *m_ui;
    QString m_fileName;
};

#endif // WNDFILTERS_H
