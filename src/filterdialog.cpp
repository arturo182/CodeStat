#include "filterdialog.h"
#include "ui_filterdialog.h"

#include "filter.h"

#include <QInputDialog>
#include <QSettings>
#include <QFileDialog>

FilterDialog::FilterDialog(QWidget *parent, const QString &fileName):
	QDialog(parent),
	m_ui(new Ui::FilterDialog),
	m_fileName(fileName)
{
	m_ui->setupUi(this);

	if(m_fileName.isEmpty()) {
		setWindowTitle(tr("Add filter"));
	} else {
		setWindowTitle(tr("Edit filter"));

		Filter filter(fileName);

		m_ui->nameEdit->setText(filter.name());
		m_ui->commentsEdit->setText(filter.comments().pattern());

		m_ui->includeTree->clear();
		foreach(const QString &include, filter.includes()) {
			QTreeWidgetItem *treeItem = new QTreeWidgetItem(m_ui->includeTree);
			treeItem->setText(0, include);
		}

		foreach(const QString &exclude, filter.excludes()) {
			QTreeWidgetItem *treeItem = new QTreeWidgetItem(m_ui->excludeTree);
			treeItem->setText(0, exclude);
		}
	}
}

FilterDialog::~FilterDialog()
{
	delete m_ui;
}

void FilterDialog::done(int result)
{
	if(result == QDialog::Rejected) {
		QDialog::done(result);
		return;
	}

	if(m_fileName.isEmpty()) {
		m_fileName = QFileDialog::getSaveFileName(this, tr("Select file name"), "", "Json (*.json)");
	}

	Filter filter(m_fileName);
	filter.setName(m_ui->nameEdit->text());
	filter.setComments(QRegExp(m_ui->commentsEdit->text()));

	QStringList includes;
	for(int i = 0; i< m_ui->includeTree->topLevelItemCount(); ++i)
		includes << m_ui->includeTree->topLevelItem(i)->text(0);
	filter.setIncludes(includes);

	QStringList excludes;
	for(int i = 0; i< m_ui->excludeTree->topLevelItemCount(); ++i)
		excludes << m_ui->excludeTree->topLevelItem(i)->text(0);
	filter.setExcludes(excludes);
	filter.save();

	QDialog::done(result);
}

void FilterDialog::on_includeTree_currentItemChanged(QTreeWidgetItem *current)
{
	m_ui->removeIncludeButton->setEnabled(current != 0);
}

void FilterDialog::on_excludeTree_currentItemChanged(QTreeWidgetItem *current)
{
	m_ui->removeExcludeButton->setEnabled(current != 0);
}

void FilterDialog::on_removeIncludeButton_clicked()
{
	QTreeWidgetItem *item = m_ui->includeTree->currentItem();
	if(!item)
		return;

	const int idx = m_ui->includeTree->indexOfTopLevelItem(item);
	m_ui->includeTree->takeTopLevelItem(idx);
}

void FilterDialog::on_removeExcludeButton_clicked()
{
	QTreeWidgetItem *item = m_ui->excludeTree->currentItem();
	if(!item)
		return;

	const int idx = m_ui->excludeTree->indexOfTopLevelItem(item);
	m_ui->excludeTree->takeTopLevelItem(idx);
}

void FilterDialog::on_addIncludeButton_clicked()
{
	QString include = QInputDialog::getText(this, tr("Add include"), tr("You can use * and ? wildcards."));
	if(include.isEmpty())
		return;

	if(!m_ui->includeTree->findItems(include, Qt::MatchExactly).count()) {
		QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->includeTree);
		item->setText(0, include);
	}
}

void FilterDialog::on_addExcludeButton_clicked()
{
	QString exclude = QInputDialog::getText(this, tr("Add exclude"), tr("You can use * and ? wildcards."));
	if(exclude.isEmpty())
		return;

	if(!m_ui->excludeTree->findItems(exclude, Qt::MatchExactly).count()) {
		QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->excludeTree);
		item->setText(0, exclude);
	}
}
