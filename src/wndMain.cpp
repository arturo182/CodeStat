#include "wndMain.h"
#include "ui_wndMain.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QFileDialog>

wndMain::wndMain(QWidget *parent): QMainWindow(parent), m_ui(new Ui::wndMain)
{
  m_ui->setupUi(this);
  m_ui->pbProgress->hide();
  m_ui->gbExport->hide();

  loadFilters();
  loadExports();
}

wndMain::~wndMain()
{
  delete m_ui;
}

void wndMain::loadFilters()
{
  m_ui->twFilters->clear();

  QDir filters(qApp->applicationDirPath() + "/data/filters");
  foreach(QFileInfo info, filters.entryInfoList(QStringList() << "*xml", QDir::Files, QDir::Name)) {
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setCheckState(0, Qt::Unchecked);
    item->setText(0, info.fileName());
    m_ui->twFilters->addTopLevelItem(item);
  }
}

void wndMain::loadExports()
{
  QDir exports(qApp->applicationDirPath() + "/data/exports");
}

void wndMain::on_btnDirectoriesAdd_clicked()
{
  QString dir = QFileDialog::getExistingDirectory(this, tr("Add directory"));
  if(!dir.isEmpty()) {
    m_ui->twDirectories->addTopLevelItem(new QTreeWidgetItem(QStringList() << dir));
  }
}
