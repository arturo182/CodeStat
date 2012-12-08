#ifndef WNDMAIN_H
#define WNDMAIN_H

#include <QMainWindow>

class QLabel;
class QTreeWidgetItem;

class FilterDialog;

namespace Ui
{
	class MainWindow;
}

class MainWindow: public QMainWindow
{
		Q_OBJECT

	public:
		MainWindow(QWidget *parent = 0);
		~MainWindow();

		void loadFilters();
		void loadExports();

		QStringList fileList(const QString &fileDir, const QStringList &filters, const QStringList &excludes);
		QString calcFileSize(const qint64 &bytes);

	private slots:
		void slotResizeStats(const int &index, const int &oldSize, const int &newSize);
		void on_addDirButton_clicked();
		void on_startButton_clicked();
		void on_dirsTree_currentItemChanged(QTreeWidgetItem *current);
		void on_removeDirButton_clicked();
		void on_statsTree_itemDoubleClicked(QTreeWidgetItem *item);
		void on_exportSaveButton_clicked();
		void on_newFilterButton_clicked();
		void on_filtersTree_currentItemChanged(QTreeWidgetItem *item);
		void on_editFilterButton_clicked();
		void on_filtersTree_itemDoubleClicked(QTreeWidgetItem *item);

	private:
		Ui::MainWindow *m_ui;
		FilterDialog *m_wndFilters;
};

#endif // WNDMAIN_H
