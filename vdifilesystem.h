#ifndef VDIFILESYSTEM_H
#define VDIFILESYSTEM_H

#include <QAbstractItemModel>
#include <QTreeView>
#include "vdifile.h"

class VdiFileSystem : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit VdiFileSystem(QTreeView *intialTree, VdiFile *file, QObject *parent = 0);
    ~VdiFileSystem();

    //mandantory method overloads for QAbstractItemModel inheritance
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;


private:
    QTreeView *tree;
    VdiFile *vdi;
};

#endif // VDIFILESYSTEM_H
