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
    map = NULL;
    mbr = NULL;
    superBlock = NULL;
    groupDescriptors = NULL;
    fsManager = NULL;
    DataBlockBitmap = new QVector<unsigned char>;
    blockBitmap = new vector<bool>;
    inodesBitmap = new vector<bool>;
    SinglyIndirectPointers = new QVector <unsigned int>;
    DoublyIndirectPointers = new QVector <unsigned int>;
    TriplyIndirectPointers = new QVector <unsigned int>;
}

VdiFile::~VdiFile() {
#warning delete everything allocated with new
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

    input.open(fileName.toStdString().c_str(), ios::in | ios::out | ios::binary); //read and write to the vdi, ignore special char, all binary

    if(!input.is_open()) {
        cout << "File not open!" << endl;
        return;
    }
    input >> noskipws;

    initialized = true;

    header.headerSize = getStreamData((int)4,(long long)72, input,"",true,false,false);
    header.imageType = getStreamData(4,76, input,"",true,false,false);
    header.offsetBlocksSize= getStreamData(4,340,input,"",true,false,false);
    header.offsetDataSize = getStreamData(4, 344, input,"",true,false,false);
    header.sectorSize = getStreamData(4, 360, input ,"",true,false,false);
    header.diskSize = getStreamData (8, 368, input,"",true,false,false);
    header.blockSize = getStreamData (4, 376, input,"",true,false,false);
    header.blocksInHDD = getStreamData(4, 384, input ,"",true,false,false);
    header.blocksAllocatedSize = getStreamData (4, 388, input,"",true,false,false);
    header.UUIDofVDI = getStreamData (16, 392, input,"",true,false,false);
    header.UUIDofSNAP = getStreamData(16, 408, input,"",true,false,false);

    globalHeader.offsetDataSize = header.offsetDataSize;
    globalHeader.blockSize = header.blockSize;
    globalHeader.blocksInHDD = header.blocksInHDD;

    map = new VdiMap (this, header.offsetBlocksSize, header.offsetBlocksSize + 4 *header.blocksInHDD, input);
    globalMap = map;
    mbr = new mbrData (this, header.offsetDataSize, input);


    unsigned int mbrDataTable0= mbr->getMbrDataTable(0);
    //nothing in the boot block, because this is not bootable
    bootBlockLocation = header.offsetDataSize+mbrDataTable0*512;
    superBlockLocation = bootBlockLocation +1024;

    superBlock = new ext2SuperBlock (this, superBlockLocation, input);

    unsigned int group_count = superBlock->getGroupCount();
    block_size = superBlock->getBlockSize();

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

    unsigned int block_bitmap_address = bootBlockLocation + block_size*groupDescriptors->getBlockBitmap(0);
    unsigned int inode_bitmap_address = bootBlockLocation + block_size*groupDescriptors->getInodeBitmap(0);
    cout << "Hello:: Inode Bitmap Location: " << hex << setw(2) << setfill('0') << inode_bitmap_address << endl;
    unsigned int inode_table_address = bootBlockLocation + block_size*groupDescriptors->getInodeTable(0);
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

    fsManager = new ext2FileSystemManager(&input, groupDescriptors, superBlock, bootBlockLocation);
    emit FSManagerConstructed(fsManager);
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

void VdiFile::transferToLocalFS(CSCI5806::ext2File *sourceFile, QDir *destDir) {
    qDebug() << "source file " << sourceFile->getName() << " destination directory " << destDir->absolutePath();
    sourceFile->getInodeTable()->i_block[0];
    string directory = destDir->absolutePath().toStdString();
    string absoluteDir = sourceFile->getName().toStdString();
    OutputFileIntoLocalFS.open((directory +"/" + absoluteDir).c_str() , ios::out|ios::binary);
    if(OutputFileIntoLocalFS.is_open()){
       cout << "File is open for writing..." << endl;
       }
    else {
        cout << "error unable to open output file" << endl;
        return;
        }
    //qDebug() << "The size we have to work with is" <<  sourceFile->getInodeTable()->i_size << "  and the block num is " << sourceFile->getInodeTable()->i_block[0] << endl;
    //cout << "Location of first indirect block " << sourceFile->getInodeTable()->i_block[12] << endl;
    loadLocalFile(sourceFile->getInodeTable(),sourceFile->getInodeTable()->i_size,0, input, OutputFileIntoLocalFS);
    OutputFileIntoLocalFS.close();
/*
    //QDir myDirectory("");
    // myDirectory.setCurrent;


    // USE THIS TO REPLACE THE THINGS IN OUR PATH!!
    //mystring.replace("/","\\\\");
    QFile localFile("C:/Users/Andy/Desktop/sf-book.txt");
    if (!localFile.open(QIODevice::WriteOnly)) {
      std::cerr << "Cannot open file for writing: "
                << qPrintable(localFile.errorString()) << std::endl;
      return;
    }

    QTextStream out(&localFile);
    //out << "Thomas M. Disch: " << 334 << endl;

    */
}

void VdiFile::transferToVDI(CSCI5806::ext2Folder *VDIFolder, QFileInfo *sourceFile) {
    qDebug() << "destination folder on VDI " << VDIFolder->getName() << " source file local FS " << sourceFile->absoluteFilePath();

    emit progressUpdate(100);

    //get folder table
    InodeTable *tab = VDIFolder->getInodeTable();

    //get folder inode number
    unsigned int inodeNum = VDIFolder->getInodeNumber();

    if (this->fsManager == NULL) {
        return;
    }
    //get folder inode offset in disk
    long long folderInodeOffset = fsManager->getInodeOffset(inodeNum);

    //write file inode to table (all block pointers 0 (NULL))

    //allocate direct block pointers
}

void VdiFile::loadLocalFile(InodeTable* InodeTab, unsigned int size, unsigned int inodeIndexNum, ifstream& input , ofstream& localFile){
    cout << "The size of this field is " << size << " bytes" << endl;
    cout << "Inode index num" << inodeIndexNum << endl;
    if (inodeIndexNum <12){
        long long block_num = InodeTab->i_block[inodeIndexNum];
        unsigned long long offset = bootBlockLocation+(block_size * (block_num));
        //cout << "Our current offset is " << offset << endl;
        //cout << "Size "<< size << "  block size " << block_size << " differ" << (size-block_size) << endl;
        if(size >block_size){
            for(int i=0; i<block_size; i++){
                //input.seekg(offset+i);
                localFile << (char)getStreamData(1,offset +i,input,"",false);
                //input.clear();
                //cout << "Looping in here: " << i << endl;
                }
            size=size-block_size;
            //cout << "We are out of the loop right now" << endl;
            }
        else{

            for (int i=0; i<size; i++){
                //input.seekg(offset+i);
                localFile << (char)getStreamData(1,offset +i,input,"",false);
                //input.clear();
                }
            size =0;
            }
        cout << "We got in the direct with Inode Index num" << inodeIndexNum << endl;
    }
    else if (inodeIndexNum ==12)    {
        qDebug() << "Entering Singly Indirect..." << endl;
        size = singlyIndirectPointersValues(InodeTab->i_block[inodeIndexNum],input,localFile,size);
        }
    else if (inodeIndexNum ==13)    {
        qDebug() << "Entering Doubly Indirect..." << endl;
        size = doublyIndirectPointersValues(InodeTab->i_block[inodeIndexNum],input,localFile,size);
        }
    else if (inodeIndexNum ==14)    {
        qDebug() << "Entering Triply Indirect..." << endl;
        size = triplyIndirectPointersValues(InodeTab->i_block[inodeIndexNum],input,localFile,size);
        }

    inodeIndexNum++;
    cout << "The size before the if was" <<size<< endl;
    if(size!=0){
        cout << "We got into here" << endl;
        loadLocalFile(InodeTab, size, inodeIndexNum, input, localFile);
    }
    else
        return;
 //qDebug() << "End of File Writing" << endl;

}
unsigned long long VdiFile::singlyIndirectPointersValues(unsigned long long blockNumberOfSinglyIndirect, ifstream& input, ofstream& localFile, unsigned long long size){
    unsigned int offset = bootBlockLocation+(block_size * (blockNumberOfSinglyIndirect));
    // we know that each entry is 4 bytes long due to what we used for inode reading in ext2
    //cout << "The offset we got for you" << hex << offset << endl;
    //cout << "Before the for loop" << endl;
    //cout << getStreamData(4,offset,input,"",false) << endl;
    SinglyIndirectPointers->clear();
    for(int i=0; i<block_size; i=i+4){ //4 is the number of bytes in each entry
        SinglyIndirectPointers->push_back(getStreamData(4,offset+i,input,"",false));
        //cout << SinglyIndirectPointers->back() << endl;
        if(SinglyIndirectPointers->back() == 0)
            break;
    }
    cout << "The size of the Vector is now: " << SinglyIndirectPointers->size() << endl;
    unsigned int iterator=0;
    while(iterator < SinglyIndirectPointers->size() && size>0 && SinglyIndirectPointers->at(iterator) !=0){
        offset = bootBlockLocation+(block_size * (SinglyIndirectPointers->at(iterator)));
        if(size > block_size){
            //cout << "We got into the if " << endl;
            //cout << "Our Current size is "<< size << endl;
            for(int i=0; i<block_size; i++){
                //input.seekg(offset+i);
                localFile << (char)getStreamData(1,offset +i,input,"",false);
                //input.clear();
                //cout << "Looping in here: " << i << endl;
            }
            size= size-block_size;
            //cout << "We are out of the loop right now" << endl;
        } else {
            //cout << "We got into the else " << endl;
            //cout << "Our Current size is "<< size << endl;
            for (int i=0; i<size; i++){
                //input.seekg(offset+i);
                localFile << (char)getStreamData(1,offset +i,input,"",false);
                //input.clear();
            }
            size =0;
        }

        iterator ++;
        //cout << "Singly Indirect..." << endl;
        if(size == 0) break;
 }





    cout << "The size we returned was" << size<< endl;
    return size;
    // end of singly indirect
}


unsigned long long VdiFile::doublyIndirectPointersValues(unsigned long long blockNumberOfDoublyIndirect, ifstream& input, ofstream& localFile, unsigned long long size){
    unsigned long long offsetStartDoublyIndirect = bootBlockLocation+(block_size * (blockNumberOfDoublyIndirect));
    // we know that each entry is 4 bytes long due to what we used for inode reading in ext
    DoublyIndirectPointers->clear();
    for(int i=0; i<block_size; i=i+4){ //4 is the number of bytes in each entry
        DoublyIndirectPointers->push_back(getStreamData(4,offsetStartDoublyIndirect+i,input,"",false));
        //cout << DoublyIndirectPointers->back() << endl;
        if(DoublyIndirectPointers->back() == 0)
            break;
    }
    //cout << "Done with for loop" << endl;
    unsigned int doublyIterator=0;
    while(doublyIterator < DoublyIndirectPointers->size() && DoublyIndirectPointers->at(doublyIterator) !=0 && size>0){
        //offset = bootBlockLocation+(block_size * (DoublyIndirectPointers->at(doublyIterator)));
        size = singlyIndirectPointersValues(DoublyIndirectPointers->at(doublyIterator),input,localFile,size);
        doublyIterator++;
        qDebug() << "Doubly Indirect..." << endl;
        if(size == 0) break;

        }
    return size;


// end of doublyindirect
}

unsigned long long VdiFile::triplyIndirectPointersValues(unsigned long long blockNumberOfTriplyIndirect, ifstream& input, ofstream& localFile, unsigned long long size){
    unsigned long long offsetStartTriplyIndirect = bootBlockLocation+(block_size * (blockNumberOfTriplyIndirect));
    // we know that each entry is 4 bytes long due to what we used for inode reading in ext
    TriplyIndirectPointers->clear();
    for(int i=0; i<block_size; i=i+4){ //4 is the number of bytes in each entry
        TriplyIndirectPointers->push_back(getStreamData(4,offsetStartTriplyIndirect+i,input,"",false));
        //cout << TriplyIndirectPointers->back() << endl;
        if(TriplyIndirectPointers->back() == 0)
            break;
    }
    //cout << "Done with for loop" << endl;
    unsigned int triplyIterator=0;
    while(triplyIterator < TriplyIndirectPointers->size() && TriplyIndirectPointers->at(triplyIterator) !=0 && size>0){
        //offset = bootBlockLocation+(block_size * (DoublyIndirectPointers->at(doublyIterator)));
        size = doublyIndirectPointersValues(TriplyIndirectPointers->at(triplyIterator),input,localFile,size);
        triplyIterator++;
        qDebug() << "Triply Indirect..." << endl;
        if(size == 0) break;

        }
    return size;


// end of doublyindirect
}
