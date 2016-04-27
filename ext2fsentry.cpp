#include "ext2fsentry.h"
#include <iostream>

using namespace std;
using namespace CSCI5806;

ext2FSEntry::ext2FSEntry(InodeTable tab, unsigned int iNodeNum, QString entryName)
{
    table = tab;
    iNodeNumber = iNodeNum;
    cout << "iNodeNum entry " << iNodeNum << " " << iNodeNumber << endl;
    name = entryName;
}

InodeTable* ext2FSEntry::getInodeTable() {
    return &table;
}

QString ext2FSEntry::getName() const {
    return name;
}

unsigned int ext2FSEntry::getInodeNumber() const {
    cout << "iNodeNum entry get " << iNodeNumber << endl;
    return iNodeNumber;
}

bool ext2FSEntry::operator==(const ext2FSEntry &other) const {
    if (name != other.name) {
        return false;
    }
    if (iNodeNumber != other.iNodeNumber) {
        return false;
    }
    InodeTable t1 = table, t2 = other.table;
    if (t1.i_blocks != t2.i_blocks) {
        return false;
    }
    for (int i = 0; i < 15; i++) {
        if (t1.i_block[i] != t2.i_block[i]) {
            return false;
        }
    }
    return true;
}
