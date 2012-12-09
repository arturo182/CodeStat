#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "filterdialog.h"
#include "treeitem.h"
#include "filter.h"
#include "export.h"

#include <QFileIconProvider>
#include <QDesktopServices>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QFileInfo>
#include <QSettings>
#include <qmath.h>
#include <QDebug>
#include <QDir>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent):
	QMainWindow(parent),
	m_ui(new Ui::MainWindow)
{
	QSettings set;

	m_ui->setupUi(this);
	m_ui->progressBar->hide();
	m_ui->exportGroupBox->hide();

	loadFilters();
	loadExports();

	restoreGeometry(set.value("geometry").toByteArray());
	restoreState(set.value("state").toByteArray());
	m_ui->statsTree->header()->restoreState(set.value("columns").toByteArray());
	m_ui->statsTable->horizontalHeader()->restoreState(set.value("columns").toByteArray());

	connect(m_ui->statsTree->header(), &QHeaderView::sectionResized, this, &MainWindow::slotResizeStats);
	connect(m_ui->statsTree->horizontalScrollBar(), &QScrollBar::sliderMoved, m_ui->statsTable->horizontalScrollBar(), &QScrollBar::setValue);
}

MainWindow::~MainWindow()
{
	QSettings set;
	set.setValue("geometry", saveGeometry());
	set.setValue("state", saveState());
	set.setValue("columns", m_ui->statsTree->header()->saveState());

	delete m_ui;
}

void MainWindow::loadFilters()
{
	m_ui->filtersTree->clear();

	QDir filters(qApp->applicationDirPath() + "/data/filters");
	foreach(const QFileInfo &info, filters.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Name)) {
		const Filter filter(info.absoluteFilePath());

		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setCheckState(0, Qt::Unchecked);
		item->setText(0, filter.name());
		item->setText(1, info.absoluteFilePath());

		m_ui->filtersTree->addTopLevelItem(item);
	}
}

void MainWindow::loadExports()
{
	m_ui->exportFormatCombo->clear();

	QDir exports(qApp->applicationDirPath() + "/data/exports");
	foreach(const QFileInfo &info, exports.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Name)) {
		Export expo(info.fileName());

		m_ui->exportFormatCombo->addItem(expo.name(), expo.fileName());
	}
}

QStringList MainWindow::fileList(const QString &fileDir, const QStringList &filters, const QStringList &excludes)
{
	QStringList files;

	QDir dir(fileDir);
	foreach(const QFileInfo &fileInfo, dir.entryInfoList(filters, QDir::Files)) {
		bool canAdd = true;

		foreach(const QString &exclude, excludes) {
			if(fileInfo.absoluteFilePath().contains(QRegExp(exclude, Qt::CaseInsensitive, QRegExp::Wildcard))) {
				canAdd = false;
				break;
			}
		}

		if(!canAdd)
			continue;

		files << fileInfo.absoluteFilePath();
	}

	if(m_ui->subdirsCheckBox->isChecked()) {
		const QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
		foreach(const QFileInfo &subdir, subdirs)
			files << fileList(subdir.absoluteFilePath(), filters, excludes);
	}

	return files;
}

QString MainWindow::calcFileSize(const qint64 &bytes)
{
	if(bytes == 0)
		return "0 B";

	const QStringList units = QStringList() << "B" << "KB" << "MB" << "GB" << "TB";
	const int digitGroups = log10(bytes) / log10(1024);

	return QString::number(bytes / qPow(1024.0, digitGroups), 'f', 2) + " " + units[digitGroups];
}

void MainWindow::slotResizeStats(const int &index, const int &oldSize, const int &newSize)
{
	m_ui->statsTable->horizontalHeader()->resizeSection(index, newSize);
}

