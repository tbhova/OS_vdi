#ifndef VDIFILESYSTEMTREEITEM_H
#define VDIFILESYSTEMTREEITEM_H

#include <QList>
#include <QVariant>

//node entry in vdi file system tree view
//mostly borrowed code from QT's online examples

namespace CSCI5806 {

class VDIFileSystemTreeItem
{
public:
    explicit VDIFileSystemTreeItem(const QList<QVariant> &data, VDIFileSystemTreeItem *parent = 0);
    ~VDIFileSystemTreeItem();

    void appendChild(VDIFileSystemTreeItem *child);

    VDIFileSystemTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    VDIFileSystemTreeItem* parentItem();

private:
    QList<VDIFileSystemTreeItem*> childItems;
    QList<QVariant> itemData;
    VDIFileSystemTreeItem *m_parentItem;
};
}

#endif // VDIFILESYSTEMTREEITEM_H
