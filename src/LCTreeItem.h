#ifndef LCTREEITEM_H
#define LCTREEITEM_H

#include <QtGui/QTreeWidgetItem>

class LCTreeItem: public QTreeWidgetItem
{
  public:
    LCTreeItem(): QTreeWidgetItem() { }

  private:
    bool operator<(const QTreeWidgetItem &other) const;
};

#endif // LCTREEITEM_H