void MainWindow::on_addDirButton_clicked()
{
	QSettings set;

	const QString dir = QFileDialog::getExistingDirectory(this, tr("Add directory"), set.value("lastdir").toString());
	if(!dir.isEmpty()) {
		if(m_ui->dirsTree->findItems(dir, Qt::MatchExactly).count() == 0) {
			QTreeWidgetItem *treeItem = new QTreeWidgetItem(m_ui->dirsTree);
			treeItem->setText(0, dir);
		}

		set.setValue("lastdir", dir);
	}

	m_ui->startButton->setEnabled(m_ui->dirsTree->topLevelItemCount() > 0);
}

void MainWindow::on_startButton_clicked()
{
	QStringList includes;
	QStringList excludes;
	QList<QRegExp> comments;

	m_ui->statsTree->clear();
	m_ui->progressBar->show();
	m_ui->progressBar->setMinimum(0);
	m_ui->progressBar->setMaximum(0);

	for(int i = 0; i < m_ui->filtersTree->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = m_ui->filtersTree->topLevelItem(i);
		if(item->checkState(0) == Qt::Checked) {
			const Filter filter(item->text(1));

			includes.append(filter.includes());
			excludes.append(filter.excludes());
			comments.append(filter.comments());
		}
	}

	includes.removeDuplicates();
	excludes.removeDuplicates();

	QStringList files;
	for(int i = 0; i < m_ui->dirsTree->topLevelItemCount(); i++) {
		const QTreeWidgetItem *item = m_ui->dirsTree->topLevelItem(i);
		files << fileList(item->text(0), includes, excludes);
	}

	files.removeDuplicates();

	const QFileIconProvider iconProv;
	int totalFileSize = 0;
	int totalLineCount = 0;
	int totalEmptyLineCount = 0;
	int totalCommentLineCount = 0;

	m_ui->progressBar->setMaximum(m_ui->progressBar->maximum() + files.count() - 1);
	foreach(QString file, files) {
		QFile text(file);
		if(text.open(QIODevice::ReadOnly)) {
			int lineCount = 0;
			int emptyLineCount = 0;
			int commentLineCount = 0;

			QTextStream stream(&text);
			QString line;
			while(!stream.atEnd()) {
				line = stream.readLine();
				lineCount++;

				if(line.trimmed().isEmpty()) {
					emptyLineCount++;
				}

				foreach(const QRegExp &regexp, comments) {
					if(regexp.exactMatch(line)) {
						commentLineCount++;
						break;
					}
				}
			}

			const int fileSize = text.size();
			totalFileSize += fileSize;
			totalLineCount += lineCount;
			totalEmptyLineCount += emptyLineCount;
			totalCommentLineCount += commentLineCount;

			TreeItem *item = new TreeItem();
			item->setIcon(0, iconProv.icon(file));
			item->setText(0, file);

			item->setText(1, calcFileSize(fileSize));
			item->setData(1, Qt::UserRole, text.size());

			item->setText(2, QString::number(lineCount - (commentLineCount + emptyLineCount)));
			item->setText(3, QString::number(commentLineCount));
			item->setText(4, QString::number(emptyLineCount));
			item->setText(5, QString::number(lineCount));

			m_ui->statsTree->addTopLevelItem(item);
		}
		text.close();

		m_ui->progressBar->setValue(m_ui->progressBar->value() + 1);
	}

	const int totalSourceLineCount = totalLineCount - (totalCommentLineCount + totalEmptyLineCount);

	m_ui->statsTable->item(0, 0)->setText(QString("%1 %2").arg(files.count()).arg((files.count() == 1) ? tr("file") : tr("files")));
	m_ui->statsTable->item(0, 0)->setData(Qt::UserRole, files.count());

	m_ui->statsTable->item(0, 1)->setText(calcFileSize(totalFileSize));
	m_ui->statsTable->item(0, 1)->setData(Qt::UserRole, totalFileSize);

	m_ui->statsTable->item(0, 2)->setText(QString("%1 (%2%)").arg(totalSourceLineCount).arg(totalSourceLineCount * 100.0 / totalLineCount, 0, 'g', 2));
	m_ui->statsTable->item(0, 2)->setData(Qt::UserRole, totalSourceLineCount);

	m_ui->statsTable->item(0, 3)->setText(QString("%1 (%2%)").arg(totalCommentLineCount).arg(totalCommentLineCount * 100.0 / totalLineCount, 0, 'g', 2));
	m_ui->statsTable->item(0, 3)->setData(Qt::UserRole, totalCommentLineCount);

	m_ui->statsTable->item(0, 4)->setText(QString("%1 (%2%)").arg(totalEmptyLineCount).arg(totalEmptyLineCount * 100.0 / totalLineCount, 0, 'g', 2));
	m_ui->statsTable->item(0, 4)->setData(Qt::UserRole, totalEmptyLineCount);

	m_ui->statsTable->item(0, 5)->setText(QString::number(totalLineCount));

	m_ui->exportGroupBox->show();
}

