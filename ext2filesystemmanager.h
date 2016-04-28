#ifndef EXT2FILESYSTEMMANAGER_H
#define EXT2FILESYSTEMMANAGER_H

#include "ext2folder.h"
#include "ext2file.h"
#include <fstream>
#include "ext2superblock.h"
#include "ext2groupdescriptor.h"

namespace CSCI5806 {

struct DirectoryEntry {
    unsigned int	inode;		/* Inode number */
    unsigned short	rec_len;		/* Directory entry length */
    unsigned char 	name_len;		/* Name length */
    unsigned char	file_type;  //1 is file, 2 is folder
    std::string   name;	    /* File name */
};

class ext2FileSystemManager
{
public:
    ext2FileSystemManager(std::fstream *file, ext2GroupDescriptor *group, ext2SuperBlock *super, unsigned int bootBlock);

    ~ext2FileSystemManager();

    ext2Folder* getFolderAtPath(QString path);

    bool exploreToPath(QString path);

    void addFilesAndFolders(ext2Folder *folder);

    void fillInFilesFromBlock(ext2Folder *folder, unsigned int block_num, unsigned long long offsetOfStruct);

    //used for gui traversal
    ext2Folder* getRoot() const;

    void getInodeTableData(unsigned int inodeNumber);

    long long getInodeOffset(unsigned int inodeNumber);

    long long getBlockOffset(unsigned int block_num);

    unsigned int getBlockNumAtIndex(const InodeTable *tab, unsigned int index);

    unsigned int getBlocksPerIndirection(int indirection);



private:
    void addEntry(ext2Folder *folder);

    ext2Folder *root;
    std::fstream *input;
    ext2SuperBlock *superBlock;
    ext2GroupDescriptor *groupDescriptor;
    InodeTable tab;
    DirectoryEntry InodeIn;

    unsigned int bootBlockAddress, block_size;
};
}

#endif // EXT2FILESYSTEMMANAGER_H
