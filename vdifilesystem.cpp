#include "vdifilesystem.h"
#include <QList>
#include <QList>
#include <QVariant>
#include <QTime>
#include <QString>
#include <QDebug>
#include "vdifunctions.h"
#include "linuxstat.h"

//so we can omit the long stuff for readability
#define lsl

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
#ifdef  lsl
    rootData.push_back(tr("Permissions"));
    rootData.push_back(tr("Hard Links"));
    rootData.push_back(tr("Owner"));
    rootData.push_back(tr("Group"));
#endif
    rootNode = new VDIFileSystemTreeItem(rootData, NULL, NULL);

    tree->setModel(this);
    vdi = new VdiFile();

    fsManager = NULL;
    //signal passthrough
    connect(vdi, &VdiFile::vdiFileSelected, this, &VdiFileSystem::vdiFileSelected);
    connect(this, &VdiFileSystem::onBrowseVDIClicked, vdi, &VdiFile::selectVdiPrompt);
    connect(vdi, &VdiFile::FSManagerConstructed, this, &VdiFileSystem::fsManagerConstructed);

    connect(tree, &QTreeView::expanded, this, &VdiFileSystem::folderExpanded);
    connect(this, &VdiFileSystem::transferToLocalFS, vdi, &VdiFile::transferToLocalFS);
    connect(this, &VdiFileSystem::transferToVDI, vdi, &VdiFile::transferToVDI);
    connect(vdi, &VdiFile::progressUpdate, this, &VdiFileSystem::progressUpdate);
    connect(vdi, &VdiFile::updateFolder, this, &VdiFileSystem::folderExpanded);
    connect(vdi, &VdiFile::updateRoot, this, &VdiFileSystem::updateRootNode);
}

VdiFileSystem::~VdiFileSystem() {
    if (rootNode != NULL)
        delete rootNode;
    delete vdi;
}

void VdiFileSystem::setupModelData(ext2FSEntry *extNode, VDIFileSystemTreeItem *guiNode) {
    if (guiNode->getExt2Entry() == NULL) {
      //do nothing this is the root node
    } else if (guiNode->getExt2Entry()->isFolder()) { //guiNode represent a folder
        for (int i = 0; i < guiNode->childCount(); i++) {
            if (guiNode->child(i)->getExt2Entry()->isFolder()) { //child is folder
                if (!extNode->isFolder())
                    continue; //we don't want to compare folders and files
                if (*(static_cast<ext2Folder*>(extNode)) == *(static_cast<ext2Folder*>(guiNode->child(i)->getExt2Entry()))) {
                    qDebug() << "don't add duplicate folder " << extNode->getName();
                    return; // don't add duplicate folder to gui
                }
            } else { //child guiNode is file
                if (extNode->isFolder()) {
                    continue; //don't compare a file and folder
                }
                if (*(static_cast<ext2File*>(extNode)) == *(static_cast<ext2File*>(guiNode->child(i)->getExt2Entry()))) {
                    qDebug() << "don't add duplicate file " << extNode->getName();
                    return; // don't add duplicate folder to gui
                }
            }
        }
    } else { //guiNode represents a file
        qDebug () << "Error - setupModelData guiNode is file. This probably shouldn't happen";
    }

    QList<QVariant> data;
    QString permissions = "";

    data.push_back(extNode->getName());
    data.push_back(FileSizeToString(extNode->getInodeTable()->i_size));
    if (extNode->isFolder()) {
        data.push_back(QObject::tr("Folder"));
        permissions.append("d");
    } else {
        data.push_back(QObject::tr("File"));
        permissions.append("-");
    }
    QDateTime time;
    time.setTime_t(extNode->getInodeTable()->i_mtime);
    data.push_back(time.toString(QObject::tr("M/d/yyyy h:mm AP")));

#ifdef lsl
    unsigned short mode = extNode->getInodeTable()->i_mode;
    if ((mode & S_IRWXU) == S_IRWXU) {
        permissions.append("rwx");
    } else {
        if ((mode & S_IRUSR) > 0) {
            permissions.append("r");
        } else {
            permissions.append("-");
        }
        if ((mode & S_IWUSR) > 0) {
            permissions.append("w");
        } else {
            permissions.append("-");
        }
        if ((mode & S_IXUSR) > 0) {
            permissions.append("x");
        } else {
            permissions.append("-");
        }
    }

    if ((mode & S_IRWXG) == S_IRWXG) {
        permissions.append("rwx");
    } else {
        if ((mode & S_IRGRP) > 0) {
            permissions.append("r");
        } else {
            permissions.append("-");
        }
        if ((mode & S_IWGRP) > 0) {
            permissions.append("w");
        } else {
            permissions.append("-");
        }
        if ((mode & S_IXGRP) > 0) {
            permissions.append("x");
        } else {
            permissions.append("-");
        }
    }

    if ((mode & S_IRWXO) == S_IRWXO) {
        permissions.append("rwx");
    } else {
        if ((mode & S_IROTH) > 0) {
            permissions.append("r");
        } else {
            permissions.append("-");
        }
        if ((mode & S_IWOTH) > 0) {
            permissions.append("w");
        } else {
            permissions.append("-");
        }
        if ((mode & S_IXOTH) > 0) {
            permissions.append("x");
        } else {
            permissions.append("-");
        }
    }

    data.push_back(permissions);
    data.push_back(QString::number(extNode->getInodeTable()->i_links_count));
    data.push_back(QString::number(extNode->getInodeTable()->i_uid));
    data.push_back(QString::number(extNode->getInodeTable()->i_gid));
#endif

    //special sauce
    static bool initialized = false;
    if (initialized && extNode->getName() == "/") {
        qDebug() << "Special Sauce alert - setup model data - don't add duplicate root";
    } else {
        guiNode->appendChild(new VDIFileSystemTreeItem(data, guiNode, extNode));
    }
    initialized = true;
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
}

