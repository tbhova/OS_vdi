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
#include <ios>
#include <QMessageBox>
#include <qmessagebox.h>
#include <cstring>

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
    InputData = new QVector <char>;
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

    block_bitmap_address = bootBlockLocation + block_size*groupDescriptors->getBlockBitmap(0);
    inode_bitmap_address = bootBlockLocation + block_size*groupDescriptors->getInodeBitmap(0);
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

    //updateBitmap(inode_bitmap_address,21,input,false,true);
}

void VdiFile::closeAndReset() {
    input.close();
    DataBlockBitmap->clear();
    blockBitmap->clear();
    inodesBitmap->clear();
}

void VdiFile::fillDataBlockBitmap(QVector<unsigned char>* DataBlockBitmap, unsigned int block_bitmap_address,unsigned int inode_bitmap_address,fstream& input) {
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
    OffsetForProgressBar = sourceFile->getInodeTable()->i_size/50;
    FileSizeForProgressBar = sourceFile->getInodeTable()->i_size;
    if(FileSizeForProgressBar > 3000)
        QMessageBox::information(NULL, tr("Download Time"), tr("Your download may take a little time. Please click 'OK' to start download and please do not click off of this window until complete..."));

    loadLocalFile(sourceFile->getInodeTable(),sourceFile->getInodeTable()->i_size,0, input, OutputFileIntoLocalFS);
    OutputFileIntoLocalFS.close();
}


void VdiFile::loadLocalFile(InodeTable* InodeTab, unsigned int size, unsigned int inodeIndexNum, fstream& input , ofstream& localFile){

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
            emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
            //cout << "We are out of the loop right now" << endl;
            }
        else{

            for (int i=0; i<size; i++){
                //input.seekg(offset+i);
                localFile << (char)getStreamData(1,offset +i,input,"",false);
                //input.clear();
                }
            size =0;
            emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
            }
        cout << "We are in the direct with Inode Index num" << inodeIndexNum << endl;
    }
    else if (inodeIndexNum ==12)    {
        qDebug() << "Entering Singly Indirect..." << endl;
        size = singlyIndirectPointersValues(InodeTab->i_block[inodeIndexNum],input,localFile,size);
        //emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
        }
    else if (inodeIndexNum ==13)    {
        qDebug() << "Entering Doubly Indirect..." << endl;
        size = doublyIndirectPointersValues(InodeTab->i_block[inodeIndexNum],input,localFile,size);
        //emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
        }
    else if (inodeIndexNum ==14)    {
        qDebug() << "Entering Triply Indirect..." << endl;
        size = triplyIndirectPointersValues(InodeTab->i_block[inodeIndexNum],input,localFile,size);
        //emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
        }

    inodeIndexNum++;
    cout << "The size before the if was" <<size<< endl;
    if(size!=0){
        cout << "We got into here" << endl;
        loadLocalFile(InodeTab, size, inodeIndexNum, input, localFile);
    }
    else{
        emit progressUpdate (100);
        return;
        }

}

unsigned long long VdiFile::singlyIndirectPointersValues(unsigned long long blockNumberOfSinglyIndirect, fstream& input, ofstream& localFile, unsigned long long size){
    unsigned int offset = bootBlockLocation+(block_size * (blockNumberOfSinglyIndirect));
    // we know that each entry is 4 bytes long due to what we used for inode reading in ext2
    //cout << "The offset we got for you" << hex << offset << endl;
    //cout << "Before the for loop" << endl;
    //cout << getStreamData(4,offset,input,"",false) << endl;
    unsigned long long lastSize = size;
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

        emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
 }





    cout << "The size we returned was" << size<< endl;
    return size;
    // end of singly indirect
}

unsigned long long VdiFile::doublyIndirectPointersValues(unsigned long long blockNumberOfDoublyIndirect, fstream& input, ofstream& localFile, unsigned long long size){
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
        //emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
        doublyIterator++;
        qDebug() << "Doubly Indirect..." << endl;
        if(size == 0) break;

        }
    return size;


// end of doublyindirect
}

unsigned long long VdiFile::triplyIndirectPointersValues(unsigned long long blockNumberOfTriplyIndirect, fstream& input, ofstream& localFile, unsigned long long size){
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
        //emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
        triplyIterator++;
        qDebug() << "Triply Indirect..." << endl;
        if(size == 0) break;

        }
    return size;


// end of doublyindirect
}


