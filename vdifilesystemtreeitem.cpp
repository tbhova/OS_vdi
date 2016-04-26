#include "vdifilesystemtreeitem.h"
#include <QDebug>


using namespace CSCI5806;

VDIFileSystemTreeItem::VDIFileSystemTreeItem(const QList<QVariant> &data, VDIFileSystemTreeItem *parent, ext2FSEntry *fsPointer) {
    m_parentItem = parent;
    itemData = data;
    fsNode = fsPointer;
}

VDIFileSystemTreeItem::~VDIFileSystemTreeItem() {
    qDeleteAll(childItems); //delete tree - call delete root on tree - deletes recursively
}

ext2FSEntry* VDIFileSystemTreeItem::getExt2Entry() const {
    return fsNode;
}

void VDIFileSystemTreeItem::appendChild(VDIFileSystemTreeItem *child) {
    childItems.append(child);
}

VDIFileSystemTreeItem* VDIFileSystemTreeItem::child(int row) {
    return childItems.value(row);
}

int VDIFileSystemTreeItem::childCount() const {
    return childItems.count();
}

int VDIFileSystemTreeItem::columnCount() const {
    return itemData.count();
}

QVariant VDIFileSystemTreeItem::data(int column) const {
    return itemData.value(column);
}

int VDIFileSystemTreeItem::row() const {
    if (m_parentItem) {
        return m_parentItem->childItems.indexOf(const_cast<VDIFileSystemTreeItem*>(this));
    }
    return 0; //root node
}

VDIFileSystemTreeItem* VDIFileSystemTreeItem::parentItem() {
    return m_parentItem;
}
