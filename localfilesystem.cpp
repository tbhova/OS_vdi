#include "localfilesystem.h"

LocalFileSystem::LocalFileSystem(QTreeView *initialTree, QObject *parent) : QObject(parent)
{
    tree = initialTree; //tree comes from ui
    fileSystem = new QFileSystemModel(this); //this is parent

    //set file system root to C for now, change to somehting configurable later
    fileSystem->setRootPath("C:/");

    tree->setModel(fileSystem);
}

LocalFileSystem::~LocalFileSystem() {
    delete fileSystem;
}

void LocalFileSystem::onRootPathChanged(QString newRootPath) {
    fileSystem->setRootPath(newRootPath);
}
