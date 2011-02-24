#include "wndMain.h"
#include "ui_wndMain.h"

#include "LCFilter.h"
#include "LCExport.h"
#include "LCTreeItem.h"
#include "wndFilters.h"

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtGui/QFileDialog>
#include <QtGui/QFileIconProvider>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QScrollBar>

wndMain::wndMain(QWidget *parent): QMainWindow(parent), m_ui(new Ui::wndMain)
{
  QSettings set;

  m_ui->setupUi(this);
  m_ui->pbProgress->hide();
  m_ui->gbExport->hide();

  loadFilters();
  loadExports();

  show();

  restoreGeometry(set.value("geometry", saveGeometry()).toByteArray());
  restoreState(set.value("state", saveState()).toByteArray());
  m_ui->splitter->restoreState(set.value("splitter", m_ui->splitter->saveState()).toByteArray());
  m_ui->twStats->header()->restoreState(set.value("columns", m_ui->twStats->header()->saveState()).toByteArray());
  m_ui->tblStats->horizontalHeader()->restoreState(set.value("columns", m_ui->twStats->header()->saveState()).toByteArray());

  connect(m_ui->twStats->header(), SIGNAL(sectionResized(int,int,int)), this, SLOT(slotResizeStats(int,int,int)));
  connect(m_ui->twStats->horizontalScrollBar(), SIGNAL(sliderMoved(int)), m_ui->tblStats->horizontalScrollBar(), SLOT(setValue(int)));

  m_wndFilters = new wndFilters(this);
}

wndMain::~wndMain()
{
  QSettings set;

  set.setValue("geometry", saveGeometry());
  set.setValue("state", saveState());
  set.setValue("splitter", m_ui->splitter->saveState());
  set.setValue("columns", m_ui->twStats->header()->saveState());

  delete m_wndFilters;
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
  m_ui->cbExportFormats->clear();

  QDir exports(qApp->applicationDirPath() + "/data/exports");
  foreach(QFileInfo info, exports.entryInfoList(QStringList() << "*xml", QDir::Files, QDir::Name)) {
    LCExport expo(info.fileName());

    m_ui->cbExportFormats->addItem(expo.name(), expo.fileName());
  }
}

QStringList wndMain::fileList(const QString &fileDir, const QStringList &filters, const QStringList &excludes)
{
  QStringList files;

  QDir dir(fileDir);
  foreach(QString file, dir.entryList(filters, QDir::Files)) {
    bool canAdd = true;

    file.prepend(dir.absolutePath() + "/");

    foreach(QString exclude, excludes) {
      if(file.contains(QRegExp(exclude, Qt::CaseInsensitive, QRegExp::Wildcard))) {
        canAdd = false;
        break;
      }
    }

    if(canAdd) {
      files.append(file);
    }
  }

  if(m_ui->cbDirectoriesSubDirs->isChecked()) {
    QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QFileInfo subdir, subdirs) {
      files.append(fileList(subdir.absoluteFilePath(), filters, excludes));
    }
  }

  return files;
}

QString wndMain::calcFileSize(qint64 bytes)
{
  int KB = 1024;
  int MB = KB * 1024;
  int GB = MB * 1024;

  if(bytes >= GB) {
    if(bytes % GB == 0) {
      return QString::number(bytes / GB) + " GB";
    } else {
      return QString::number(bytes / (float)GB, 'f', 2) + " GB";
    }
  } else if(bytes >= MB) {
    if(bytes % MB == 0) {
      return QString::number(bytes / MB) + " MB";
    } else {
      return QString::number(bytes / (float)MB, 'f', 2) + " MB";
    }
  } else if(bytes >= KB) {
    if(bytes % KB == 0) {
      return QString::number(bytes / KB) + " KB";
    } else {
      return QString::number(bytes / (float)KB, 'f', 2) + " KB";
    }
  } else {
    return QString::number(bytes) + " B";
  }
}

void wndMain::slotResizeStats(int index, int oldSize, int newSize)
{
  m_ui->tblStats->horizontalHeader()->resizeSection(index, newSize);
}

