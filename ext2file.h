#ifndef EXT2FILE_H
#define EXT2FILE_H

#include "ext2fsentry.h"

/*
 * Sub class of ext2FSEntry
 * Different from folder because a file will always be a leaf node of a tree, and make traversing simpler
 * Pretty much inherits everything from ext2FSEntry
 * */

namespace CSCI5806 {

class ext2File : public ext2FSEntry
{
public:
    ext2File(InodeTable tab, unsigned int iNodeNum, QString entryName);
    virtual ~ext2File();
    virtual bool isFolder();
    virtual bool operator==(const ext2File &other) const;
};

}

#endif // EXT2FILE_H
