#ifndef FILTER_H
#define FILTER_H

#include <QStringList>

class Filter
{
	public:
		Filter(const QString &fileName);

		QString name() const { return m_name; }
		QString fileName() const { return m_fileName; }
		QStringList includes() const { return m_includes; }
		QStringList excludes() const { return m_excludes; }
		QRegExp comments() const { return m_comments; }

	private:
		QString m_name;
		QString m_fileName;
		QStringList m_includes;
		QStringList m_excludes;
		QRegExp m_comments;
};

#endif // FILTER_H
