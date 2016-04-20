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
#include <cstring>
#include "globalfunctions.h"

using namespace std;

//convert byte data to integers
unsigned long long convertEndian(unsigned char C[], long long size, bool littleEndian){

    unsigned char temp[size];
    for (long long a=0; a<size; a++){
        if (!littleEndian)
            temp[size-1-a]= C[a];
        else
            temp[a] = C[a];
       // cout << hex << setw(2) << setfill('0') << (int)temp[b] << " ";
    }
    //cout << endl;

    unsigned long long total=0;
    stringstream charNums;
    for(long long i=0; i<size;i++){
        //cout << hex << setw(2) << setfill('0') << (int)temp[i] << " "<<endl;
        charNums << hex << setw(2) << setfill('0') << (int)temp[i];
       // cout << "in charNums: " << charNums.str() << endl;
    }
    QString chars = QString::fromStdString(charNums.str());
    //cout << chars.toStdString() <<endl;
    bool ok;
    total=chars.toULongLong(&ok,16);

    //cout << total <<endl;
    if(!ok)
        cout << "Unable to convert String"<< endl;

    return total;

}

//get data from Stream
//long long getStreamData(int size, long long seek_to, std::ifstream &input, std::string name = "", bool output = true);
unsigned long long getStreamData(int size, long long seek_to, ifstream &input, string name, bool output, bool littleEndian){


    unsigned char data[size];
    input.clear();
    input.seekg(seek_to);

    for (int i=0; i<size;i++){
        input >> data[i];
        if (output){
            //cout << endl<<endl;
            cout << hex << setw(2) << setfill('0') << (int)data[i] << " ";
        }
    }
    unsigned long long size_of_part;
    size_of_part=convertEndian(data, size, littleEndian);
    if (output) {
        cout <<"Size of "<< name << ":" << dec << size_of_part << endl;
        cout << endl;
    }
    return size_of_part;

}

void addBitsFromStreamData(vector<bool> *bits, int numBits, long long seek_to, ifstream &input){
    unsigned long long temp;

    input.clear();
    input.seekg(seek_to);
    int seeks = 0;
    for(int i = 0; i < numBits; i++) {
        //if we have used all the bits in the long long, lets get another
        if(i % (sizeof(unsigned long long)*8) == 0) {

            temp = getStreamData(sizeof(unsigned long long), seek_to+(sizeof(unsigned long long))*seeks, input, (string)"  ", false, true);
            cout << endl << "address: " << hex << seek_to+(sizeof(unsigned long long))*seeks << endl;
            cout << hex << temp << endl;
            seeks++;
        }
        bits->push_back((temp & ((unsigned long long)1 << (sizeof(unsigned long long)*8-1-(i % (sizeof(unsigned long long)*8))))) > 0);
        //cout << hex << ((unsigned long long)1 << (sizeof(unsigned long long)*8-1-(i % (sizeof(unsigned long long)*8)))) << ' ';
        //if (bits->back())
        //    cout << '1';
        //else cout << '0';
    }
    //cout << endl << endl;
}

//get data from Stream
unsigned char getCharFromStream(int size, long long seek_to, ifstream &input){

    unsigned char data[size];
    input.clear();
    input.seekg(seek_to);
    for (int i=0; i<size;i++){
        input >> data[i];
        //cout << hex << setw(2) << setfill('0') << (int)data[i] << " ";
    }
    //cout <<"" <<endl;
    return data[size];

}

//get data from Stream
char getSignedCharFromStream(int size, long long seek_to, ifstream &input){

    char data[size];
    input.clear();
    input.seekg(seek_to);
    for (int i=0; i<size;i++){
        input >> data[i];
        //cout << hex << setw(2) << setfill('0') << (int)data[i] << " ";
    }
    //cout <<"" <<endl;
    return data[size];

}



VdiFile::VdiFile(QObject *parent) : QObject(parent)
{
    vdi = new QFile();
    map = NULL;
    mbr = NULL;
    superBlock = NULL;
   // groupDescriptors = new QVector<ext2GroupDescriptor*>;
}

VdiFile::~VdiFile() {
#warning these can be null
#warning delete everything allocated with new
    delete vdi;
    delete map;
}


void VdiFile::selectVdiPrompt() {
    //Open File
    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Please open a .vdi file"), "C://", ".VDI File (*.*);;All Files (*.*)");

    QMessageBox::information(NULL,tr("FileNameOut"),fileName);
    emit(this->vdiFileSelected(fileName));
    this->openFile(fileName);
}

void VdiFile::openFile(QString fileName) {
    static bool initialized = false; //says we already have a file open
    if (initialized)
        this->closeAndReset();

    string fileString = fileName.toStdString();

    cout << fileString << endl;
    char *fileChar = new char[fileString.length() + 1];

    strcpy(fileChar, fileString.c_str());

    input.open(fileChar, ios::in);


    if(!input.is_open())
        cout << "File not open!" << endl;
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
    blockBitmap = new vector<bool>;
    inodesBitmap = new vector<bool>;
    addBitsFromStreamData(blockBitmap, block_size*8, block_bitmap_address, input);
    addBitsFromStreamData(inodesBitmap, block_size*8, inode_bitmap_address, input);
    //cout << dec <<blockBitmap->size() << " " << sizeof(*blockBitmap) << endl;
    //cout << blockBitmap->max_size() << endl;
    cout << "Bit reading/ converting complete" << endl;


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

}


void VdiFile:: fillDataBlockBitmap(QVector<unsigned char>* DataBlockBitmap, unsigned int block_bitmap_address,unsigned int inode_bitmap_address,ifstream& input){
    for (unsigned int i=block_bitmap_address; i++; i <inode_bitmap_address){
         DataBlockBitmap->push_back(getCharFromStream(1,i,input));
    }
}


void VdiFile::getInodeTableData(long long InitialOffset, int InodeNumber, ifstream &file){

    unsigned int block_group= (InodeNumber -1) /superBlock->getInodesPerGroup();
    //cout << block_group << endl;
    unsigned int local_inode_index= (InodeNumber-1) % superBlock->getInodesPerGroup();
    //cout << local_inode_index << endl;


    long long offset = InitialOffset + ((block_group)*group_size) + (local_inode_index * sizeof(tab));

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
