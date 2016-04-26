#include "ext2fsentry.h"
#include <iostream>

using namespace std;
using namespace CSCI5806;

ext2FSEntry::ext2FSEntry(InodeTable tab, unsigned int iNodeNum, QString entryName)
{
    table = tab;
    iNodeNumber = iNodeNum;
    name = entryName;
}

InodeTable* ext2FSEntry::getInodeTable() {
    return &table;
}

void ext2FSEntry::setName(QString newName) {
    name = newName;
}

QString ext2FSEntry::getName() const {
    return name;
}

unsigned int ext2FSEntry::getInodeNumber() const {
    return iNodeNumber;
}

bool ext2FSEntry::operator==(const ext2FSEntry &other) const {
    cout << "== name " << name.toStdString() << " other " << other.name.toStdString() << endl;
    if (name != other.name) {
        cout << "names do not match" << endl;
        return false;
    }
    if (iNodeNumber != other.iNodeNumber) {
        cout << "inode num do not match" << endl;
        return false;
    }
    InodeTable t1 = table, t2 = other.table;
    if (t1.i_blocks != t2.i_blocks) {
        cout << "iBlocks do not match" << endl;
        return false;
    }
    for (int i = 0; i < 15; i++) {
        if (t1.i_block[i] != t2.i_block[i]) {
            cout << "block pointers do not match" << endl;
            return false;
        }
    }
    return true;
}
