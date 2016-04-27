#include "ext2file.h"

using namespace std;
using namespace CSCI5806;

ext2File::ext2File(InodeTable tab, unsigned int iNodeNum, QString entryName) : ext2FSEntry(tab, iNodeNum, entryName)
{

}

bool ext2File::isFolder() {
    return false;
}

bool ext2File::operator==(const ext2File &other) const {
    return ext2FSEntry::operator ==(other);
}
