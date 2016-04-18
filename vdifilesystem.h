#ifndef VDIFILESYSTEM_H
#define VDIFILESYSTEM_H

#include <QAbstractItemModel>
#include <QTreeView>
#include "vdifile.h"
#include "vdifilesystemtreeitem.h"

namespace CSCI5806 {
class VdiFileSystem : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit VdiFileSystem(QTreeView *intialTree, VdiFile *file, QObject *parent = 0);
    ~VdiFileSystem();
    void setupModelData();

    //mandantory method overloads for QAbstractItemModel inheritance
    virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE; //alternative to rowCount
    virtual int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;


private:
    QTreeView *tree;
    VdiFile *vdi;
    VDIFileSystemTreeItem *rootNode;
};
}

#endif // VDIFILESYSTEM_H
