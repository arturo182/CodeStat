#ifndef LCFILTER_H
#define LCFILTER_H

#include <QtCore/QStringList>

class LCFilter
{
  public:
    LCFilter(QString fileName);

  private:
    QString m_name;
    QString m_fileName;
    QStringList m_includes;
    QStringList m_excludes;
    QRegExp m_comments;
};

#endif // LCFILTER_H
