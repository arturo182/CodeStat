#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "filterdialog.h"
#include "treeitem.h"
#include "filter.h"
#include "export.h"

#include <QFileIconProvider>
#include <QDesktopServices>
#include <QProgressDialog>
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
	m_ui->setupUi(this);
	m_ui->progressBar->hide();
	m_ui->exportGroupBox->hide();

	readSettings();

	reloadFilters();
	loadExports();

	connect(m_ui->statsTree->header(), &QHeaderView::sectionResized, this, &MainWindow::resizeStats);
	connect(m_ui->statsTree->horizontalScrollBar(), &QScrollBar::sliderMoved, m_ui->statsTable->horizontalScrollBar(), &QScrollBar::setValue);
}

MainWindow::~MainWindow()
{
	delete m_ui;
}

void MainWindow::reloadFilters()
{
	QStringList selectedFilters;
	for(int i = 0; i < m_ui->filtersTree->topLevelItemCount(); ++i) {
		QTreeWidgetItem *item = m_ui->filtersTree->topLevelItem(i);
		if(item->checkState(0) == Qt::Checked)
			selectedFilters << item->text(1);
	}


	m_ui->filtersTree->clear();

	QDir filters(qApp->applicationDirPath() + "/filters");
	foreach(const QFileInfo &info, filters.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Name)) {
		const Filter filter(info.absoluteFilePath());

		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setCheckState(0, selectedFilters.contains(info.absoluteFilePath()) ? Qt::Checked : Qt::Unchecked);
		item->setText(0, filter.name());
		item->setText(1, info.absoluteFilePath());

		m_ui->filtersTree->addTopLevelItem(item);
	}
}

void MainWindow::loadExports()
{
	m_ui->exportFormatCombo->clear();

	QDir exports(qApp->applicationDirPath() + "/exports");
	foreach(const QFileInfo &info, exports.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Name)) {
		Export expo(info.absoluteFilePath());

		m_ui->exportFormatCombo->addItem(expo.name(), expo.fileName());
	}
}

