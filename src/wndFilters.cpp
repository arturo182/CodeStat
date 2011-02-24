#include "wndFilters.h"
#include "ui_wndFilters.h"

#include "LCFilter.h"

#include <QtCore/QSettings>

wndFilters::wndFilters(QWidget *parent): QDialog(parent), m_ui(new Ui::wndFilters)
{
  QSettings set;

  m_ui->setupUi(this);
  setModal(true);

  restoreGeometry(set.value("filters/geometry", saveGeometry()).toByteArray());
}

wndFilters::~wndFilters()
{
  QSettings set;

  set.setValue("filters/geometry", saveGeometry());

  delete m_ui;
}

void wndFilters::showEvent(QShowEvent *event)
{
  move(parentWidget()->geometry().center() - QPoint((width() / 2), (height() / 2)));

  if(m_fileName.isEmpty()) {
    setWindowTitle(tr("Add filter"));

    m_ui->edName->clear();
    m_ui->edComments->clear();
    m_ui->twFilesInclude->clear();
    m_ui->twFilesExclude->clear();
  } else {
    setWindowTitle(tr("Edit filter"));

    LCFilter filter(m_fileName);

    m_ui->edName->setText(filter.name());
    m_ui->edComments->setText(filter.comments().pattern());

    foreach(QString include, filter.includes()) {
      m_ui->twFilesInclude->addTopLevelItem(new QTreeWidgetItem(QStringList() << include));
    }

    foreach(QString exclude, filter.excludes()) {
      m_ui->twFilesExclude->addTopLevelItem(new QTreeWidgetItem(QStringList() << exclude));
    }
  }
}

void wndFilters::on_btnOK_clicked()
{
  accept();
}
