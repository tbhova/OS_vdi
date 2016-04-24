#include "vdifile.h"
#include <QMessageBox>
#include <QFileDialog>
#include <string>
#include <iostream>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog.h>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <qmessagebox.h>
#include <iostream>
#include <String>
#include <iostream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <cmath>
#include <sstream>
#include <string>
#include "vdifunctions.h"
#include <QDebug>

using namespace std;
using namespace CSCI5806;

VdiFile::VdiFile(QObject *parent) : QObject(parent)
{
    vdi = new QFile();
    map = NULL;
    mbr = NULL;
    superBlock = NULL;
    groupDescriptors = NULL;
    DataBlockBitmap = new QVector<unsigned char>;
    blockBitmap = new vector<bool>;
    inodesBitmap = new vector<bool>;
}

VdiFile::~VdiFile() {
#warning delete everything allocated with new

    delete vdi;
    if (map != NULL)
        delete map;
    if (mbr != NULL)
        delete mbr;
    if (superBlock != NULL)
        delete superBlock;
    if (groupDescriptors != NULL)
        delete groupDescriptors;
    if (DataBlockBitmap != NULL)
        delete DataBlockBitmap;
    if (blockBitmap != NULL)
        delete blockBitmap;
    if (inodesBitmap != NULL)
        delete inodesBitmap;
    if (fsManager != NULL)
        delete fsManager;
}


void VdiFile::selectVdiPrompt() {
    //Open File
    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Please open a .vdi file"), "./", ".VDI File (*.*);;All Files (*.*)");

    QMessageBox::information(NULL,tr("FileNameOut"),fileName);
    emit(this->vdiFileSelected(fileName));
    this->openFile(fileName);
}

