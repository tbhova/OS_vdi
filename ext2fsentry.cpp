#include "ext2fsentry.h"

using namespace std;
using namespace CSCI5806;

ext2FSEntry::ext2FSEntry(InodeTable tab, QString entryName)
{
    table = tab;
    name = entryName;
}

#warning
bool isFolder() {
    //get
    return false;
}

#warning
bool isFile() {
    return false;
}
