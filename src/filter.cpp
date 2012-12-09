#include "filter.h"

#include <QJsonDocument>
#include <QApplication>
#include <QJsonObject>
#include <QFile>

Filter::Filter(const QString &fileName):
	m_fileName(fileName)
{
	QFile file(qApp->applicationDirPath() + "/data/filters/" + fileName);
	if(file.open(QIODevice::ReadOnly)) {
		QJsonParseError error;
		QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
		file.close();

		const QJsonObject rootObj = doc.object();
		m_name = rootObj.value("name").toString();
		m_comments = QRegExp(rootObj.value("comments").toString());
		m_includes = rootObj.value("includes").toVariant().toStringList();
		m_excludes = rootObj.value("excludes").toVariant().toStringList();
	}
}
