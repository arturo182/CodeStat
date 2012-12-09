#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>

class QTreeWidgetItem;

namespace Ui
{
	class FilterDialog;
}

class FilterDialog: public QDialog
{
	Q_OBJECT

	public:
		FilterDialog(QWidget *parent = 0, const QString &fileName = QString());
		~FilterDialog();

	protected:
		void done(int result);

	private slots:
		void on_includeTree_currentItemChanged(QTreeWidgetItem *current);
		void on_excludeTree_currentItemChanged(QTreeWidgetItem *current);
		void on_removeIncludeButton_clicked();
		void on_removeExcludeButton_clicked();
		void on_addIncludeButton_clicked();

		void on_addExcludeButton_clicked();

	private:
		Ui::FilterDialog *m_ui;
		QString m_fileName;
};

#endif // FILTERDIALOG_H
