#ifndef LCEXPORT_H
#define LCEXPORT_H

#include <QtCore/QString>

class QTreeWidgetItem;

class LCExport
{
  public:
    LCExport(const QString &fileName);

    QString createItem(QTreeWidgetItem *item);
    QString prepend(const QStringList &totalStats);
    QString append(const QStringList &totalStats);

    QString name() { return m_name; }
    QString ext() { return m_ext; }
    QString fileName() { return m_fileName; }

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

#endif // LCEXPORT_H
