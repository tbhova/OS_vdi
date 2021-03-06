#include "ext2filesystemmanager.h"
#include "vdifunctions.h"
#include <QObject>
#include <sstream>
#include <iostream>
#include <QDebug>

using namespace std;
using namespace CSCI5806;

ext2FileSystemManager::ext2FileSystemManager(fstream *file, ext2GroupDescriptor *group, ext2SuperBlock *super, unsigned int bootBlock)
{
    input = file;
    groupDescriptor = group;
    //cout << "iNodeTableAddress " << inodeAddress << endl;
    //cout << "bootBlockAddress " << bootBlockAddress << endl;
    superBlock = super;
    bootBlockAddress = bootBlock;
    block_size = super->getBlockSize();
    //get root info
    //qDebug() << "begin construct fsManager";
    getInodeTableData(2); //root is inode 2
    root = new ext2Folder(tab, 2, QObject::tr("/"));
    addFilesAndFolders(root);
}

ext2FileSystemManager::~ext2FileSystemManager() {
    delete root;
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
    //qDebug() << "end traverse current = " << current->getName();
    bool entryAdded = false;
    //call addfilesFolders for all folders in the current folder
    foreach (ext2Folder *f, *(current->getFolders())) {
        qDebug() << "addFilesAndFolders at folder " <<(f->getName());

        //original vector sizes
        int origFiles = f->getFiles()->size();
        int origFolders = f->getFolders()->size();
        this->addFilesAndFolders(f);

        //if we added entries, return true that entries were added
        if (origFiles != f->getFiles()->size() || origFolders != f->getFolders()->size())
            entryAdded = true;
    }

    return entryAdded;
}

void ext2FileSystemManager::addFilesAndFolders(ext2Folder *folder) {
    //cout << "add files and folders - folderName = " << folder->getName().toStdString() << endl;
    const InodeTable *tempTab = folder->getInodeTable();
    //cout << "tempTab->i_blocks " << tempTab->i_blocks << endl;
    for (unsigned int i = 0; (i < (tempTab->i_blocks)/(block_size/512)) && (i < 12); i++) {
        //qDebug() << "i " << i << " blocks " << (tempTab->i_blocks)/(block_size/512);
        //qDebug() << "add files i_block " << tempTab->i_block[i];
        if (tempTab->i_block[i] == 0)
            break;
        fillInFilesFromBlock(folder, tempTab->i_block[i], 24);
    }
    /*for (int i = 0; i < 15; i++)
        cout << dec << "dir i_block " << i << " " << tempTab->i_block[i] << endl;*/
}

void ext2FileSystemManager::fillInFilesFromBlock(ext2Folder *folder, unsigned int block_num, unsigned long long offsetOfStruct) {
    while (true) {
        long long offset = this->getBlockOffset(block_num) + offsetOfStruct;
        //cout << "fill in files from block - folderName = " << folder->getName().toStdString() << endl;
        //cout << hex << "offset " << offset << endl;
        //cout << dec << bootBlockAddress << endl;
        //cout << block_size << endl;
        //cout << block_num << endl;
        //cout << offsetOfStruct << endl;
        stringstream ss;

        InodeIn.inode = (unsigned int)getStreamData(4, offset, *input, "Inode Number", true);
        InodeIn.rec_len = getStreamData(2, offset+4, *input, "Directory Length", true);
        InodeIn.name_len  = getStreamData(1, offset+6, *input, "Name Length", true);
        InodeIn.file_type = getStreamData(1, offset+7, *input, "File Type", true);
        for(int i=0; i<(InodeIn.name_len); i++)
            ss << (char)input->get();
        input->clear();

        InodeIn.name = ss.str();

        cout << dec << "The file is " << InodeIn.name << endl;

        offsetOfStruct += InodeIn.rec_len; //increment running count
        if (InodeIn.file_type == 1 || InodeIn.file_type == 2) {
            this->addEntry(folder);
        } else if (InodeIn.file_type == 0 || InodeIn.file_type > 7 || InodeIn.rec_len < 1) //if invalid file type/invalid directory entry
            break;
        if (offsetOfStruct >= block_size)
            break;
    }
}