void VdiFile::openFile(QString fileName) {
    static bool initialized = false; //says we already have a file open
    if (initialized)
        this->closeAndReset();

    qDebug() << QObject::tr("vdi file ") << fileName;

    if (fileName.isEmpty() || fileName.isNull() || !(fileName.toLower()).contains(".vdi")) {
        QMessageBox::information(NULL,tr("Error"), tr("Please choose a valid vdi file."));
        return;
    }


    input.open(fileName.toStdString().c_str(), ios::in);



    if(!input.is_open()) {
        cout << "File not open!" << endl;
        return;
    }
    input >> noskipws;

    initialized = true;

    header.headerSize = getStreamData((int)4,(long long)72, input);
    header.imageType = getStreamData(4,76, input);
    header.offsetBlocksSize= getStreamData(4,340,input);
    header.offsetDataSize = getStreamData(4, 344, input);
    header.sectorSize = getStreamData(4, 360, input );
    header.diskSize = getStreamData (8, 368, input);
    header.blockSize = getStreamData (4, 376, input);
    header.blocksInHDD = getStreamData(4, 384, input );
    header.blocksAllocatedSize = getStreamData (4, 388, input);
    header.UUIDofVDI = getStreamData (16, 392, input);
    header.UUIDofSNAP = getStreamData(16, 408, input);



    map = new VdiMap (this, header.offsetBlocksSize, header.offsetBlocksSize + 4 *header.blocksInHDD, input);
    mbr = new mbrData (this, header.offsetDataSize, input);


    unsigned int mbrDataTable0= mbr->getMbrDataTable(0);
    //nothing in the boot block, because this is not bootable
    bootBlockLocation = header.offsetDataSize+mbrDataTable0*512;
    superBlockLocation = bootBlockLocation +1024;

    superBlock = new ext2SuperBlock (this, superBlockLocation, input);

    unsigned int group_count = superBlock->getGroupCount();
    block_size = superBlock->getBlockSize();
    unsigned int inodes_per_group = superBlock->getInodesPerGroup();

    cout << "andy block_size" << block_size << endl;
    cout << "andy super block size" << superBlock->getBlockSize() << endl;
    cout << "andy blocksupergroup" << superBlock->getBlocksPerGroup() << endl;
    group_size = block_size*superBlock->getBlocksPerGroup();
    cout << "The total size of this block group is (bytes): " << group_size << endl;


    //cout << superBlock->getBlockSize() << "    " << superBlock->getGroupDescriptorBlockNumber()<< endl;
    unsigned int gdtLocation = bootBlockLocation + (block_size*superBlock->getGroupDescriptorBlockNumber());

    cout << "GDT Location(dec): " << hex << setw(2) << setfill('0') << gdtLocation << endl;

    cout << "Group count= " << dec <<group_count << endl;
    cout << "Block Size = " << dec << block_size << endl;



    //groupDescriptors->push_back(new ext2GroupDescriptor (this, group_count, gdtLocation, input));
    groupDescriptors = new ext2GroupDescriptor (this, group_count, gdtLocation, input);

    unsigned int block_bitmap_address = bootBlockLocation + block_size*groupDescriptors->getBlockBitmap(1);
    unsigned int inode_bitmap_address = bootBlockLocation + block_size*groupDescriptors->getInodeBitmap(1);
    cout << "Hello:: Inode Bitmap Location: " << hex << setw(2) << setfill('0') << inode_bitmap_address << endl;
    unsigned int inode_table_address = bootBlockLocation + block_size*groupDescriptors->getInodeTable(1);
    cout << "Hello:: Inode Table Location: " << hex << setw(2) << setfill('0') << inode_table_address << endl;

    //cout << hex << setw(2) << setfill('0') << inode_table_address <<" "<<endl << endl<< endl;
    //unsigned int block_bitmap = getStreamData(4,inode_bitmap_address, input);



    //DataBlockBitmap = new QVector <unsigned char>;
    // fillDataBlockBitmap(DataBlockBitmap, block_bitmap_address, inode_bitmap_address, input);
    //cout << "This is the adress of block bitmap" << hex<< block_bitmap_address << endl;

    addBitsFromStreamData(blockBitmap, block_size*8, block_bitmap_address, input);
    addBitsFromStreamData(inodesBitmap, block_size*8, inode_bitmap_address, input);
    //cout << dec <<blockBitmap->size() << " " << sizeof(*blockBitmap) << endl;
    //cout << blockBitmap->max_size() << endl;
    cout << "Bit reading/ converting complete" << endl;

    fsManager = new ext2FileSystemManager(&input, inode_table_address, superBlock, bootBlockLocation, block_size);
    emit FSManagerConstructed(fsManager);

    //The second value of the inode table gives you the root directory. Note that these are 1 based indices

    //for(int i=5;i<15; i++){
    getInodeTableData(inode_table_address,2, input);
    //int toBlock = tab.i_block[0];
    //cout << "This is the toBlock" << toBlock << endl;
    //getDataBlock(36,input);
    //getInodeTableData(hello23,0,input);
    //getInodeTableData(inode_table_address,11, input);
    // getInodeTableData(inode_table_address,12, input);
    //getInodeTableData(inode_table_address,13, input);
    //getInodeTableData(inode_table_address,14, input);



    //ACL is 0 if it is a file


    //cout << "The boot block location is " << hex << bootBlockLocation << endl;
    //cout << "THe block size is" << dec <<block_size << endl;
    //cout << "The superBlock location is " << hex << superBlockLocation <<endl;
    cout << "The address of first block thing is:" << hex << bootBlockLocation+(block_size * (tab.i_block[0])) << endl;
    InodeInfo = new QVector <Inode_info>;
    fillRootDir(tab.i_block[0],0,input);


    //print out names
    for (int i=0; i<InodeInfo->length(); i++)
           cout << "Title "<< i << " : " << InodeInfo->at(i).name<< endl;

    cout << "Num for next inode" << hex << bootBlockLocation +(block_size *InodeInfo->at(0).inode) << endl;
    getInodeTableData(inode_table_address,InodeInfo->at(0).inode,input);
}

void VdiFile::closeAndReset() {
    input.close();
    DataBlockBitmap->clear();
    blockBitmap->clear();
    inodesBitmap->clear();
}


void VdiFile::fillDataBlockBitmap(QVector<unsigned char>* DataBlockBitmap, unsigned int block_bitmap_address,unsigned int inode_bitmap_address,ifstream& input) {
    for (unsigned int i=block_bitmap_address; i <inode_bitmap_address; i++){
         DataBlockBitmap->push_back(getCharFromStream(1,i,input));
    }   
}

