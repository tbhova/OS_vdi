#include "localfilesystem.h"

using namespace std;
using namespace CSCI5806;

LocalFileSystem::LocalFileSystem(QTreeView *initialTree, QObject *parent) : QObject(parent)
{
    tree = initialTree; //tree comes from ui
    fileSystem = new QFileSystemModel(this); //this is parent

    fileSystem->setRootPath("C://Users//Andy//Desktop//"); //"C:/ also works, but / is system independent

    tree->setModel(fileSystem);
}

LocalFileSystem::~LocalFileSystem() {
    delete fileSystem;
}

void LocalFileSystem::onRootPathChanged(QString newRootPath) {
    fileSystem->setRootPath(newRootPath);
}

QFileSystemModel* LocalFileSystem::getFS() {
    return fileSystem;
}
