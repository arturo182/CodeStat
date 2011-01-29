#include "LCExport.h"

#include <QtCore/QDate>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QTime>
#include <QtGui/QApplication>
#include <QtGui/QTreeWidgetItem>
#include <QtXml/QDomDocument>

LCExport::LCExport(const QString &fileName): m_fileName(fileName)
{
  QFile file(qApp->applicationDirPath() + "/data/exports/" + fileName);
  if(file.open(QIODevice::ReadOnly)) {
    QDomDocument doc;
    doc.setContent(&file);

    QDomElement root = doc.documentElement();
    QDomNodeList nodes = root.childNodes();
    for(int i = 0; i < nodes.count(); i++) {
      QDomElement elem = nodes.at(i).toElement();

      if(elem.nodeName() == "info") {
        m_name = elem.attribute("name");
        m_ext = elem.attribute("ext");
      } else if(elem.nodeName() == "append") {
        if(elem.childNodes().count() > 0) {
          m_append = elem.firstChild().toCDATASection().nodeValue();
        }
      } else if(elem.nodeName() == "item") {
        if(elem.childNodes().count() > 0) {
          m_item = elem.firstChild().toCDATASection().nodeValue();
        }
      } else if(elem.nodeName() == "prepend") {
        if(elem.childNodes().count() > 0) {
          m_prepend = elem.firstChild().toCDATASection().nodeValue();
        }
      }
    }
  }

  file.close();
}

QString LCExport::createItem(QTreeWidgetItem *item)
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

QString LCExport::prepend(const QStringList &totalStats)
{
  return createCode(m_prepend, totalStats);
}

QString LCExport::append(const QStringList &totalStats)
{
  return createCode(m_append, totalStats);
}

QString LCExport::createCode(const QString &templ, const QStringList &totalStats)
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
