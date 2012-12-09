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
		QString m_fileName;
		QString m_prepend;
		QString m_append;
		QString m_name;
		QString m_item;
		QString m_ext;
};

#endif // EXPORT_H
