#ifndef VDIFILESYSTEMTREEITEM_H
#define VDIFILESYSTEMTREEITEM_H

#include <QList>
#include <QVariant>
#include "ext2fsentry.h"

namespace CSCI5806 {

class VDIFileSystemTreeItem
{
public:
    explicit VDIFileSystemTreeItem(const QList<QVariant> &data, VDIFileSystemTreeItem *parent, ext2FSEntry *fsPointer);
    ~VDIFileSystemTreeItem();

    void appendChild(VDIFileSystemTreeItem *child);

    VDIFileSystemTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    VDIFileSystemTreeItem* parentItem();
    ext2FSEntry* getExt2Entry() const;

private:
    QList<VDIFileSystemTreeItem*> childItems;
    QList<QVariant> itemData;
    VDIFileSystemTreeItem *m_parentItem;
    ext2FSEntry *fsNode;
};
}

#endif // VDIFILESYSTEMTREEITEM_H
