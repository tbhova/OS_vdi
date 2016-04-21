#ifndef EXT2FILE_H
#define EXT2FILE_H

//#include <QObject>
#include <QString>
#include "ext2fsentry.h"

namespace CSCI5806 {

class ext2File : public ext2FSEntry
{
public:
    ext2File(InodeTable tab, unsigned int iNodeNum, QString entryName);

private:
};

}

#endif // EXT2FILE_H
