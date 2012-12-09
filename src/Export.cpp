#include "export.h"

#include <QTreeWidgetItem>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QDebug>
#include <QDate>
#include <QFile>
#include <QTime>

Export::Export(const QString &fileName) :
	m_fileName(fileName)
{
	QFile file(fileName);
	if(file.open(QIODevice::ReadOnly)) {
		QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
		file.close();

		const QJsonObject rootObj = doc.object();

		m_name = rootObj.value("name").toString();
		m_ext = rootObj.value("extension").toString();
		m_append = rootObj.value("append").toString();
		m_item = rootObj.value("item").toString();
		m_prepend = rootObj.value("prepend").toString();
	}
}

QString Export::createItem(const QTreeWidgetItem *item)
{
	QString code = m_item;

	code.replace("%FILENAME%", item->text(0));
	code.replace("%FILESIZE%", item->text(1));
	code.replace("%FILESIZEB%", item->data(1, Qt::UserRole).toString());
	code.replace("%LINES_SOURCE%", item->text(2));
	code.replace("%LINES_COMMENTS%", item->text(3));
	code.replace("%LINES_EMPTY%", item->text(4));
	code.replace("%LINES_TOTAL%", item->text(5));
	code.replace("%PERCENT_SOURCE%", QString::number(item->text(2).toInt() * 100.0 / item->text(5).toInt(), 'f', 2) + "%");
	code.replace("%PERCENT_COMMENTS%", QString::number(item->text(3).toInt() * 100.0 / item->text(5).toInt(), 'f', 2) + "%");
	code.replace("%PERCENT_EMPTY%", QString::number(item->text(4).toInt() * 100.0 / item->text(5).toInt(), 'f', 2) + "%");

	return code;
}

QString Export::prepend(const QStringList &totalStats)
{
	return createCode(m_prepend, totalStats);
}

QString Export::append(const QStringList &totalStats)
{
	return createCode(m_append, totalStats);
}

QString Export::createCode(const QString &templ, const QStringList &totalStats)
{
	QString code = templ;

	code.replace("%TOTAL_FILECOUNT%", totalStats.at(0));
	code.replace("%TOTAL_FILESIZE%", totalStats.at(2));
	code.replace("%TOTAL_FILESIZEB%", totalStats.at(1));
	code.replace("%TOTAL_LINES_SOURCE%", totalStats.at(3));
	code.replace("%TOTAL_LINES_COMMENTS%", totalStats.at(4));
	code.replace("%TOTAL_LINES_EMPTY%", totalStats.at(5));
	code.replace("%TOTAL_LINES%", totalStats.at(6));
	code.replace("%TOTAL_PERCENT_SOURCE%", QString::number(totalStats.at(3).toInt() * 100.0 / totalStats.at(6).toInt(), 'f', 2) + "%");
	code.replace("%TOTAL_PERCENT_COMMENTS%", QString::number(totalStats.at(4).toInt() * 100.0 / totalStats.at(6).toInt(), 'f', 2) + "%");
	code.replace("%TOTAL_PERCENT_EMPTY%", QString::number(totalStats.at(5).toInt() * 100.0 / totalStats.at(6).toInt(), 'f', 2) + "%");
	code.replace("%CURRENT_DATE%", QDate::currentDate().toString());
	code.replace("%CURRENT_TIME%", QTime::currentTime().toString());

	return code;
}
