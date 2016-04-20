#ifndef EXT2FILESYSTEMMANAGER_H
#define EXT2FILESYSTEMMANAGER_H

//#include <QObject>
#include "ext2folder.h"
#include "ext2file.h"
#include <fstream>
#include "ext2superblock.h"

namespace CSCI5806 {

class ext2FileSystemManager
{
public:
    ext2FileSystemManager(std::ifstream *file, long long inodeAddress, int InodeNumber, ext2SuperBlock super);

    void exploreToPath(QString path);

    void addFilesAndFolders(ext2Folder *folder);

    //used for gui traversal
    const ext2Folder* getRoot() const;

    void getInodeTableData(unsigned int inodeNumber);

private:
    ext2Folder *root;
    std::ifstream *input;
    long long iNodeTableAddress;
    ext2SuperBlock *superBlock;
    InodeTable *tab;
};
}

#endif // EXT2FILESYSTEMMANAGER_H
