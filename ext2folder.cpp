#include "ext2folder.h"

using namespace std;
using namespace CSCI5806;

ext2Folder::ext2Folder(InodeTable tab, unsigned int iNodeNum, QString entryName) : ext2FSEntry(tab, iNodeNum, entryName)
{
    folders = new QVector<ext2Folder*>;
    files = new QVector<ext2File*>;
}

ext2Folder::~ext2Folder() {
    for (int i = 0; i < folders->size(); i++)
        delete folders->at(i);
    for (int i = 0; i < files->size(); i++)
        delete files->at(i);

    delete folders;
    delete files;

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

bool ext2Folder::operator==(const ext2Folder &other) const {
    return ext2FSEntry::operator ==(other);
}
