#include "vdifilesystem.h"

using namespace std;
using namespace CSCI5806;

VdiFileSystem::VdiFileSystem(QTreeView *initialTree, VdiFile *file, QObject *parent) : QAbstractItemModel(parent)
{
    tree = initialTree;
    this->setParent(parent);
    vdi = file;
}

VdiFileSystem::~VdiFileSystem() {

}

//mandantory overloads (full of junk to get rid of warnings
QModelIndex VdiFileSystem::index(int row, int column, const QModelIndex &parent) const {
    row++;
    column++;
    parent.isValid();
    QModelIndex ret;
    return ret;
}

QModelIndex VdiFileSystem::parent(const QModelIndex &child) const {
    child.column();
    QModelIndex ret;
    return ret;
}

int VdiFileSystem::rowCount(const QModelIndex &parent) const {
    parent.column();
    return 0;
}

int VdiFileSystem::columnCount(const QModelIndex &parent) const {
    parent.column();
    return 0;
}

QVariant VdiFileSystem::data(const QModelIndex &index, int role) const {
    index.column();
    role++;
    QVariant ret;
    return ret;
}