void VdiFile::getInodeTableData(long long InitialOffset, int InodeNumber, ifstream &file){

    unsigned int block_group= (InodeNumber -1) /superBlock->getInodesPerGroup();
    cout << "andy block_group " << block_group << endl;
    unsigned int local_inode_index= (InodeNumber-1) % superBlock->getInodesPerGroup();
    cout << "andy local_inode_index " << local_inode_index << endl;


    long long offset = InitialOffset + ((block_group)*group_size) + (local_inode_index * sizeof(tab));
    cout << "andy group_size " << group_size << endl;
    cout << "andy offset " << offset << endl;
    cout << "andy sizeof tab " << sizeof(tab) << endl;

    tab.i_mode = getStreamData(2,offset, file, "Mode", true);
    tab.i_uid = getStreamData(2,offset+2, file, "Uid", false);
    tab.i_size = getStreamData(4,offset+4, file, "Size", true);
    tab.i_atime = getStreamData(4,offset+8, file, "Atime", false);
    tab.i_ctime = getStreamData(4,offset+12, file, "Ctime", false);
    tab.i_mtime = getStreamData(4,offset+16, file, "Mtime", false);
    tab.i_dtime = getStreamData(4,offset+20, file, "Dtime", false);
    tab.i_gid = getStreamData(2,offset+24, file, "Gid", false);
    tab.i_links_count = getStreamData(2,offset+26, file, "Links Count", true);
    tab.i_blocks = getStreamData(4,offset+28, file, "Blocks", true);
    tab.i_flags = getStreamData(4,offset+32, file, "Flags", false);
    tab.i_osd1 = getStreamData(4,offset+36, file, "Osd1", false);
    int add =0;
    for (int i=0; i<15; i++){
        tab.i_block[i] = getStreamData(4,offset+40+add, file, "", true);
        add+=4;
    }
    tab.i_generation = getStreamData(4,offset+100, file, "Generation", false);
    tab.i_file_acl = getStreamData(4,offset+104, file, "File ACL", false);
    tab.i_dir_acl = getStreamData(4,offset+108, file, "Dir ACL", true);
    tab.i_faddr = getStreamData(4,offset+112, file, "Faddr", false);
    tab.i_osd2[12] = getCharFromStream(12,offset+116, file);

}

void VdiFile::getDataBlock(long long BlockNumber, ifstream &file){

    long long offset = bootBlockLocation + BlockNumber * block_size ;

   // cout << "SuperBlock Location" << hex<< superBlockLocation << endl;
   // cout << hex << offset << endl;

   // cout << endl << "I am in the new one now" << endl;

    tab.i_mode = getStreamData(2,offset, file, "Mode", false);
    tab.i_uid = getStreamData(2,offset+2, file, "Uid", false);
    tab.i_size = getStreamData(4,offset+4, file, "Size", true);
    tab.i_atime = getStreamData(4,offset+8, file, "Atime", false);
    tab.i_ctime = getStreamData(4,offset+12, file, "Ctime", false);
    tab.i_mtime = getStreamData(4,offset+16, file, "Mtime", false);
    tab.i_dtime = getStreamData(4,offset+20, file, "Dtime", false);
    tab.i_gid = getStreamData(2,offset+24, file, "Gid", false);
    tab.i_links_count = getStreamData(2,offset+26, file, "Links Count", true);
    tab.i_blocks = getStreamData(4,offset+28, file, "Blocks", true);
    tab.i_flags = getStreamData(4,offset+32, file, "Flags", false);
    tab.i_osd1 = getStreamData(4,offset+36, file, "Osd1", false);
    int add =0;
    for (int i=0; i<15; i++){
        tab.i_block[i] = getStreamData(4,offset+40+add, file, "", true);
        add+=4;
    }
    tab.i_generation = getStreamData(4,offset+100, file, "Generation", false);
    tab.i_file_acl = getStreamData(4,offset+104, file, "File ACL", false);
    tab.i_dir_acl = getStreamData(4,offset+108, file, "Dir ACL", true);
    tab.i_faddr = getStreamData(4,offset+112, file, "Faddr", false);
    tab.i_osd2[12] = getCharFromStream(12,offset+116, file);

}

void VdiFile::fillRootDir( long long block_num,long long offsetOfStruct,ifstream &file){

    long long offset = bootBlockLocation+(block_size * (block_num))+24+offsetOfStruct; //the "+24" allows us to skip unneeded data
    cout << "andy offset " << offset << endl;
    cout << bootBlockLocation << endl;
    cout << block_size << endl;
    cout << block_num << endl;
    cout << offsetOfStruct << endl;
    stringstream ss;

    InodeIn.inode = getStreamData(4,offset, file, "Inode Number", true);
    InodeIn.rec_len = getStreamData(2,offset+4, file, "Directory Length", true);
    InodeIn.name_len  = getStreamData(1,offset+6, file, "Name Length", true);
    InodeIn.file_type = getStreamData(1,offset+7, file, "File Type", true);

    for(int i=0; i<(InodeIn.name_len); i++)
        ss << (char)file.get();

    string temp;
    ss >> temp;
    InodeIn.name = temp;

    cout << "The name of the file is " << InodeIn.name << endl;

    if(InodeIn.file_type !=0){
            InodeInfo->push_back(InodeIn);
            fillRootDir(block_num,offsetOfStruct+InodeIn.rec_len,file);
        }

}
