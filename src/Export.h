#ifndef EXPORT_H
#define EXPORT_H

#include <QString>
#include <QFont>

class QTreeWidgetItem;

class Export
{
	public:
		Export(const QString &fileName);

		QString createItem(const QTreeWidgetItem *item);
		QString prepend(const QStringList &totalStats);
		QString append(const QStringList &totalStats);

		QString name() const { return m_name; }
		QString ext() const { return m_ext; }
		QString fileName() const { return m_fileName; }

	private:
		QString createCode(const QString &templ, const QStringList &totalStats);

	private:
		QString m_name;
		QString m_ext;
		QString m_fileName;
		QString m_prepend;
		QString m_item;
		QString m_append;
};

#endif // EXPORT_H
