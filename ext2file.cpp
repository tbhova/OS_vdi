#include "ext2file.h"

using namespace std;
using namespace CSCI5806;

ext2File::ext2File(InodeTable tab, unsigned int iNodeNum, QString entryName) : ext2FSEntry(tab, iNodeNum, entryName)
{

}

