#ifndef LCFILTER_H
#define LCFILTER_H

#include <QtCore/QStringList>

class LCFilter
{
  public:
    LCFilter(QString fileName);

    QString name() { return m_name; }
    QString fileName() { return m_fileName; }
    QStringList includes() { return m_includes; }
    QStringList excludes() { return m_excludes; }
    QRegExp comments() { return m_comments; }

  private:
    QString m_name;
    QString m_fileName;
    QStringList m_includes;
    QStringList m_excludes;
    QRegExp m_comments;
};

#endif // LCFILTER_H
