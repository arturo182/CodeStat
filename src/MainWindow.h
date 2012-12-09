#ifndef WNDMAIN_H
#define WNDMAIN_H

#include <QMainWindow>

class QTreeWidgetItem;

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

		void reloadFilters();
		void loadExports();

		QStringList fileList(const QString &fileDir, const QStringList &filters, const QStringList &excludes);
		QString calcFileSize(const qint64 &bytes);

	protected:
		void closeEvent(QCloseEvent *event);

	private slots:
		void resizeStats(const int &index, const int &oldSize, const int &newSize);
		void updateDirButtons(QTreeWidgetItem *current);
		void openFile(QTreeWidgetItem *item);
		void updateFiltersButtons(QTreeWidgetItem *item);
		void on_filtersTree_itemDoubleClicked(QTreeWidgetItem *item);
		void addDir();
		void removeDir();
		void start();
		void saveExport();
		void newFilter();
		void editFilter();
		void deleteFilter();

	private:
		void readSettings();
		void writeSettings();

	private:
		Ui::MainWindow *m_ui;
};

#endif // WNDMAIN_H
