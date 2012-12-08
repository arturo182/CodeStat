#include "filter.h"

#include <QApplication>
#include <QDomDocument>
#include <QDebug>
#include <QFile>

Filter::Filter(const QString &fileName):
	m_fileName(fileName)
{
	QFile file(qApp->applicationDirPath() + "/data/filters/" + fileName);
	if(file.open(QIODevice::ReadOnly)) {
		QDomDocument doc;
		doc.setContent(&file);

		QDomElement root = doc.documentElement();
		QDomNodeList nodes = root.childNodes();
		for(int i = 0; i < nodes.count(); i++) {
			QDomElement elem = nodes.at(i).toElement();

			if(elem.nodeName() == "info") {
				m_name = elem.attribute("name");
			} else if(elem.nodeName() == "includes") {
				QDomNodeList includes = elem.elementsByTagName("include");
				for(int j = 0; j < includes.count(); j++) {
					QString include = includes.at(j).toElement().attribute("name");
					if(!include.isEmpty()) {
						m_includes.append(include);
					}
				}
			} else if(elem.nodeName() == "excludes") {
				QDomNodeList excludes = elem.elementsByTagName("exclude");
				for(int j = 0; j < excludes.count(); j++) {
					QString exclude = excludes.at(j).toElement().attribute("name");
					if(!exclude.isEmpty()) {
						m_excludes.append(exclude);
					}
				}
			} else if(elem.nodeName() == "comments") {
				m_comments = QRegExp(elem.attribute("regexp"));
			}
		}
	}

	file.close();
}
