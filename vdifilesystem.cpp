#include "vdifilesystem.h"
#include <QMessageBox>
#include <QFileDialog>

VdiFileSystem::VdiFileSystem(QTreeView *initialTree, QObject *parent) : QAbstractItemModel(parent)
{
    tree = initialTree;
    vdi = new VdiFile();
    this->setParent(parent);
}

VdiFileSystem::~VdiFileSystem() {
    delete vdi;
}

void VdiFileSystem::selectVdiPrompt() {
    //Open File
    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Please open a .vdi file"), "C://", ".VDI File (*.*);;All Files (*.*)");

    QMessageBox::information(NULL,tr("FileNameOut"),fileName);
    emit(this->vdiFileSelected(fileName));
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