void VdiFileSystem::fsManagerConstructed(ext2FileSystemManager *fs) {
    qDebug() << QObject::tr("fsManager Constructed!");
    fsManager = fs;
    this->updateRootNode();
}

//slot detecting when a folder is expanded for lazy loading
void VdiFileSystem::folderExpanded(const QModelIndex &index) {
//    qDebug() << "folder expanded slot";

    if (!index.isValid()) {
//        qDebug() << QObject::tr("folderExpanded - invalid model index");
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

    //build path by walking up the tree
    while (parent != rootNode) {
        path.append("/");
        path.append(parent->data(0).toString());
        parent = parent->parentItem();
    }

//    qDebug() << "folder expanded path = " << path;
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


    //qDebug() << "folder expanded revPath = " << revPath;

    if (!fsManager->exploreToPath(revPath))
        return; //folder already explored

    emit this->layoutAboutToBeChanged();
    //setupModelData(fsManager->getRoot(), rootNode);

    //traverse fsManagerTree to get expanded folder
    ext2Folder *fsManagerFolder = fsManager->getFolderAtPath(revPath);

    //qDebug() << "fsManagerFolder name " << fsManagerFolder->getName();
    //qDebug() << "expanded folder name " << expandedFolder->data(0).toString();

    foreach (ext2Folder *f, *(fsManagerFolder->getFolders())) {
        //qDebug() << "fsManager sub Folder name " << f->getName();
        for (int i = 0; i < expandedFolder->childCount(); i++) {
            if (f->getName() == expandedFolder->child(i)->data(0).toString()) {
                //qDebug() << "found matching folders in both trees";
                foreach (ext2Folder *subFolder, *(f->getFolders())) {
                    setupModelData(subFolder, expandedFolder->child(i));
                }
                foreach (ext2File *subFile, *(f->getFiles())) {
                    setupModelData(subFile, expandedFolder->child(i));
                }
                break;
            }
        }
    }

    emit this->layoutChanged();
}

void VdiFileSystem::updateRootNode() {
    emit this->layoutAboutToBeChanged();
    setupModelData(fsManager->getRoot(), rootNode);
    emit this->layoutChanged();
}

ext2FSEntry* VdiFileSystem::getExt2Entry(const QModelIndex &index) const {
    VDIFileSystemTreeItem *item;

    //invalid input, invalid return
    if(index.column() > 0)
        return 0;

    if(!index.isValid())
        return NULL;
    else
        item = static_cast<VDIFileSystemTreeItem*>(index.internalPointer());
    return item->getExt2Entry();
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
    bool ret = (rowCount(parent) > 0) && (columnCount(parent) > 0);
    return ret;
}