void VdiFile::transferToVDI(CSCI5806::ext2Folder *VDIFolder, QFileInfo *sourceFile) {
    qDebug() << "destination folder on VDI " << VDIFolder->getName() << " source file local FS " << sourceFile->absoluteFilePath();
    //open file,check to make sure its open
    string sourceDir = sourceFile->absoluteFilePath().toStdString();
    InputFileIntoVdiFS.open(sourceDir.c_str(), ios::in|ios::binary|ios::ate);

    //Find size of the file
    cout << "The size of that file was " << InputFileIntoVdiFS.tellg() << endl;

    trialToDekstop.open("/Users/Andy/Desktop/trial.txt", ios::in|ios::out|ios::binary);
    if (InputFileIntoVdiFS.is_open()){
        cout << hex << "File is open and is ready to go" << endl;
        }

    InputFileIntoVdiFS.seekg(1);
    InputFileIntoVdiFS >> noskipws;
    string input;

    for(int i=0; i< 32; i++){
        InputData->push_back((char) InputFileIntoVdiFS.get());
        //cout << "The value at " << i << " is: " << InputData->at(i) << " with a total size of: " << InputData->size() << " ..." << InputData->length() << endl;
        input=input+InputData->at(i);


    }
    cout << "We got here after vector" << input << endl;
    cout << "The length of the string is" << InputData->length() << endl;


    trialToDekstop.seekp(32|ios::beg);
    trialToDekstop.write(input.c_str(),InputData->length());

    trialToDekstop.close();
    InputFileIntoVdiFS.close();
    cout << "The size of that file was " << InputFileIntoVdiFS.tellg() << endl;

    //get folder table
    InodeTable *tab = VDIFolder->getInodeTable();

    //get folder inode number
    unsigned int inodeNum = VDIFolder->getInodeNumber();

    if (this->fsManager == NULL) {
        return;
    }
    //get folder inode offset in disk
    long long folderInodeOffset = fsManager->getInodeOffset(inodeNum);
    DirectoryEntry newEntry;
    //create and write directory entry to the directory
    this->writeDirectoryEntry(newEntry, tab, inodeNum, folderInodeOffset, sourceFile);

    //write file inode to table (all block pointers 0 (NULL))
    InodeTable newTab;
    this->writeNewInode(newEntry, newTab, InputFileIntoVdiFS.tellg());

    //allocate block pointers

    //write data to blocks

    //close the file you are writing from
    InputFileIntoVdiFS.close();
}

void VdiFile::updateBitmap (long long BitmapLocation, unsigned int inodeOrBlockNumber, fstream& VDIFile, bool setToUsed, bool isInodeBitmap){
    inodeOrBlockNumber--; //this converts us into 0 based indexing on the inodeNumber
    long long inodeByteNumber = inodeOrBlockNumber /8;
    long long localBitNumberInByte = inodeOrBlockNumber%8;
    vector <bool> *localVec;
    if(isInodeBitmap)
        localVec =inodesBitmap;
    else
        localVec = blockBitmap;

    QString byteString;
    for(int i=0; i<8; i++){
        if(i == localBitNumberInByte && setToUsed )
            byteString.append('1');
        else if(i == localBitNumberInByte && !setToUsed )
#warning what is this condition?
            byteString.append('0');
        else if(inodesBitmap->at(inodeByteNumber*8+i) == true )
            byteString.append('1');
        else
            byteString.append('0');
    }

    bool ok;
    string byteInput;
    byteInput = byteInput + (char) byteString.toUShort(&ok,2);
    cout << "Size "<< sizeof(byteInput) << " with value of " << byteInput << endl;

    /* Now we need to open up the file and and replace this new byte with what was orginally there */
    long long location = BitmapLocation +inodeByteNumber;
    VDIFile.seekp (location|ios::beg);
    VDIFile.write(byteInput.c_str(),1);

}

void VdiFile::addBytesToFile (QVector <unsigned char> * toLoadVector, long long offset,fstream& VDIFile ){
    string localAddToFile;
    for(int i=0; i<toLoadVector->length(); i++){
        localAddToFile = localAddToFile + (char)toLoadVector->at(i);
    }

    VDIFile.seekp (offset|ios::beg);
    VDIFile.write(localAddToFile.c_str(),localAddToFile.length());

}


