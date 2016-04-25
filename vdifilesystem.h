#ifndef VDIFILESYSTEM_H
#define VDIFILESYSTEM_H

#include <QAbstractItemModel>
#include <QTreeView>
#include <QString>
#include "vdifile.h"
#include "vdifilesystemtreeitem.h"
#include "ext2filesystemmanager.h"
#include "ext2fsentry.h"

namespace CSCI5806 {
class VdiFileSystem : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit VdiFileSystem(QTreeView *intialTree, QObject *parent = 0);
    ~VdiFileSystem();
    void setupModelData(ext2FSEntry *extNode, VDIFileSystemTreeItem *guiNode);
    ext2FSEntry* getExt2Entry(const QModelIndex &index) const;

    //mandantory method overloads for QAbstractItemModel inheritance
    virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE; //alternative to rowCount
    virtual int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;

signals:
    void vdiFileSelected(QString fileName);
    void onBrowseVDIClicked();
    void transferToLocalFS(CSCI5806::ext2File *sourceFile, QDir *destDir);
    void transferToVDI(CSCI5806::ext2Folder *VDIFolder, QFileInfo *sourceFile);

private slots:
    void fsManagerConstructed(ext2FileSystemManager *fs);
    void folderExpanded(const QModelIndex &index);

private:
    QTreeView *tree;
    VdiFile *vdi;
    VDIFileSystemTreeItem *rootNode;
    ext2FileSystemManager *fsManager;
};
}

#endif // VDIFILESYSTEM_H
