#ifndef EXT2FILESYSTEMMANAGER_H
#define EXT2FILESYSTEMMANAGER_H

#include "ext2folder.h"
#include "ext2file.h"
#include <fstream>
#include "ext2superblock.h"
#include "ext2groupdescriptor.h"

/*
 * Performs the traversal of the file system starting from the root node using lazy loading.
 * As folders in the gui are expand, this learns which folder was expanded and visits files in that folder.
 * This also will get an inode or block offset for all inodes and blocks
 *
 *
 * */

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

    //parses path and gets the last explored folder
    ext2Folder* getFolderAtPath(QString path);

    //calls getFolderAtPath then performs lazy loading to load in more files
    bool exploreToPath(QString path);

    //explore the folder passed to this method
    void addFilesAndFolders(ext2Folder *folder);

    //explore a directory entry at the block given
    void fillInFilesFromBlock(ext2Folder *folder, unsigned int block_num, unsigned long long offsetOfStruct);

    //used for gui traversal by the vdiFileSystem class
    ext2Folder* getRoot() const;

    //fill in the variable tab with the inode table data of the given inodeNumber
    void getInodeTableData(unsigned int inodeNumber);

    //get the absolute disk offset of the inode
    long long getInodeOffset(unsigned int inodeNumber);

    //get the absolute disk offset of the block
    long long getBlockOffset(unsigned int block_num);

    //gets the index'th block number for a directory entry
    unsigned int getBlockNumAtIndex(const InodeTable *tab, unsigned int index);

    //this was going to be used more, then we forgot we wrote it
    unsigned int getBlocksPerIndirection(int indirection);



private:
    void addEntry(ext2Folder *folder);

    //root node of the FS tree we will build - also represents the root directory
    ext2Folder *root;
    //pointer to our VDI input file
    std::fstream *input;
    ext2SuperBlock *superBlock;
    ext2GroupDescriptor *groupDescriptor;
    InodeTable tab;
    DirectoryEntry InodeIn;

    //value needed to find block and inode offsets
    unsigned int bootBlockAddress, block_size;
};
}

#endif // EXT2FILESYSTEMMANAGER_H
