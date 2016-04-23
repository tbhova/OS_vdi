#include "ext2filesystemmanager.h"
#include "vdifunctions.h"
#include <QObject>
#include <sstream>
#include <iostream>
#include <QDebug>

using namespace std;
using namespace CSCI5806;

ext2FileSystemManager::ext2FileSystemManager(ifstream *file, long long inodeAddress, ext2SuperBlock *super, unsigned int bootBlock, unsigned int blockSize)
{
    input = file;
    iNodeTableAddress = inodeAddress;
    superBlock = super;
    bootBlockLocation = bootBlock;
    block_size = super->getBlockSize();
    //get root info
    qDebug() << "begin construct fsManager";
    getInodeTableData(2); //root is inode 2
    root = new ext2Folder(tab, 2, QObject::tr("/"));
    //root->setPath("/");
    addFilesAndFolders(root);
}

ext2FileSystemManager::~ext2FileSystemManager() {
    //delete tab;
}

ext2Folder* ext2FileSystemManager::getFolderAtPath(QString path) {
    ext2Folder *current = root;
    path.remove(0, 1); //remove root
    int firstSlash = path.indexOf("/");
    firstSlash = (firstSlash == -1) ? path.size() : firstSlash;
    while (path.size() > 0) {
        qDebug() << "path: " << path << "1st / " << firstSlash;
        QString folderName = path.left(firstSlash);
        qDebug() << "explore to path folder name = " << folderName;
        foreach (ext2Folder *f, *(current->getFolders())) {
            if (f->getName() == folderName) {
                current = f;
                break;
            }
        }
        path.remove(0, firstSlash+1); //remove folder
        firstSlash = path.indexOf("/");
        firstSlash = (firstSlash == -1) ? path.size() : firstSlash;
    }
    return current;
}

bool ext2FileSystemManager::exploreToPath(QString path) {
    ext2Folder *current = this->getFolderAtPath(path);


    //determine whether we should return early
    foreach (ext2Folder *f, *(current->getFolders())) {
        if (f->getFolders()->size() != 0 || f->getFiles()->size() != 0) {
            //we have already explored this folder, no need to reexplore
            qDebug() << "folder " << f->getName() << " already explored.";
            return false;
        }
    }

    qDebug() << "end traverse current = " << current->getName();
    //call addfilesFolders for all folders in the current folder
    foreach (ext2Folder *f, *(current->getFolders())) {
        qDebug() << "addFilesAndFolders" <<(f->getName());
        this->getInodeTableData(f->getInodeNumber());
        this->addFilesAndFolders(f);
    }

    return true;
}

void ext2FileSystemManager::addFilesAndFolders(ext2Folder *folder) {
    tempTab = folder->getInodeTable();
    qDebug() << "tempTab->i_blocks " << tempTab->i_blocks;
    for (int i = 0; i < (tempTab->i_blocks*(block_size/512)) && i < 12; i++) {
        qDebug() << "add files i_block " << tempTab->i_block[i];
        if (!fillInFilesFromBlock(folder, tempTab->i_block[i], 0))
            break;
    }
    for (int i = 0; i < 14; i++)
        cout << dec << "dir i_block " << i << " " << tempTab->i_block[i] << endl;
}

bool ext2FileSystemManager::fillInFilesFromBlock(ext2Folder *folder, unsigned int block_num, unsigned long long offsetOfStruct) {
    while (true) {
        long long offset = bootBlockLocation+(block_size * (block_num))+24+offsetOfStruct; //the "+24" allows us to skip unneeded data
        cout << hex << "offset " << offset << endl;
        cout << bootBlockLocation << endl;
        cout << block_size << endl;
        cout << block_num << endl;
        cout << offsetOfStruct << endl;
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

        cout << dec << "The name of the file is " << InodeIn.name << endl;

        offsetOfStruct += InodeIn.rec_len; //increment running count
        if (InodeIn.file_type !=0) {
            this->addEntry(folder, InodeIn);
        } else
            break;
        if (offsetOfStruct > block_size)
            break;
    }

    if (offsetOfStruct >= block_size) {
        return false; //we are not done getting files
    }
    return true;
}

