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
    //build model header (root node)
    QList<QVariant> rootData;
    rootData.push_back(tr("Name"));
    rootData.push_back(tr("Size"));
    rootData.push_back(tr("Type"));
    rootData.push_back(tr("Date Modified"));
    //rootData.push_back(QDateTime::currentDateTime().toString(tr("M/d/yyyy h:mm AP")));
    rootNode = new VDIFileSystemTreeItem(rootData);

    tree->setModel(this);
    vdi = new VdiFile();

    fsManager = NULL;
    //signal passthrough
    connect(vdi, VdiFile::vdiFileSelected, this, VdiFileSystem::vdiFileSelected);
    connect(this, VdiFileSystem::onBrowseVDIClicked, vdi, VdiFile::selectVdiPrompt);
    connect(vdi, VdiFile::FSManagerConstructed, this, VdiFileSystem::fsManagerConstructed);

    connect(tree, QTreeView::expanded, this, VdiFileSystem::folderExpanded);
}

VdiFileSystem::~VdiFileSystem() {
    if (rootNode != NULL)
        delete rootNode;
    delete vdi;
}

void VdiFileSystem::setupModelData(ext2FSEntry *extNode, VDIFileSystemTreeItem *guiNode) {
    QList<QVariant> data;

    data.push_back(extNode->getName());
    data.push_back(FileSizeToString(extNode->getInodeTable().i_size));
    if (extNode->isFolder()) {
        data.push_back(QObject::tr("Folder"));
    } else {
        data.push_back(QObject::tr("File"));
    }

    data.push_back(QString::number(extNode->getInodeTable().i_mtime));

    guiNode->appendChild(new VDIFileSystemTreeItem(data, guiNode));
    qDebug() << QObject::tr("append ") << extNode->getName();

    if (extNode->isFolder()) {
        qDebug() << QObject::tr("this is a folder");
        ext2Folder* folder = (ext2Folder*)extNode;
        for (int i = 0; i < guiNode->childCount(); i++) {
            if (guiNode->child(i)->data(0).toString() == folder->getName()) {
                qDebug() << QObject::tr("this folder found ") << folder->getName();
                guiNode = guiNode->child(i);
                break;
            }
        }
        foreach (ext2Folder *f, *(folder->getFolders()) ) {
            setupModelData(f, guiNode);
        }
        foreach (ext2File *f, *(folder->getFiles()) ) {
            setupModelData(f, guiNode);
        }
    } else return;




    /*rootData.clear();
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

    rootNode->child(0)->appendChild(new VDIFileSystemTreeItem(rootData, rootNode->child(0)));*/

}

void VdiFileSystem::fsManagerConstructed(ext2FileSystemManager *fs) {
    emit this->layoutAboutToBeChanged();
    qDebug() << QObject::tr("fsManager Constructed!");
    fsManager = fs;
    setupModelData(fs->getRoot(), rootNode);
    emit this->layoutChanged();
}

//slot detecting when a folder is expanded for lazy loading
void VdiFileSystem::folderExpanded(const QModelIndex &index) {
    emit this->layoutAboutToBeChanged();

    if (!index.isValid()) {
        qDebug() << QObject::tr("folderExpanded - invalid model index");
        return;
    }
    if (fsManager == NULL) {
        //can't ask vdi to load more files, since there isn't one yet
        return;
    }
    VDIFileSystemTreeItem *expandedFolder;
    expandedFolder = static_cast<VDIFileSystemTreeItem*>(index.internalPointer());
    QString path = "";
    //path.append(expandedFolder->data(0).toString());
    VDIFileSystemTreeItem *parent;
    parent = expandedFolder;

    //build path by walking upn the tree
    while (parent != rootNode) {
        path.append("/");
        path.append(parent->data(0).toString());
        parent = parent->parentItem();
    }

    qDebug() << "folder expanded path = " << path;
    //reverse the path to be the actual path starting from the root
    QString revPath;
    int lastSlash = path.lastIndexOf("/");
    while (lastSlash != -1) {
        //qDebug() << "lastSlash = " << lastSlash;
        revPath.append(path.right(path.size()-lastSlash));
        path.chop(path.size()-lastSlash);
        lastSlash = path.lastIndexOf("/");
        //qDebug() << "folder expanded revPath = " << path;
        //qDebug() << "folder expanded path = " << path;
    }

    //qDebug() << "folder expanded revPath = " << revPath;

    //remove leading /'s
    for (int i = 0; i < revPath.length(); i++) {
        int leadingSlashes = revPath.indexOf("/", i);
        if (leadingSlashes != i) {
            revPath.remove(0, i-1); //remove duplicate /'s
            break;
        }
        //i == size-1 is last iteration, and root is being expanded
        if (i == revPath.size()-1) {
            revPath.remove(0, i);
            break;
        }
    }


    qDebug() << "folder expanded revPath = " << revPath;

    fsManager->exploreToPath(revPath);

    emit this->layoutChanged();
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
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootNode->data(section);

    return QVariant();
}

bool VdiFileSystem::hasChildren(const QModelIndex &parent) const {
#warning lazy loading here, determine whether parent is expanded
    bool ret = (rowCount(parent) > 0) && (columnCount(parent) > 0);
    return ret;
}
