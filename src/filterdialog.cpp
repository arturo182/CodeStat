#include "filterdialog.h"
#include "ui_filterdialog.h"

#include "filter.h"

#include <QInputDialog>
#include <QSettings>

FilterDialog::FilterDialog(QWidget *parent):
	QDialog(parent),
	m_ui(new Ui::FilterDialog)
{
	QSettings set;

	m_ui->setupUi(this);
	setModal(true);

	restoreGeometry(set.value("filters/geometry", saveGeometry()).toByteArray());
}

FilterDialog::~FilterDialog()
{
	QSettings set;

	set.setValue("filters/geometry", saveGeometry());

	delete m_ui;
}

void FilterDialog::showEvent(QShowEvent *event)
{
	move(parentWidget()->geometry().center() - QPoint((width() / 2), (height() / 2)));

	if(m_fileName.isEmpty()) {
		setWindowTitle(tr("Add filter"));

		m_ui->nameEdit->clear();
		m_ui->commentsEdit->clear();
		m_ui->includeTree->clear();
		m_ui->excludeTree->clear();
	} else {
		setWindowTitle(tr("Edit filter"));

		Filter filter(m_fileName);

		m_ui->nameEdit->setText(filter.name());
		m_ui->commentsEdit->setText(filter.comments().pattern());

		m_ui->includeTree->clear();
		foreach(const QString &include, filter.includes()) {
			m_ui->includeTree->addTopLevelItem(new QTreeWidgetItem(QStringList() << include));
		}

		m_ui->excludeTree->clear();
		foreach(const QString &exclude, filter.excludes()) {
			m_ui->excludeTree->addTopLevelItem(new QTreeWidgetItem(QStringList() << exclude));
		}
	}
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
	if(item) {
		const int idx = m_ui->includeTree->indexOfTopLevelItem(item);
		m_ui->includeTree->takeTopLevelItem(idx);
	}
}

void FilterDialog::on_removeExcludeButton_clicked()
{
	QTreeWidgetItem *item = m_ui->excludeTree->currentItem();
	if(item) {
		const int idx = m_ui->excludeTree->indexOfTopLevelItem(item);
		m_ui->excludeTree->takeTopLevelItem(idx);
	}
}

void FilterDialog::on_addIncludeButton_clicked()
{
	QString include = QInputDialog::getText(this, tr("Add include"), tr("You can use * and ? wildcards."));
	if(!include.isEmpty()) {
		if(!m_ui->includeTree->findItems(include, Qt::MatchExactly).count()) {
			QTreeWidgetItem *item = new QTreeWidgetItem(m_ui->includeTree);
			item->setText(0, include);
		}
	}
}