void wndMain::on_btnDirectoriesAdd_clicked()
{
  QSettings set;

  QString dir = QFileDialog::getExistingDirectory(this, tr("Add directory"), set.value("lastdir").toString());
  if(!dir.isEmpty()) {
    if(m_ui->twDirectories->findItems(dir, Qt::MatchExactly).count() == 0) {
      m_ui->twDirectories->addTopLevelItem(new QTreeWidgetItem(QStringList() << dir));
    }

    set.setValue("lastdir", dir);
  }

  m_ui->btnStart->setEnabled(m_ui->twDirectories->topLevelItemCount() > 0);
}

void wndMain::on_btnStart_clicked()
{
  QStringList includes;
  QStringList excludes;
  QList<QRegExp> comments;

  m_ui->twStats->clear();
  m_ui->pbProgress->show();
  m_ui->pbProgress->setMinimum(0);
  m_ui->pbProgress->setMaximum(0);

  for(int i = 0; i < m_ui->twFilters->topLevelItemCount(); i++) {
    QTreeWidgetItem *item = m_ui->twFilters->topLevelItem(i);
    if(item->checkState(0) == Qt::Checked) {
      LCFilter filter(item->text(1));

      includes.append(filter.includes());
      excludes.append(filter.excludes());
      comments.append(filter.comments());
    }
  }

  includes.removeDuplicates();
  excludes.removeDuplicates();

  QStringList files;
  for(int i = 0; i < m_ui->twDirectories->topLevelItemCount(); i++) {
    QTreeWidgetItem *item = m_ui->twDirectories->topLevelItem(i);

    files.append(fileList(item->text(0), includes, excludes));
  }

  files.removeDuplicates();

  QFileIconProvider prov;
  int totalFileSize = 0;
  int totalLineCount = 0;
  int totalEmptyLineCount = 0;
  int totalCommentLineCount = 0;

  m_ui->pbProgress->setMaximum(m_ui->pbProgress->maximum() + files.count() - 1);
  foreach(QString file, files) {
    QFile text(file);
    if(text.open(QIODevice::ReadOnly)) {
      int lineCount = 0;
      int emptyLineCount = 0;
      int commentLineCount = 0;

      //QString content = text.readAll();
      //text.reset();
      QTextStream stream(&text);
      QString line;
      while(!stream.atEnd()) {
        line = stream.readLine();
        lineCount++;

        if(line.trimmed().isEmpty()) {
          emptyLineCount++;
        }

        foreach(QRegExp regexp, comments) {
          if(regexp.exactMatch(line)) {
            commentLineCount++;
            break;
          }
        }
      }

      int fileSize = text.size();
      totalFileSize += fileSize;
      totalLineCount += lineCount;
      totalEmptyLineCount += emptyLineCount;
      totalCommentLineCount += commentLineCount;

      LCTreeItem *item = new LCTreeItem();

      item->setIcon(0, prov.icon(file));
      item->setText(0, file);

      item->setText(1, calcFileSize(fileSize));
      item->setData(1, Qt::UserRole, text.size());

      item->setText(2, QString::number(lineCount - (commentLineCount + emptyLineCount)));
      item->setText(3, QString::number(commentLineCount));
      item->setText(4, QString::number(emptyLineCount));
      item->setText(5, QString::number(lineCount));

      m_ui->twStats->addTopLevelItem(item);
    }
    text.close();

    m_ui->pbProgress->setValue(m_ui->pbProgress->value() + 1);
  }

  int totalSourceLineCount = totalLineCount - (totalCommentLineCount + totalEmptyLineCount);

  m_ui->tblStats->item(0, 0)->setText(QString("%1 %2").arg(files.count()).arg((files.count() == 1)?tr("file"):tr("files")));
  m_ui->tblStats->item(0, 0)->setData(Qt::UserRole, files.count());

  m_ui->tblStats->item(0, 1)->setText(calcFileSize(totalFileSize));
  m_ui->tblStats->item(0, 1)->setData(Qt::UserRole, totalFileSize);

  m_ui->tblStats->item(0, 2)->setText(QString("%1 (%2%)").arg(totalSourceLineCount).arg(totalSourceLineCount * 100.0 / totalLineCount, 0, 'g', 2));
  m_ui->tblStats->item(0, 2)->setData(Qt::UserRole, totalSourceLineCount);

  m_ui->tblStats->item(0, 3)->setText(QString("%1 (%2%)").arg(totalCommentLineCount).arg(totalCommentLineCount * 100.0 / totalLineCount, 0, 'g', 2));
  m_ui->tblStats->item(0, 3)->setData(Qt::UserRole, totalCommentLineCount);

  m_ui->tblStats->item(0, 4)->setText(QString("%1 (%2%)").arg(totalEmptyLineCount).arg(totalEmptyLineCount * 100.0 / totalLineCount, 0, 'g', 2));
  m_ui->tblStats->item(0, 4)->setData(Qt::UserRole, totalEmptyLineCount);

  m_ui->tblStats->item(0, 5)->setText(QString::number(totalLineCount));

  m_ui->gbExport->show();
}