void VdiFile::writeDirectoryEntry(DirectoryEntry &newEntry, InodeTable *tab, unsigned int inodeNum, long long folderInodeOffset, QFileInfo *sourceFile) {
    newEntry.name = sourceFile->fileName().toStdString();
    newEntry.name_len = newEntry.name.size();
    newEntry.rec_len = newEntry.name.size() + sizeof(newEntry.file_type) + sizeof(newEntry.inode) + sizeof(newEntry.name_len) + sizeof(newEntry.rec_len);
    unsigned int calculatedSize = newEntry.rec_len;
    if (newEntry.rec_len % 4 != 0) {
        newEntry.rec_len += 4 - (newEntry.rec_len % 4); //align to 4 byte blocks
    }
    newEntry.inode = this->findFreeBitmap(inodesBitmap);
    newEntry.file_type = 1; //we only support writing files
    //update inodeBitmap
    this->updateBitmap(inode_bitmap_address, newEntry.inode, input, false, true);

    //find space in directory blocks to write DirectoryEntry
    unsigned int destinationBlockIndex = tab->i_blocks/(block_size/512);
    qDebug() << "destBlock " << destinationBlockIndex;

    //get last block used in directory
    unsigned int destBlock = fsManager->getBlockNumAtIndex(tab, destinationBlockIndex);

    //find out if we have enough space to place directory entry
    unsigned short usedInBlock = 0;
    for (unsigned int i = 24; i < block_size;) { //start at 24 to get
        unsigned short rec_len = getStreamData(2, fsManager->getBlockOffset(destBlock) + i+4, input, "Directory Length", true);
        unsigned char file_type = getStreamData(1, fsManager->getBlockOffset(destBlock) +i+7, input, "File Type", true);

        if (file_type == 0 || file_type > 7 || rec_len < 1) { //if invalid file type/invalid directory entry
            break;
        }
        usedInBlock = i;
        i+= rec_len;
    }

    if((block_size - usedInBlock) < newEntry.rec_len) {
        //there is not enough size in the current block
        usedInBlock = 0;
#warning ToDo allocate new block and update directory pointers, and update destBlock
        qDebug() << "very bad, this code needs finished";
        return;
    }

    //build QVector<unsigned char> for directory entry
    QVector<unsigned char> dirEntry;
    addBytesToVector(dirEntry, newEntry.inode, sizeof(newEntry.inode));
    addBytesToVector(dirEntry, newEntry.rec_len, sizeof(newEntry.rec_len));
    addBytesToVector(dirEntry, newEntry.name_len, sizeof(newEntry.name_len));
    addBytesToVector(dirEntry, newEntry.file_type, sizeof(newEntry.file_type));
    for (int i = 0; i < newEntry.name.size(); i++) {
        addBytesToVector(dirEntry, newEntry.name.at(i), sizeof(char));
    }
    //pad with \0 bytes
    for (int i = 0; i < newEntry.rec_len-calculatedSize; i++) {
        addBytesToVector(dirEntry, 0, 1);
    }
    cout << "dir Entry byte array: ";
    for (int j = 0; j < dirEntry.size(); j++) {
        cout << dirEntry.at(j) << " ";
    }
    cout << endl;

    //find offset to write at from destBlock and usedInBlock
    long long writeOffset = this->fsManager->getBlockOffset(destBlock) + usedInBlock;

    //write QVector at offset
    this->addBytesToFile(&dirEntry, writeOffset, input);
}

void VdiFile::addBytesToVector(QVector<unsigned char> &vec, unsigned long long value, unsigned char bytes) {
    for (int i = 0; i < bytes; i++) {
        unsigned long long maskedValue = value & (0xFF << (sizeof(unsigned char)*i));
        unsigned char append = (unsigned char)(maskedValue >> (sizeof(unsigned char)*i));
        vec.push_back(append);
    }
}

unsigned int VdiFile::findFreeBitmap(vector<bool> *vec) {
    for (unsigned int i = 2; i < vec->size(); i++) { //skip badBlocks and root iNodes
        if (!vec->at(i)) {
            vec->at(i) = true; //inode is now used
            return i+1; //1 based iNode indexing
        }
    }
    cout << "error, no free iNodes";
    return inodesBitmap->size();
}

void VdiFile::writeNewInode(DirectoryEntry &newEntry, InodeTable newTab, unsigned int fileSize) {
    long long writeOffset = this->fsManager->getInodeOffset(newEntry.inode);

    newTab.i_mode = 0;
    newTab.i_uid = 100;
    newTab.i_size = 0;
    newTab.i_atime = 0;
    newTab.i_ctime = 0;
    newTab.i_mtime = 0;
    newTab.i_dtime = 0;
    newTab.i_gid = 0;
    newTab.i_links_count = 0;
    newTab.i_blocks = 0;
    newTab.i_flags = 0;
    newTab.i_osd1 = 0;
    newTab.i_generation = 0;
    newTab.i_file_acl = 0;
    newTab.i_dir_acl = 0;
    newTab.i_faddr = 0;
    for (int i = 0; i < 12; i ++) {
        newTab.i_osd2[i] = 0;
    }
    this->allocateBlockPointers(newTab.i_block, fileSize);
}

void VdiFile::allocateBlockPointers(unsigned int i_block[], unsigned int fileSize) {
    unsigned int blocks = fileSize/block_size;
    for (int i = 0; i < 12; i++) {
        if ((i+1) < blocks) {
            i_block[i] = findFreeBitmap(blockBitmap);
        } else {
            i_block[i] = 0;
        }
    }
}
