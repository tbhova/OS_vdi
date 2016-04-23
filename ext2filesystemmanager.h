#ifndef EXT2FILESYSTEMMANAGER_H
#define EXT2FILESYSTEMMANAGER_H

//#include <QObject>
#include "ext2folder.h"
#include "ext2file.h"
#include <fstream>
#include "ext2superblock.h"

namespace CSCI5806 {

struct Inode_info {
    unsigned int	inode;		/* Inode number */
    unsigned short	rec_len;		/* Directory entry length */
    unsigned char 	name_len;		/* Name length */
    unsigned char	file_type;  //1 is file, 2 is folder
    //QVector <unsigned char>   name;	    /* File name */
    std::string   name;	    /* File name */
};

class ext2FileSystemManager
{
public:
    ext2FileSystemManager(std::ifstream *file, long long inodeAddress, ext2SuperBlock *super, unsigned int bootBlock, unsigned int blockSize);

    ~ext2FileSystemManager();

    ext2Folder* getFolderAtPath(QString path);

    bool exploreToPath(QString path);

    void addFilesAndFolders(ext2Folder *folder);

    bool fillInFilesFromBlock(ext2Folder *folder, unsigned int block_num, unsigned long long offsetOfStruct);

    //used for gui traversal
    ext2Folder* getRoot() const;

    void getInodeTableData(unsigned int inodeNumber);



private:
    /*bool isDirInTable() const;

    bool isFileInTable() const;*/

    void addEntry(ext2Folder *folder, const Inode_info &InodeIn);

    ext2Folder *root;
    std::ifstream *input;
    long long iNodeTableAddress;
    ext2SuperBlock *superBlock;
    InodeTable tab, *tempTab;
    Inode_info InodeIn;

    unsigned int bootBlockLocation, block_size;
};
}

#endif // EXT2FILESYSTEMMANAGER_H
