#include "localfilesystem.h"

LocalFileSystem::LocalFileSystem(QTreeWidget *initialTree, QObject *parent) : QObject(parent)
{
    tree = initialTree; //tree comes from ui
    model = new QFileSystemModel(this); //this is parent

    //set file system root to C for now, change to somehting configurable later
    model->setRootPath("C:/");

    tree->setModel(model);
}

LocalFileSystem::~LocalFileSystem() {
    delete model;
}

void LocalFileSystem::onRootPathChanged(QString newRootPath) {
    model->setRootPath(newRootPath);
}
