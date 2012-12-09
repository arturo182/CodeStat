#ifndef FILTER_H
#define FILTER_H

#include <QStringList>

class Filter
{
	public:
		Filter(const QString &fileName);

		void save();

		void setName(const QString &name) { m_name = name; }
		QString name() const { return m_name; }

		void setFileName(const QString &fileName) { m_fileName = fileName; }
		QString fileName() const { return m_fileName; }

		void setIncludes(const QStringList &includes) { m_includes = includes; }
		QStringList includes() const { return m_includes; }

		void setExcludes(const QStringList &excludes) { m_excludes = excludes; }
		QStringList excludes() const { return m_excludes; }

		void setComments(const QRegExp &comments) { m_comments = comments; }
		QRegExp comments() const { return m_comments; }

	private:
		QStringList m_excludes;
		QStringList m_includes;
		QRegExp m_comments;
		QString m_fileName;
		QString m_name;
};

#endif // FILTER_H