void ext2FileSystemManager::addEntry(ext2Folder *folder) {
    //cout << "add entry - folderName = " << folder->getName().toStdString() << endl;
    //cout << "add entry name  = " << InodeIn.name << endl;
    if (InodeIn.name == ".." || InodeIn.name == ".")
        return;
    ext2File *newFile;
    ext2Folder *newFolder;
    bool exists = false;
    switch (InodeIn.file_type) {
    case (1) : //file
        //populate folder inode data in tab
        this->getInodeTableData(InodeIn.inode);
        //add new file to current folder
        newFile = new ext2File(tab, (InodeIn.inode), QObject::tr(InodeIn.name.c_str()));
        foreach (ext2File *f, *folder->getFiles()) {
            if (*f == *newFile) {
                exists = true;
                //qDebug() << "file already in folder";
                delete newFile; //delete this now since it won't be deleted by the destructor
                break;
            }
        }
        if (!exists) {
            folder->getFiles()->push_back(newFile);
            cout << "add file " << newFile->getName().toStdString() << " to folder " << folder->getName().toStdString() << endl;
        }
        break;
    case (2): //folder
        //populate new folder inode data in tab
        this->getInodeTableData(InodeIn.inode);
        //add new folder to our current folder
        newFolder = new ext2Folder(tab, InodeIn.inode, QObject::tr(InodeIn.name.c_str()));
        foreach (ext2Folder *f, *folder->getFolders()) {
            if (*f == *newFolder) {
                exists = true;
                //qDebug() << "folder already in folder";
                delete newFolder; //delete this now since it won't be deleted by the destructor
                break;
            }
        }
        if (!exists) {
            folder->getFolders()->append(newFolder);
            cout << "add folder " << newFolder->getName().toStdString() << " to folder " << folder->getName().toStdString() << endl;
        }
        break;
    }
}

void ext2FileSystemManager::getInodeTableData(unsigned int InodeNumber) {
    long long offset = this->getInodeOffset(InodeNumber);

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

    tab.i_osd2[12] = getCharFromStream(12,offset+116, *input);
    InodeIn.inode = InodeNumber; //for reasons I can't explain, this is needed because the program breaks otherwise
}

ext2Folder* ext2FileSystemManager::getRoot() const {
    return root;
}

long long ext2FileSystemManager::getInodeOffset(unsigned int InodeNumber) {
    /*cout << "InodeNumber = " << dec << InodeNumber << endl;
    cout << "block_size" << block_size << endl;
    cout << "super block size" << superBlock->getBlockSize() << endl;
    cout << "blocksupergroup" << superBlock->getBlocksPerGroup() << endl;*/

    unsigned int block_group= (InodeNumber -1) /superBlock->getInodesPerGroup();
    //cout << "block_group " << block_group << endl;
    unsigned int local_inode_index= (InodeNumber-1) % superBlock->getInodesPerGroup();
    //cout << "local_inode_index " << local_inode_index << endl;

    long long offset = bootBlockAddress + block_size*groupDescriptor->getInodeTable(block_group) + (local_inode_index * sizeof(tab));

    //cout << "offset " << hex << offset << endl;
    //cout << "sizeof tab " << dec << sizeof(tab) << endl;
    //cout << "bootBlockAddress " << bootBlockAddress << endl;

    return offset;
}

long long ext2FileSystemManager::getBlockOffset(unsigned int block_num) {
    return bootBlockAddress+(block_size * (block_num));
}

unsigned int ext2FileSystemManager::getBlocksPerIndirection(int indirection) {
    unsigned int size = 1;
    switch (indirection) {
    case (3) :
        size = block_size/sizeof(unsigned int);
    case (2) :
        size *= block_size/sizeof(unsigned int);
    case (1) :
        size *= block_size/sizeof(unsigned int);
        return size;
    case (0) :
        return 12;
    default:
        return -1;

    }
}

unsigned int ext2FileSystemManager::getBlockNumAtIndex(const InodeTable *tab, unsigned int index) {
    if (index < 12) {
        return tab->i_block[index];
    }

    unsigned int blocksPerInd = this->getBlocksPerIndirection(1); //singly indirect blocks
    index -= 11; //remove direct
    if (index <= blocksPerInd) {
        unsigned int block_num = tab->i_block[12];
        return getStreamData(4, getBlockOffset(block_num) + index*sizeof(unsigned int), *input, "block num");
    }

    index -= blocksPerInd; //remove singly indirect
    unsigned int blocksPerDoublyInd = this->getBlocksPerIndirection(1); //doubly indirect blocks
    if (index <= blocksPerDoublyInd) {
        unsigned int block_num = tab->i_block[13];
        block_num = getStreamData(4, getBlockOffset(block_num) + (index/blocksPerInd)*sizeof(unsigned int), *input, "double block num - singly pointer");
        return getStreamData(4, getBlockOffset(block_num) + (index % blocksPerInd) * sizeof(unsigned int), *input, "doubly block num - direct pointer");
    }

    index -= blocksPerInd; //remove doubly indirect
    unsigned int blocksPerTriplyInd = this->getBlocksPerIndirection(1); //triply indirect blocks
    if (index <= blocksPerTriplyInd) {
        unsigned int block_num = tab->i_block[14];
        block_num = getStreamData(4, getBlockOffset(block_num) + (index/blocksPerDoublyInd)*sizeof(unsigned int), *input, "triply block num - doubly pointer");
        index /= blocksPerDoublyInd;
        block_num = getStreamData(4, getBlockOffset(block_num) + (index / blocksPerInd) * sizeof(unsigned int), *input, "triply block num - singly pointer");
        return getStreamData(4, getBlockOffset(block_num) + (index % blocksPerInd) * sizeof(unsigned int), *input, "triply block num - direct pointer");
    }
    return 0;
}
