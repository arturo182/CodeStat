#include "filter.h"

#include <QJsonDocument>
#include <QApplication>
#include <QJsonObject>
#include <QDebug>
#include <QFile>

Filter::Filter(const QString &fileName):
	m_fileName(fileName)
{
	QFile file(fileName);
	if(file.open(QIODevice::ReadOnly)) {
		QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
		file.close();

		const QJsonObject rootObj = doc.object();
		m_name = rootObj.value("name").toString();
		m_comments = QRegExp(rootObj.value("comments").toString());
		m_includes = rootObj.value("includes").toVariant().toStringList();
		m_excludes = rootObj.value("excludes").toVariant().toStringList();
	}
}

void Filter::save()
{
	QJsonDocument doc;

	QJsonObject rootObj;
	rootObj.insert("name", m_name);
	rootObj.insert("comments", m_comments.pattern());
	rootObj.insert("includes", QJsonValue::fromVariant(QVariant::fromValue(m_includes)));
	rootObj.insert("excludes", QJsonValue::fromVariant(QVariant::fromValue(m_excludes)));
	doc.setObject(rootObj);

	QFile file(m_fileName);
	if(file.open(QFile::ReadWrite | QIODevice::Truncate)) {
		file.write(doc.toJson());
		file.close();
	}
}
