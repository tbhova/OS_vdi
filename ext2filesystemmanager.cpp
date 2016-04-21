#include "ext2filesystemmanager.h"
#include "vdifunctions.h"
#include <QObject>
#include <sstream>
#include <iostream>

using namespace std;
using namespace CSCI5806;

ext2FileSystemManager::ext2FileSystemManager(ifstream *file, long long inodeAddress, ext2SuperBlock *super, unsigned int bootBlock, unsigned int blockSize)
{
    input = file;
    iNodeTableAddress = inodeAddress;
    superBlock = super;
    bootBlockLocation = bootBlock;
    block_size = blockSize;
    //get root info
    getInodeTableData(2); //root is inode 2
    root = new ext2Folder(*tab, 2, QObject::tr("/"));
    //root->setPath("/");
    addFilesAndFolders(root);
}

void ext2FileSystemManager::exploreToPath(QString path) {
    //traverse folder from root to path
    //if last folder has non empty vectors return early

    //start at root and get next folder from path
    //repeat for new path

    //call addfilesFolders
}

void ext2FileSystemManager::addFilesAndFolders(ext2Folder *folder) {
    tab = &(folder->getInodeTable());
    for (int i = 0; i < (tab->i_blocks*(block_size/512)) && i < 12; i++) {
        if (!fillInFilesFromBlock(folder, i, 0))
            break;
    }
}

bool ext2FileSystemManager::fillInFilesFromBlock(ext2Folder *folder, unsigned int block_num, unsigned long long offsetOfStruct) {
    while (true) {
        long long offset = bootBlockLocation+(block_size * (block_num))+24+offsetOfStruct; //the "+24" allows us to skip unneeded data

        stringstream ss;

        InodeIn.inode = getStreamData(4,offset, *input, "Inode Number", true);
        InodeIn.rec_len = getStreamData(2,offset+4, *input, "Directory Length", true);
        InodeIn.name_len  = getStreamData(1,offset+6, *input, "Name Length", true);
        InodeIn.file_type = getStreamData(1,offset+7, *input, "File Type", true);

        for(int i=0; i<(InodeIn.name_len); i++)
            ss << (char)input->get();

        string temp;
        ss >> temp;
        InodeIn.name = temp;

        cout << "The name of the file is " << InodeIn.name << endl;

        offsetOfStruct += InodeIn.rec_len; //increment running count
        if (InodeIn.file_type !=0 && offsetOfStruct < block_size) {
            this->addEntry(folder, InodeIn);
        } else
            break;
    }

    if (offsetOfStruct >= block_size) {
        return false; //we are not done getting files
    }
    return true;
}

void ext2FileSystemManager::addEntry(ext2Folder *folder, Inode_info InodeIn) {

}

bool ext2FileSystemManager::isDirInTable() const {

    return false;
}

bool ext2FileSystemManager::isFileInTable() const {
    return false;
}

void ext2FileSystemManager::getInodeTableData(unsigned int InodeNumber) {
    if (tab != NULL) {
        delete tab;
        tab = NULL;
    }
    tab = new InodeTable;

    unsigned int block_group= (InodeNumber -1) /superBlock->getInodesPerGroup();
    //cout << block_group << endl;
    unsigned int local_inode_index= (InodeNumber-1) % superBlock->getInodesPerGroup();
    //cout << local_inode_index << endl;

    long long group_size = superBlock->getBlockSize()*superBlock->getBlocksPerGroup();
    long long offset = iNodeTableAddress + ((block_group)*group_size) + (local_inode_index * sizeof(tab));

    tab->i_mode = getStreamData(2,offset, *input, "Mode", true);
    tab->i_uid = getStreamData(2,offset+2, *input, "Uid", false);
    tab->i_size = getStreamData(4,offset+4, *input, "Size", true);
    tab->i_atime = getStreamData(4,offset+8, *input, "Atime", false);
    tab->i_ctime = getStreamData(4,offset+12, *input, "Ctime", false);
    tab->i_mtime = getStreamData(4,offset+16, *input, "Mtime", false);
    tab->i_dtime = getStreamData(4,offset+20, *input, "Dtime", false);
    tab->i_gid = getStreamData(2,offset+24, *input, "Gid", false);
    tab->i_links_count = getStreamData(2,offset+26, *input, "Links Count", true);
    tab->i_blocks = getStreamData(4,offset+28, *input, "Blocks", true);
    tab->i_flags = getStreamData(4,offset+32, *input, "Flags", false);
    tab->i_osd1 = getStreamData(4,offset+36, *input, "Osd1", false);
    int add =0;
    for (int i=0; i<15; i++){
        tab->i_block[i] = getStreamData(4,offset+40+add, *input, "", true);
        add+=4;
    }
    tab->i_generation = getStreamData(4,offset+100, *input, "Generation", false);
    tab->i_file_acl = getStreamData(4,offset+104, *input, "File ACL", false);
    tab->i_dir_acl = getStreamData(4,offset+108, *input, "Dir ACL", true);
    tab->i_faddr = getStreamData(4,offset+112, *input, "Faddr", false);

#warning this probably doesnt work
    tab->i_osd2[12] = getCharFromStream(12,offset+116, *input);

}

