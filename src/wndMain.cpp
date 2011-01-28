#include "wndMain.h"
#include "ui_wndMain.h"

#include "LCFilter.h"
#include "LCTreeItem.h"

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtGui/QFileDialog>
#include <QtGui/QFileIconProvider>

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
    LCFilter filter(info.fileName());

    QTreeWidgetItem *item = new QTreeWidgetItem();

    item->setCheckState(0, Qt::Unchecked);
    item->setText(0, filter.name());
    item->setText(1, info.fileName());

    m_ui->twFilters->addTopLevelItem(item);
  }
}

void wndMain::loadExports()
{
  QDir exports(qApp->applicationDirPath() + "/data/exports");
}

QStringList wndMain::getFileList(const QString &fileDir, const QStringList &filters)
{
  QStringList files;

  QDir dir(fileDir);
  foreach(QString file, dir.entryList(filters, QDir::Files)) {
    files.append(dir.absolutePath() + "/" + file);
  }

  if(m_ui->cbDirectoriesSubDirs->isChecked()) {
    QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QFileInfo subdir, subdirs) {
      files.append(getFileList(subdir.absoluteFilePath(), filters));
    }
  }

  return files;
}

void wndMain::on_btnDirectoriesAdd_clicked()
{
  QString dir = QFileDialog::getExistingDirectory(this, tr("Add directory"));
  if(!dir.isEmpty()) {
    m_ui->twDirectories->addTopLevelItem(new QTreeWidgetItem(QStringList() << dir));
  }

  m_ui->btnStart->setEnabled(m_ui->twDirectories->topLevelItemCount() > 0);
}

void wndMain::on_btnStart_clicked()
{
  QStringList includes;
  QStringList excludes;

  m_ui->twStats->clear();

  for(int i = 0; i < m_ui->twFilters->topLevelItemCount(); i++) {
    QTreeWidgetItem *item = m_ui->twFilters->topLevelItem(i);
    if(item->checkState(0) == Qt::Checked) {
      LCFilter filter(item->text(1));

      includes.append(filter.includes());
      excludes.append(filter.excludes());
    }
  }

  includes.removeDuplicates();
  excludes.removeDuplicates();

  QStringList files;
  for(int i = 0; i < m_ui->twDirectories->topLevelItemCount(); i++) {
    QTreeWidgetItem *item = m_ui->twDirectories->topLevelItem(i);

    files.append(getFileList(item->text(0), includes));
  }

  QFileIconProvider prov;

  foreach(QString file, files) {
    QFile text(file);
    if(text.open(QIODevice::ReadOnly)) {
      int lineCount = 0;
      int emptyLineCount = 0;
      //int comment

      //QString content = text.readAll();
      QTextStream stream(&text);
      QString line;
      while(!stream.atEnd()) {
        line = stream.readLine();
        lineCount++;

        if(line.trimmed().isEmpty()) {
          emptyLineCount++;
        }
      }

      LCTreeItem *item = new LCTreeItem();

      item->setIcon(0, prov.icon(file));
      item->setText(0, file);
      item->setText(1, QString::number(text.size() / 1024.0));
      //item->setText(2, );
      //item->setText(3, );
      item->setText(4, QString::number(emptyLineCount));
      item->setText(5, QString::number(lineCount));

      m_ui->twStats->addTopLevelItem(item);
    }
    text.close();
  }
}
