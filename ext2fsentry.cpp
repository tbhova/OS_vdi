#include "ext2fsentry.h"

using namespace std;
using namespace CSCI5806;

ext2FSEntry::ext2FSEntry(InodeTable tab, unsigned int iNodeNum, QString entryName)
{
    table = tab;
    iNodeNumber = iNodeNum;
    name = entryName;
}

/*
#warning
bool isFolder() {
    //get
    return false;
}

#warning
bool isFile() {
    return false;
}*/

InodeTable& ext2FSEntry::getInodeTable() {
    return table;
}

void ext2FSEntry::setName(QString newName) {
    name = newName;
}

QString ext2FSEntry::getName() const {
    return name;
}