void ext2FileSystemManager::addEntry(ext2Folder *folder, const Inode_info &InodeIn) {
    cout << "add entry name  = " << InodeIn.name << endl;
    cout << "inodeNumber = " << InodeIn.inode << endl;
    if (InodeIn.name == "..")
        return;
    switch (InodeIn.file_type) {
    case (1) : //file
        //populate folder inode data in tab
        this->getInodeTableData(InodeIn.inode);
        //add new file to current folder
        folder->getFiles()->push_back(new ext2File(tab, InodeIn.inode, QObject::tr(InodeIn.name.c_str())));
        break;

    case(2): //folder
        //populate new folder inode data in tab
        this->getInodeTableData(InodeIn.inode);
        //add new folder to our current folder
        folder->getFolders()->push_back(new ext2Folder(tab, InodeIn.inode, QObject::tr(InodeIn.name.c_str())));
        break;
    }
}

/*
bool ext2FileSystemManager::isDirInTable() const {

    return false;
}

bool ext2FileSystemManager::isFileInTable() const {
    return false;
}*/

void ext2FileSystemManager::getInodeTableData(unsigned int InodeNumber) {
    /*if (tab != NULL) {
        delete tab;
        tab = NULL;
    }*/
    //tab = new InodeTable;

    cout << "InodeNumber = " << InodeNumber << endl;
    cout << "block_size" << block_size << endl;
    cout << "super block size" << superBlock->getBlockSize() << endl;
    cout << "blocksupergroup" << superBlock->getBlocksPerGroup() << endl;

    unsigned int block_group= (InodeNumber -1) /superBlock->getInodesPerGroup();
    cout << "block_group " << block_group << endl;
    unsigned int local_inode_index= (InodeNumber-1) % superBlock->getInodesPerGroup();
    cout << "local_inode_index " << local_inode_index << endl;

    long long group_size = superBlock->getBlockSize()*superBlock->getBlocksPerGroup();
    long long offset = iNodeTableAddress + ((block_group)*group_size) + (local_inode_index * sizeof(tab));
    cout << "group_size " << group_size << endl;
    cout << "offset " << offset << endl;
    cout << "sizeof tab " << sizeof(tab) << endl;

    tab.i_mode = getStreamData(2,offset, *input, "Mode", true);
    tab.i_uid = getStreamData(2,offset+2, *input, "Uid", false);
    tab.i_size = getStreamData(4,offset+4, *input, "Size", true);
    tab.i_atime = getStreamData(4,offset+8, *input, "Atime", false);
    tab.i_ctime = getStreamData(4,offset+12, *input, "Ctime", false);
    tab.i_mtime = getStreamData(4,offset+16, *input, "Mtime", false);
    tab.i_dtime = getStreamData(4,offset+20, *input, "Dtime", false);
    tab.i_gid = getStreamData(2,offset+24, *input, "Gid", false);
    tab.i_links_count = getStreamData(2,offset+26, *input, "Links Count", true);
    tab.i_blocks = getStreamData(4,offset+28, *input, "Blocks", true);
    tab.i_flags = getStreamData(4,offset+32, *input, "Flags", false);
    tab.i_osd1 = getStreamData(4,offset+36, *input, "Osd1", false);
    int add =0;
    for (int i=0; i<15; i++){
        tab.i_block[i] = getStreamData(4,offset+40+add, *input, "i_block", true);
        add+=4;
    }
    tab.i_generation = getStreamData(4,offset+100, *input, "Generation", false);
    tab.i_file_acl = getStreamData(4,offset+104, *input, "File ACL", false);
    tab.i_dir_acl = getStreamData(4,offset+108, *input, "Dir ACL", true);
    tab.i_faddr = getStreamData(4,offset+112, *input, "Faddr", false);

#warning this probably doesnt work
    tab.i_osd2[12] = getCharFromStream(12,offset+116, *input);

}

ext2Folder* ext2FileSystemManager::getRoot() const {
    return root;
}