void MainWindow::on_dirsTree_currentItemChanged(QTreeWidgetItem *current)
{
	m_ui->removeDirButton->setEnabled(current != 0);
}

void MainWindow::on_removeDirButton_clicked()
{
	delete m_ui->dirsTree->currentItem();
}

void MainWindow::on_statsTree_itemDoubleClicked(QTreeWidgetItem *item)
{
	if(!item)
		return;

	QDesktopServices::openUrl(QUrl("file:///" + item->text(0)));
}

void MainWindow::on_exportSaveButton_clicked()
{
	if(m_ui->exportFormatCombo->currentIndex() > -1) {
		const int currentIndex = m_ui->exportFormatCombo->currentIndex();
		Export expo(m_ui->exportFormatCombo->itemData(currentIndex).toString());

		QString fileName = QFileDialog::getSaveFileName(this, tr("Export report"), QString(), expo.ext());
		if(!fileName.isEmpty()) {
			QStringList totalStats;
			totalStats.append(m_ui->statsTable->item(0, 0)->data(Qt::UserRole).toString());
			totalStats.append(m_ui->statsTable->item(0, 1)->data(Qt::UserRole).toString());
			totalStats.append(m_ui->statsTable->item(0, 1)->text());
			totalStats.append(m_ui->statsTable->item(0, 2)->data(Qt::UserRole).toString());
			totalStats.append(m_ui->statsTable->item(0, 3)->data(Qt::UserRole).toString());
			totalStats.append(m_ui->statsTable->item(0, 4)->data(Qt::UserRole).toString());
			totalStats.append(m_ui->statsTable->item(0, 5)->text());


			QFile file(fileName);
			if(file.open(QIODevice::WriteOnly)) {
				QTextStream str(&file);
				str << expo.prepend(totalStats);

				for(int i = 0; i < m_ui->statsTree->topLevelItemCount(); i++) {
					QTreeWidgetItem *item = m_ui->statsTree->topLevelItem(i);
					str << expo.createItem(item);
				}

				str << expo.append(totalStats);
				file.close();

				QMessageBox::information(this, tr("Export completed"), tr("The report has been saved."));
			}
		}
	}
}

void MainWindow::on_newFilterButton_clicked()
{
	FilterDialog dlg(this);
	dlg.exec();

	//TODO reload filters
}

void MainWindow::on_filtersTree_currentItemChanged(QTreeWidgetItem *item)
{
	m_ui->editFilterButton->setEnabled(item != 0);
	m_ui->deleteFilterButton->setEnabled(item != 0);
}

void MainWindow::on_editFilterButton_clicked()
{
	if(!m_ui->filtersTree->currentItem())
		return;

	FilterDialog dlg(this, m_ui->filtersTree->currentItem()->text(1));
	dlg.exec();

	//TODO reload filters
}

void MainWindow::on_filtersTree_itemDoubleClicked(QTreeWidgetItem *item)
{
	if(!item)
		return;

	on_editFilterButton_clicked();
}
