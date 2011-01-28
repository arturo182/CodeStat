#include "wndMain.h"
#include "ui_wndMain.h"

#include "LCFilter.h"
#include "LCTreeItem.h"

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtGui/QFileDialog>
#include <QtGui/QFileIconProvider>

wndMain::wndMain(QWidget *parent): QMainWindow(parent), m_ui(new Ui::wndMain)
{
  QSettings set;

  m_ui->setupUi(this);
  m_ui->pbProgress->hide();
  m_ui->gbExport->hide();

  loadFilters();
  loadExports();

  show();

  restoreGeometry(QByteArray::fromBase64(set.value("geometry", saveGeometry().toBase64()).toByteArray()));
  restoreState(QByteArray::fromBase64(set.value("state", saveState().toBase64()).toByteArray()));
  m_ui->splitter->restoreState(QByteArray::fromBase64(set.value("splitter", m_ui->splitter->saveState().toBase64()).toByteArray()));
  m_ui->twStats->header()->restoreState(QByteArray::fromBase64(set.value("columns", m_ui->twStats->header()->saveState().toBase64()).toByteArray()));
}

wndMain::~wndMain()
{
  QSettings set;

  set.setValue("geometry", saveGeometry().toBase64());
  set.setValue("state", saveState().toBase64());
  set.setValue("splitter", m_ui->splitter->saveState().toBase64());
  set.setValue("columns", m_ui->twStats->header()->saveState().toBase64());

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

void wndMain::on_btnDirectoriesAdd_clicked()
{
  QString dir = QFileDialog::getExistingDirectory(this, tr("Add directory"));
  if(!dir.isEmpty()) {
    if(m_ui->twDirectories->findItems(dir, Qt::MatchExactly).count() == 0) {
      m_ui->twDirectories->addTopLevelItem(new QTreeWidgetItem(QStringList() << dir));
    }
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

      LCTreeItem *item = new LCTreeItem();

      item->setIcon(0, prov.icon(file));
      item->setText(0, file);
      item->setText(1, calcFileSize(text.size()));
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
