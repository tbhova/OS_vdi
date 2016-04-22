#include "ext2folder.h"

using namespace std;
using namespace CSCI5806;

ext2Folder::ext2Folder(InodeTable tab, unsigned int iNodeNum, QString entryName) : ext2FSEntry(tab, iNodeNum, entryName)
{
    folders = new QVector<ext2Folder*>;
    files = new QVector<ext2File*>;
}

QVector<ext2Folder*>* ext2Folder::getFolders() {
    return folders;
}

QVector<ext2File*>* ext2Folder::getFiles() {
    return files;
}

bool ext2Folder::isFolder() {
    return true;
}

/*void ext2Folder::setName(QString newName) {
    ext2FSEntry::setName(newName);
    path.chop(path.size() - path.lastIndexOf("/")+1); //remove old folder name
    path.append(newName); //add new folder name
}

/*void ext2Folder::setPath(QString newPath) {
    path = newPath;
}

QString ext2Folder::getPath() const {
    return path;
}*/
