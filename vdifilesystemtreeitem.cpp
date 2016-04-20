#include "vdifilesystemtreeitem.h"
#include <QDebug>

using namespace CSCI5806;

VDIFileSystemTreeItem::VDIFileSystemTreeItem(const QList<QVariant> &data, VDIFileSystemTreeItem *parent) {
    m_parentItem = parent;
    itemData = data;
}

VDIFileSystemTreeItem::~VDIFileSystemTreeItem() {
    qDeleteAll(childItems); //delete tree - call delete root on tree
}

void VDIFileSystemTreeItem::appendChild(VDIFileSystemTreeItem *child) {
    childItems.append(child);
}

VDIFileSystemTreeItem* VDIFileSystemTreeItem::child(int row) {
    return childItems.value(row);
}

int VDIFileSystemTreeItem::childCount() const {
#warning this is where lazy loading can happen
    //if childItems.count is 0, we can check with the vdi file if there are any entries in that directory
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
        //qDebug() << "item row " << r;
    }
    return 0; //root node
}

VDIFileSystemTreeItem* VDIFileSystemTreeItem::parentItem() {
    return m_parentItem;
}
