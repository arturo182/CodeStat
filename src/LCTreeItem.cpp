#include "LCTreeItem.h"

bool LCTreeItem::operator<(const QTreeWidgetItem &other) const
{
  int column = treeWidget()->sortColumn();

  switch(column) {
    case 2:
    case 3:
    case 4:
    case 5:
      return text(column).toInt() < other.text(column).toInt();
    break;

    case 1:
      return data(1, Qt::UserRole).toInt() < other.data(1, Qt::UserRole).toInt();
    break;

    default:
      return text(column) < other.text(column);
    break;
  }
}