void wndMain::on_twDirectories_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
  m_ui->btnDirectoriesRemove->setEnabled(current != 0);
}

void wndMain::on_btnDirectoriesRemove_clicked()
{
  QTreeWidgetItem *item = m_ui->twDirectories->currentItem();
  if(item) {
    int index = m_ui->twDirectories->indexOfTopLevelItem(item);
    m_ui->twDirectories->takeTopLevelItem(index);
    delete item;
  }
}

void wndMain::on_twStats_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
  if(item) {
    QDesktopServices::openUrl(QUrl("file://" + item->text(0)));
  }
}

void wndMain::on_btnExportSave_clicked()
{
  if(m_ui->cbExportFormats->currentIndex() > -1) {
    LCExport expo(m_ui->cbExportFormats->itemData(m_ui->cbExportFormats->currentIndex()).toString());

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export report"), QString(), expo.ext());
    if(!fileName.isEmpty()) {
      QStringList totalStats;
      totalStats.append(m_ui->tblStats->item(0, 0)->data(Qt::UserRole).toString());
      totalStats.append(m_ui->tblStats->item(0, 1)->data(Qt::UserRole).toString());
      totalStats.append(m_ui->tblStats->item(0, 1)->text());
      totalStats.append(m_ui->tblStats->item(0, 2)->data(Qt::UserRole).toString());
      totalStats.append(m_ui->tblStats->item(0, 3)->data(Qt::UserRole).toString());
      totalStats.append(m_ui->tblStats->item(0, 4)->data(Qt::UserRole).toString());
      totalStats.append(m_ui->tblStats->item(0, 5)->text());


      QFile file(fileName);
      if(file.open(QIODevice::WriteOnly)) {
        QTextStream str(&file);
        str << expo.prepend(totalStats);

        for(int i = 0; i < m_ui->twStats->topLevelItemCount(); i++) {
          QTreeWidgetItem *item = m_ui->twStats->topLevelItem(i);
          str << expo.createItem(item);
        }

        str << expo.append(totalStats);
        file.close();

        QMessageBox::information(this, tr("Export completed"), tr("The report has been saved."));
      }
    }
  }
}

void wndMain::on_btnFiltersNew_clicked()
{
  QString fileName = QInputDialog::getText(this, tr("Filter's file name"), tr("Please input file name for the new filter:"));
  if(!fileName.isEmpty()) {
    m_wndFilters->setFileName(fileName);
    m_wndFilters->show();
  }
}

void wndMain::on_twFilters_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
  m_ui->btnFiltersEdit->setEnabled(current != 0);
  m_ui->btnFiltersDelete->setEnabled(current != 0);
}

void wndMain::on_btnFiltersEdit_clicked()
{
  if(m_ui->twFilters->currentItem()) {
    m_wndFilters->setFileName(m_ui->twFilters->currentItem()->text(1));
    m_wndFilters->show();
  }
}

void wndMain::on_twFilters_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
  if(item) {
    on_btnFiltersEdit_clicked();
  }
}
