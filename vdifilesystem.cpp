#include "vdifilesystem.h"
#include <QList>
#include <QList>
#include <QVariant>
#include <QTime>
#include <QString>
#include <QDebug>
#include "vdifunctions.h"

using namespace std;
using namespace CSCI5806;

VdiFileSystem::VdiFileSystem(QTreeView *initialTree, QObject *parent) : QAbstractItemModel(parent)
{
    tree = initialTree;
    this->setParent(parent);
    rootNode = NULL;
    this->setupModelData();

    tree->setModel(this);
    vdi = new VdiFile();

    fsManager = NULL;
    //signal passthrough
    connect(vdi, VdiFile::vdiFileSelected, this, VdiFileSystem::vdiFileSelected);
    connect(this, VdiFileSystem::onBrowseVDIClicked, vdi, VdiFile::selectVdiPrompt);
    connect(vdi, VdiFile::FSManagerConstructed, this, VdiFileSystem::fsManagerConstructed);
}

VdiFileSystem::~VdiFileSystem() {
    if (rootNode != NULL)
        delete rootNode;
    delete vdi;
}

void VdiFileSystem::setupModelData() {
    QList<QVariant> rootData;
    rootData.push_back(tr("Name"));
    rootData.push_back(tr("Size"));
    rootData.push_back(tr("Type"));
    rootData.push_back(tr("Date Modified"));
    //rootData.push_back(QDateTime::currentDateTime().toString(tr("M/d/yyyy h:mm AP")));
    rootNode = new VDIFileSystemTreeItem(rootData);

    rootData.clear();
    rootData.push_back(tr("/"));
    rootData.push_back(tr("root"));
    rootData.push_back(FileSizeToString(900));
    rootData.push_back(QDateTime::currentDateTime().toString(tr("M/d/yyyy h:mm AP")));

    rootNode->appendChild(new VDIFileSystemTreeItem(rootData, rootNode));
    rootData.clear();
    rootData.push_back(tr("TestDir"));
    rootData.push_back(tr("File Folder"));
    rootData.push_back(FileSizeToString(0));
    rootData.push_back(QDateTime::fromString(tr("1/6/2010 12:35 PM"), tr("M/d/yyyy h:mm AP")).toString("M/d/yyyy h:mm AP"));

    rootNode->appendChild(new VDIFileSystemTreeItem(rootData, rootNode));

    rootData.clear();
    rootData.push_back(tr("TestFile"));
#warning can make a class and override casting to QString and QVariant methods
    rootData.push_back(tr("File"));

    rootData.push_back(FileSizeToString(2050));
    rootData.push_back(QDateTime::fromString(tr("11/26/1989 8:08 AM"), tr("M/d/yyyy h:mm AP")).toString("M/d/yyyy h:mm AP"));

    rootNode->child(0)->appendChild(new VDIFileSystemTreeItem(rootData, rootNode->child(0)));

}

void VdiFileSystem::fsManagerConstructed(ext2FileSystemManager *fs) {
    fsManager = fs;
#warning need more
}


//mandantory overloads for gui display of file model
QModelIndex VdiFileSystem::index(int row, int column, const QModelIndex &parent) const {
    //if we have not created an index for this item, return default index
    if (!this->hasIndex(row, column, parent))
        return QModelIndex();

    VDIFileSystemTreeItem *parentItem;
    if (!parent.isValid())
        parentItem = rootNode;
    else
        parentItem = static_cast<VDIFileSystemTreeItem*>(parent.internalPointer());

    VDIFileSystemTreeItem *childItem = parentItem->child(row);
    if (childItem) //!= NULL
        return createIndex(row, column, childItem);
    else //we are requesting an invalid row
        return QModelIndex();
}

QModelIndex VdiFileSystem::parent(const QModelIndex &child) const {
    //invalid input, invalid return
    if (!child.isValid())
        return QModelIndex();

    VDIFileSystemTreeItem *childItem = static_cast<VDIFileSystemTreeItem*>(child.internalPointer());
    VDIFileSystemTreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootNode)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int VdiFileSystem::rowCount(const QModelIndex &parent) const {
    VDIFileSystemTreeItem *parentItem;

    //invalid input, invalid return
    if(parent.column() > 0)
        return 0;

    if(!parent.isValid())
        parentItem = rootNode;
    else
        parentItem = static_cast<VDIFileSystemTreeItem*>(parent.internalPointer());

    //qDebug() << tr("row count") << parentItem->childCount();
    return parentItem->childCount();
}

int VdiFileSystem::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return static_cast<VDIFileSystemTreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootNode->columnCount();
}

QVariant VdiFileSystem::data(const QModelIndex &index, int role) const {
    //invalid input, invalid return
    if(!index.isValid())
        return QVariant();

    //we cannot currently handle roles other than display
    if (role != Qt::DisplayRole)
        return QVariant();

    VDIFileSystemTreeItem *item = static_cast<VDIFileSystemTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags VdiFileSystem::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant VdiFileSystem::headerData(int section, Qt::Orientation orientation, int role) const {
#warning one of these are false, and we have no header
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootNode->data(section);

    return QVariant();
}

bool VdiFileSystem::hasChildren(const QModelIndex &parent) const {
#warning lazy loading here, determine whether parent is expanded
    bool ret = (rowCount(parent) > 0) && (columnCount(parent) > 0);
    return ret;
}