QStringList MainWindow::fileList(const QString &fileDir, const QStringList &filters, const QStringList &excludes)
{
	QStringList files;

	const QDir dir(fileDir);
	foreach(const QFileInfo &fileInfo, dir.entryInfoList(filters, QDir::Files)) {
		bool isExcluded = false;

		foreach(const QString &exclude, excludes) {
			if(fileInfo.absoluteFilePath().contains(QRegExp(exclude, Qt::CaseInsensitive, QRegExp::Wildcard))) {
				isExcluded = true;
				break;
			}
		}

		if(isExcluded)
			continue;

		files << fileInfo.absoluteFilePath();
	}

	if(m_ui->subdirsCheckBox->isChecked()) {
		const QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
		foreach(const QFileInfo &subdir, subdirs) {
			files << fileList(subdir.absoluteFilePath(), filters, excludes);
			qApp->processEvents();
		}
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

void MainWindow::closeEvent(QCloseEvent *event)
{
	writeSettings();
}

void MainWindow::resizeStats(const int &index, const int &oldSize, const int &newSize)
{
	m_ui->statsTable->horizontalHeader()->resizeSection(index, newSize);
}

void MainWindow::addDir()
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

	updateStartButton();
}

void MainWindow::start()
{
	QStringList includes;
	QStringList excludes;
	QList<QRegularExpression> comments;

	m_ui->statsTree->clear();

	for(int i = 0; i < m_ui->filtersTree->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = m_ui->filtersTree->topLevelItem(i);
		if(item->checkState(0) != Qt::Checked)
			continue;

		const Filter filter(item->text(1));

		includes << filter.includes();
		excludes << filter.excludes();
		comments << filter.comments();
	}

	includes.removeDuplicates();
	excludes.removeDuplicates();

	QProgressDialog progressDlg(this);
	progressDlg.setWindowTitle(tr("Building file list"));
	progressDlg.setMaximum(0);
	progressDlg.show();

	QStringList files;
	for(int i = 0; i < m_ui->dirsTree->topLevelItemCount(); i++) {
		const QTreeWidgetItem *item = m_ui->dirsTree->topLevelItem(i);

		progressDlg.setLabelText(tr("Listing files in \"%1\".").arg(item->text(0)));
		files << fileList(item->text(0), includes, excludes);

		if(progressDlg.wasCanceled())
			return;
	}
	files.removeDuplicates();

	progressDlg.close();

	if(!files.size())
		return;

	m_ui->progressBar->show();
	m_ui->progressBar->setMinimum(0);
	m_ui->progressBar->setMaximum(files.count() - 1);

	const QFileIconProvider iconProvider;
	qint64 totalFileSize = 0;
	qint64 totalLineCount = 0;
	qint64 totalEmptyLineCount = 0;
	qint64 totalCommentLineCount = 0;

	foreach(const QString &fileName, files) {
		QFile file(fileName);
		if(file.open(QIODevice::ReadOnly)) {
			qint64 lineCount = 0;
			qint64 emptyLineCount = 0;
			qint64 commentLineCount = 0;

			QTextStream stream(&file);
			QString line;
			while(!stream.atEnd()) {
				line = stream.readLine();
				++lineCount;

				if(line.trimmed().isEmpty())
					++emptyLineCount;
			}
			file.seek(0);

			const QString content = file.readAll();
			foreach(const QRegularExpression &regexp, comments) {
				QRegularExpressionMatchIterator matchIt = regexp.globalMatch(content);
				while(matchIt.hasNext()) {
					const QRegularExpressionMatch match = matchIt.next();
					if(!match.hasMatch())
						continue;

					commentLineCount += match.captured(0).trimmed().toLatin1().count('\n') + 1;
				}
			}

			const qint64 fileSize = file.size();
			totalFileSize += fileSize;
			totalLineCount += lineCount;
			totalEmptyLineCount += emptyLineCount;
			totalCommentLineCount += commentLineCount;

			TreeItem *item = new TreeItem(m_ui->statsTree);
			item->setIcon(0, iconProvider.icon(fileName));
			item->setText(0, fileName);

			item->setText(1, calcFileSize(fileSize));
			item->setData(1, Qt::UserRole, file.size());

			item->setText(2, QString::number(lineCount - (commentLineCount + emptyLineCount)));
			item->setText(3, QString::number(commentLineCount));
			item->setText(4, QString::number(emptyLineCount));
			item->setText(5, QString::number(lineCount));
		}
		file.close();

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

void MainWindow::updateDirButtons(QTreeWidgetItem *current)
{
	m_ui->removeDirButton->setEnabled(current != 0);
}

void MainWindow::removeDir()
{
	delete m_ui->dirsTree->currentItem();

	updateStartButton();
}

void MainWindow::openFile(QTreeWidgetItem *item)
{
	if(!item)
		return;

	QDesktopServices::openUrl(QUrl("file:///" + item->text(0)));
}

void MainWindow::saveExport()
{
	if(m_ui->exportFormatCombo->currentIndex() > -1) {
		const int currentIndex = m_ui->exportFormatCombo->currentIndex();
		Export expo(m_ui->exportFormatCombo->itemData(currentIndex).toString());

		QString fileName = QFileDialog::getSaveFileName(this, tr("Export report"), QString(), expo.extension());
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

void MainWindow::newFilter()
{
	FilterDialog dlg(this);
	if(dlg.exec())
		reloadFilters();
}

void MainWindow::updateFiltersButtons(QTreeWidgetItem *item)
{
	m_ui->editFilterButton->setEnabled(item != 0);
	m_ui->deleteFilterButton->setEnabled(item != 0);
}

void MainWindow::editFilter()
{
	if(!m_ui->filtersTree->currentItem())
		return;

	FilterDialog dlg(this, m_ui->filtersTree->currentItem()->text(1));
	if(dlg.exec())
		reloadFilters();
}

void MainWindow::on_filtersTree_itemDoubleClicked(QTreeWidgetItem *item)
{
	if(!item)
		return;

	editFilter();
}

void MainWindow::updateStartButton()
{
	m_ui->startButton->setEnabled(m_ui->dirsTree->topLevelItemCount() > 0);
}

void MainWindow::readSettings()
{
	QSettings set;
	restoreGeometry(set.value("geometry").toByteArray());
	restoreState(set.value("state").toByteArray());
	m_ui->statsTree->header()->restoreState(set.value("columns").toByteArray());
	m_ui->statsTable->horizontalHeader()->restoreState(set.value("columns").toByteArray());

}

void MainWindow::writeSettings()
{
	QSettings set;
	set.setValue("geometry", saveGeometry());
	set.setValue("state", saveState());
	set.setValue("columns", m_ui->statsTree->header()->saveState());
}

void MainWindow::deleteFilter()
{
	if(!m_ui->filtersTree->currentItem())
		return;

	const QFileInfo fileInfo(m_ui->filtersTree->currentItem()->text(1));
	const Filter filter(fileInfo.absoluteFilePath());

	if(QMessageBox::question(this, tr("Are you sure?"), tr("Do you want to delete filter \"%1\"?").arg(filter.name()), QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
		return;

	QFile::remove(fileInfo.absoluteFilePath());
	reloadFilters();
}
