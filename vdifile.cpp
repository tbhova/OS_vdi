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
#include <QDateTime>
#include <algorithm>

using namespace std;
using namespace CSCI5806;

VdiFile::VdiFile(QObject *parent) : QObject(parent)
{
    map = NULL;
    mbr = NULL;
    superBlock = NULL;
    groupDescriptors = NULL;
    fsManager = NULL;
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
    if (blockBitmap != NULL)
        delete blockBitmap;
    if (inodesBitmap != NULL)
        delete inodesBitmap;
    if (fsManager != NULL)
        delete fsManager;

    delete blockBitmap;
    delete inodesBitmap;
    delete SinglyIndirectPointers;
    delete DoublyIndirectPointers;
    delete TriplyIndirectPointers;
    delete InputData;
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

    //cout << hex << setw(2) << setfill('0') << inode_table_address <<" "<<endl << endl<< endl;
    //unsigned int block_bitmap = getStreamData(4,inode_bitmap_address, input);



    //DataBlockBitmap = new QVector <unsigned char>;
    // fillDataBlockBitmap(DataBlockBitmap, block_bitmap_address, inode_bitmap_address, input);
    //cout << "This is the adress of block bitmap" << hex<< block_bitmap_address << endl;

    fsManager = new ext2FileSystemManager(&input, groupDescriptors, superBlock, bootBlockLocation);

    blockBitmap->push_back(true); //superBlock
    for (unsigned int i = 0; i < group_count; i++) {
        addBitsFromStreamData(blockBitmap, block_size*8, fsManager->getBlockOffset(groupDescriptors->getBlockBitmap(i)), input);
        addBitsFromStreamData(inodesBitmap, block_size*8, fsManager->getBlockOffset(groupDescriptors->getInodeBitmap(i)), input);
    }
    cout << "Bit reading/ converting complete" << endl;
    cout << "size of block bitmap " << dec << blockBitmap->size() << endl;
    cout << "size of inode bitmap " << dec << inodesBitmap->size() << endl;
    cout << "free blocks " << std::count(blockBitmap->begin(), blockBitmap->end(), false);

    emit FSManagerConstructed(fsManager);
}

void VdiFile::closeAndReset() {
    input.close();
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
            for(unsigned int i=0; i<block_size; i++){
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

            for (unsigned int i=0; i<size; i++){
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

    //unsigned long long lastSize = size;

    SinglyIndirectPointers->clear();
    for(unsigned int i=0; i<block_size; i=i+4){ //4 is the number of bytes in each entry
        SinglyIndirectPointers->push_back(getStreamData(4,offset+i,input,"",false));
        //cout << SinglyIndirectPointers->back() << endl;
        if(SinglyIndirectPointers->back() == 0)
            break;
    }
    cout << "The size of the Vector is now: " << SinglyIndirectPointers->size() << endl;
    int iterator=0;
    while(iterator < SinglyIndirectPointers->size() && size>0 && SinglyIndirectPointers->at(iterator) !=0){
        offset = bootBlockLocation+(block_size * (SinglyIndirectPointers->at(iterator)));
        if(size > block_size){
            //cout << "We got into the if " << endl;
            //cout << "Our Current size is "<< size << endl;
            for(unsigned int i=0; i<block_size; i++){
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
            for (unsigned int i=0; i<size; i++){
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
    for(unsigned int i=0; i<block_size; i=i+4){ //4 is the number of bytes in each entry
        DoublyIndirectPointers->push_back(getStreamData(4,offsetStartDoublyIndirect+i,input,"",false));
        //cout << DoublyIndirectPointers->back() << endl;
        if(DoublyIndirectPointers->back() == 0)
            break;
    }
    //cout << "Done with for loop" << endl;
    int doublyIterator=0;
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
    for(unsigned int i=0; i<block_size; i=i+4){ //4 is the number of bytes in each entry
        TriplyIndirectPointers->push_back(getStreamData(4,offsetStartTriplyIndirect+i,input,"",false));
        //cout << TriplyIndirectPointers->back() << endl;
        if(TriplyIndirectPointers->back() == 0)
            break;
    }
    //cout << "Done with for loop" << endl;
    int triplyIterator=0;
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

void VdiFile::transferToVDI(CSCI5806::ext2Folder *VDIFolder, QModelIndex *index, QFileInfo *sourceFile) {
    qDebug() << "destination folder on VDI " << VDIFolder->getName() << " source file local FS " << sourceFile->absoluteFilePath();
    //open file,check to make sure its open
    string sourceDir = sourceFile->absoluteFilePath().toStdString();
    InputFileIntoVdiFS.open(sourceDir.c_str(), ios::in|ios::binary|ios::ate);

    cout << "The size of that file was " << InputFileIntoVdiFS.tellg() << endl;

    if(InputFileIntoVdiFS.tellg() > 3000)
        QMessageBox::information(NULL, tr("Upload Time"), tr("Your upload may take a little time. Please click 'OK' to start download and please do not click off of this window until complete..."));

    //get folder table
    InodeTable *tab = VDIFolder->getInodeTable();
    cout << "Received table from VDI folder" << endl;

    //get folder inode number
    unsigned int inodeNum = VDIFolder->getInodeNumber();
    cout << "The inode number for VDIFolder -> getInodeNumber is " << inodeNum << " that tells us where the inode for the folder is" << endl;

    if (this->fsManager == NULL) {
        return;
    }
    //get folder inode offset in disk
    long long folderInodeOffset = fsManager->getInodeOffset(inodeNum);
    cout << " The inode offset for the folder is (fsmanager -> getInodeOffset(inodeNum): " << hex <<folderInodeOffset << endl;
    DirectoryEntry newEntry;
    //create and write directory entry to the directory
    this->writeDirectoryEntry(newEntry, tab, inodeNum, folderInodeOffset, sourceFile);

    //write file inode to table (all block pointers 0 (NULL))
    InodeTable newTab;
    this->writeNewInode(newEntry, newTab, InputFileIntoVdiFS.tellg(),input);

    //write data to blocks
    this->writeToVDIFS(&newTab,InputFileIntoVdiFS.tellg(),0,input,InputFileIntoVdiFS);
    //close the file you are writing from
    InputFileIntoVdiFS.close();

    if(VDIFolder->getName() != "/") {
        cout << "update not root" << endl;
        QModelIndex i = index->parent();
        emit updateFolder(i); // update parent folder (2 folders up from file written)
    } else {
        cout << "update root" << endl;
        fsManager->addFilesAndFolders(VDIFolder);
        emit this->updateRoot();
    }
}

void VdiFile::updateBitmap (unsigned int inodeOrBlockNumber, fstream& VDIFile, bool isInodeBitmap){
    inodeOrBlockNumber--; //this converts us into 0 based indexing on the inodeNumber

    unsigned int group = inodeOrBlockNumber/(block_size*8);
    unsigned int localNumber = inodeOrBlockNumber%(block_size*8);

    long long location;
    long long inodeByteNumber = localNumber/8;
    vector <bool> *localVec;
    if(isInodeBitmap){
        localVec =inodesBitmap;
        location = fsManager->getBlockOffset(groupDescriptors->getBlockBitmap(group));}
    else{
        localVec = blockBitmap;
        location = fsManager->getBlockOffset(groupDescriptors->getInodeBitmap(group));}
    QString byteString;
    for(int i=0; i<8; i++) {
        if(localVec->at(inodeByteNumber*8 + group*block_size*8  +i) == true)
            byteString.append('1');
        else
            byteString.append('0');
    }

    bool ok;
    string byteInput;
    byteInput = byteInput + (char) byteString.toUShort(&ok,2);
    //cout << "Size "<< sizeof(byteInput) << " with value of " << byteInput << endl;

    /* Now we need to open up the file and and replace this new byte with what was orginally there */
    location = location +inodeByteNumber;
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
    cout << "Name: " << newEntry.name << endl;
    newEntry.name_len = newEntry.name.size();
    cout << "Name Length: " << dec<<(int)newEntry.name_len << endl;
    newEntry.rec_len = newEntry.name.size() + sizeof(newEntry.file_type) + sizeof(newEntry.inode) + sizeof(newEntry.name_len) + sizeof(newEntry.rec_len);
    cout << "Record length: " << newEntry.rec_len << endl;
    unsigned int calculatedSize = newEntry.rec_len;
    if (newEntry.rec_len % 4 != 0) {
        newEntry.rec_len += 4 - (newEntry.rec_len % 4); //align to 4 byte blocks
    }
    cout << "New Record length: " << (int)newEntry.rec_len << endl;
    newEntry.inode = this->findFreeBitmap(inodesBitmap);
    cout << "NewEntry Inode: "<< hex << newEntry.inode <<endl;
    newEntry.file_type = 1; //we only support writing files
    //update inodeBitmap

    this->updateBitmap(newEntry.inode,input, true);

    //find space in directory blocks to write DirectoryEntry
    unsigned int destinationBlockIndex = tab->i_blocks/(block_size/512);
    cout << "Destination Block Index: " << destinationBlockIndex << endl;
    qDebug() << "destBlock " << destinationBlockIndex;

    //get last block used in directory
    unsigned int destBlock = fsManager->getBlockNumAtIndex(tab, destinationBlockIndex-1);
    cout << "Destination block" <<destBlock << endl; //should return 130041 for folder examples

    //find out if we have enough space to place directory entry
    unsigned short usedInBlock = 0;
    unsigned short startOfLastCurrent = 0;

    //unsigned short startOfLastCurrent = 0;

    unsigned short SizeOfLast =0;
    cout << "Block Offset: " <<fsManager->getBlockOffset(destBlock) << endl;
    for (unsigned int i = 24; i < block_size;) { //start at 24 to get beginning of directory entry
        unsigned short rec_len = getStreamData(2, fsManager->getBlockOffset(destBlock) + i+4, input, "Directory Length", true);
        unsigned char file_type = getStreamData(1, fsManager->getBlockOffset(destBlock) +i+7, input, "File Type", true);
        startOfLastCurrent = usedInBlock;
        usedInBlock = i;
        i+= rec_len;
        SizeOfLast =0;
        //if this is the last inode
        if (i == block_size) {
            //get all chars until 0 byte
            int j;
            for (j = 0; j < rec_len; j++) {
                char fileNameData = (char)getStreamData(1, fsManager->getBlockOffset(destBlock)+usedInBlock+8+j, input, "File Type", true);
                //if 0 byte
                if (fileNameData == 0) {
                    usedInBlock += j + 8;
                    if (usedInBlock % 4 != 0) {
                        usedInBlock += 4 - (usedInBlock%4);

                    }
                    break;
                }
            SizeOfLast++;
            }
            break;
        }
        if (file_type == 0 || file_type > 7 || rec_len < 1) { //if invalid file type/invalid directory entry
            break;
        }

    }
    cout << "Value used in block: " <<usedInBlock << endl;

    /* Change value of length for the directory entry directly before the one we add*/
    QVector <unsigned char> bytesToAddToFile;
    SizeOfLast= (((SizeOfLast-1)/4)+1)*4 +8;
    cout << "Size of last: " << SizeOfLast<< endl;
    cout << "Least significant for size: " << hex <<(SizeOfLast & 0xFF); // least significant
    cout << "Most significant for size: " << hex <<((SizeOfLast>> 8) & 0xFF);
    bytesToAddToFile.push_back((SizeOfLast & 0xFF));
    bytesToAddToFile.push_back(((SizeOfLast>> 8) & 0xFF));
    unsigned short beginningOfLastEntry = usedInBlock - SizeOfLast+4;
    addBytesToFile(&bytesToAddToFile,bootBlockLocation+destBlock*block_size+beginningOfLastEntry,input);


    if((block_size - usedInBlock) < newEntry.rec_len) {
        //there is not enough size in the current block
        usedInBlock = 0;
#warning ToDo allocate new block and update directory pointers, and update destBlock
        qDebug() << "very bad, this code needs finished";
        return;
    }

#warning make newEntry.rec_len go to 1024 like the existing entries do
#warning update old (last) record length (new length is startOfLastCurrent-usedInBlock)
    //build QVector<unsigned char> for directory entry
    QVector<unsigned char> dirEntry;
    addBytesToVector(dirEntry, newEntry.inode, sizeof(newEntry.inode));
    addBytesToVector(dirEntry, newEntry.rec_len, sizeof(newEntry.rec_len));
    addBytesToVector(dirEntry, newEntry.name_len, sizeof(newEntry.name_len));
    addBytesToVector(dirEntry, newEntry.file_type, sizeof(newEntry.file_type));
    for (unsigned int i = 0; i < newEntry.name.size(); i++) {
        addBytesToVector(dirEntry, newEntry.name.at(i), sizeof(char));
    }
    //pad with \0 bytes
    for (unsigned int i = 0; i < newEntry.rec_len-calculatedSize; i++) {
        addBytesToVector(dirEntry, 0, 1);
    }
    cout << "dir Entry byte array: ";
    for (int j = 0; j < dirEntry.size(); j++) {
        cout << dirEntry.at(j) << " ";
    }
    cout << endl;

    //find offset to write at from destBlock and usedInBlock
    long long writeOffset = this->fsManager->getBlockOffset(destBlock) + usedInBlock;
    cout << "dest block " << destBlock << endl;
    cout << "write offset " << writeOffset << endl;

    cout << "directory entry bytes: " << endl;
    foreach (unsigned char c, dirEntry) {
        if (isalpha(c))
            cout << hex << c << " ";
        else
            cout << hex << (int)c << " ";
    }
    cout << dec << endl;
    //write QVector at offset
    this->addBytesToFile(&dirEntry, writeOffset, input);


    /*Change the length of the entry we add to finish off the rest of the block*/
    QVector <unsigned char> bytesToAddToFile2;
    unsigned short newEntryDirLength = block_size-usedInBlock;
    cout << "Size of newEntryDirLength: " << newEntryDirLength << endl;
    cout << "Least significant for size: " << hex <<(newEntryDirLength& 0xFF); // least significant
    cout << "Most significant for size: " << hex <<((newEntryDirLength>> 8) & 0xFF);
    bytesToAddToFile2.push_back((newEntryDirLength & 0xFF));
    bytesToAddToFile2.push_back(((newEntryDirLength>> 8) & 0xFF));
    cout << "The offset we used to write: " << hex << bootBlockLocation+destBlock*block_size+usedInBlock+4 << endl;
    addBytesToFile(&bytesToAddToFile2,bootBlockLocation+destBlock*block_size+usedInBlock+4,input);
}

void VdiFile::addBytesToVector(QVector<unsigned char> &vec, unsigned long long value, unsigned char bytes) {
    for (int i = 0; i < bytes; i++) {
        unsigned char append = (unsigned char)((value >> (i*8)) & 0xFF);
        vec.push_back(append);
    }
}

unsigned int VdiFile::findFreeBitmap(vector<bool> *vec) {
    //next fit strategy
    static unsigned int i = 0;
    unsigned int start = i;
    for (; i < vec->size(); i++) { //skip badBlocks and root iNodes
        if (!vec->at(i)) {
            vec->at(i) = true; //inode is now used
            return i+1; //1 based iNode indexing
        }
    }
    for (unsigned int j = 0; j < start; j++) { //skip badBlocks and root iNodes
        if (!vec->at(j)) {
            vec->at(j) = true; //inode is now used
            return j+1; //1 based iNode indexing
        }
    }
    cout << "error, no free blocks or iNodes" << endl;
    return vec->size();
}

void VdiFile::writeNewInode(DirectoryEntry &newEntry, InodeTable &newTab, unsigned int fileSize, fstream &input) {
    newTab.i_mode = 33204; //-rw-rw-r--
    newTab.i_uid = 1000; //default user
    newTab.i_size = fileSize;
    unsigned int currentTime = QDateTime::currentDateTime().toTime_t(); //current time in
    newTab.i_atime = currentTime;
    newTab.i_ctime = currentTime;
    newTab.i_mtime = currentTime;
    newTab.i_dtime = 0;
    newTab.i_gid = 1000; //default group
    newTab.i_links_count = 1;
    unsigned int realBlocksNeeded = ((fileSize-1)/block_size)+1;
    newTab.i_blocks = realBlocksNeeded * (block_size/512);
    newTab.i_flags = 0;
    newTab.i_osd1 = 0;
    newTab.i_generation = 0;
    newTab.i_file_acl = 0;
    newTab.i_dir_acl = 0;
    newTab.i_faddr = 0;
    for (int i = 0; i < 12; i ++) {
        newTab.i_osd2[i] = 0;
    }
    this->allocateBlockPointers(newTab.i_block, fileSize,input);

    QVector<unsigned char> inodeByteVec;

    //convert InodeTable stuct to vector of bytes (including little endian conversion)
    this->buildInodeByteVector(inodeByteVec, newTab);

    cout << "inode byte vector " << endl;
    foreach (unsigned char c, inodeByteVec) {
        cout << hex << (int)c << " ";
    }
    cout << endl;

    cout << "write inode at offset " << hex << this->fsManager->getInodeOffset(newEntry.inode) << endl;
    //write byte vector to file
    addBytesToFile(&inodeByteVec, this->fsManager->getInodeOffset(newEntry.inode), input);
}

void VdiFile::buildInodeByteVector(QVector<unsigned char> &inodeByteVec, InodeTable &newTab) {
    addBytesToVector(inodeByteVec, newTab.i_mode, sizeof(newTab.i_mode));
    addBytesToVector(inodeByteVec, newTab.i_uid, sizeof(newTab.i_uid));
    addBytesToVector(inodeByteVec, newTab.i_size, sizeof(newTab.i_size));
    addBytesToVector(inodeByteVec, newTab.i_atime, sizeof(newTab.i_atime));
    addBytesToVector(inodeByteVec, newTab.i_ctime, sizeof(newTab.i_ctime));
    addBytesToVector(inodeByteVec, newTab.i_mtime, sizeof(newTab.i_mtime));
    addBytesToVector(inodeByteVec, newTab.i_dtime, sizeof(newTab.i_dtime));
    addBytesToVector(inodeByteVec, newTab.i_gid, sizeof(newTab.i_gid));
    addBytesToVector(inodeByteVec, newTab.i_links_count, sizeof(newTab.i_links_count));
    addBytesToVector(inodeByteVec, newTab.i_blocks, sizeof(newTab.i_blocks));
    addBytesToVector(inodeByteVec, newTab.i_flags, sizeof(newTab.i_flags));
    addBytesToVector(inodeByteVec, newTab.i_osd1, sizeof(newTab.i_osd1));

    for (int i = 0; i < 15; i++) {
        addBytesToVector(inodeByteVec, newTab.i_block[i], sizeof(newTab.i_block[i]));
    }
    addBytesToVector(inodeByteVec, newTab.i_generation, sizeof(newTab.i_generation));
    addBytesToVector(inodeByteVec, newTab.i_file_acl, sizeof(newTab.i_file_acl));
    addBytesToVector(inodeByteVec, newTab.i_dir_acl, sizeof(newTab.i_dir_acl));
    addBytesToVector(inodeByteVec, newTab.i_faddr, sizeof(newTab.i_faddr));
    for (int i = 0; i < 12; i++) {
        addBytesToVector(inodeByteVec, newTab.i_osd2[i], sizeof(newTab.i_osd2[i]));
    }
}

void VdiFile::allocateBlockPointers(unsigned int i_block[], unsigned int fileSize, fstream& input) {
    unsigned int blocksNeeded = ((fileSize-1)/block_size)+1;
    cout << " The number of blocks needed is: " << blocksNeeded << endl;
    for (unsigned int i = 0; i < 15; i++) { //up to 15 to set all non used blocks to 0
        if ((i+1) <= blocksNeeded && i <12) {
                i_block[i] = findFreeBitmap(blockBitmap);
                cout << "iBlock"<< i<< " equals " <<dec << i_block[i] << " for this instance"<< endl;
                updateBitmap (i_block[i], input, false);
             }
        else{
             i_block[i] = 0;
            }

        }

    if (blocksNeeded < 12) return;
    //singly indirect
    blocksNeeded -=12; //subtract the 12 blocks that are already allocated
    unsigned int spotsAvailableForAddresses = block_size/4; //number of pointers we can store in one block
    i_block[12] = findFreeBitmap(blockBitmap);
    updateBitmap (i_block[12], input,false);
    cout << "iBlock12 equals " <<dec << i_block[12] << " for this instance"<< endl;
    cout << "The number of blocks we still need are: " << blocksNeeded << endl;
    unsigned int blocksToAllocate = (blocksNeeded < spotsAvailableForAddresses)?blocksNeeded:spotsAvailableForAddresses;
    addBlockPointers(i_block[12],blocksToAllocate,input);

    blocksNeeded-=blocksToAllocate;

    if (blocksNeeded == 0) return;
    //doubly indirect
    cout << "doubly allocate blocks" << endl;
    cout << "block bitmap size = " << blockBitmap->size() << endl;
    i_block[13] = findFreeBitmap(blockBitmap);
    updateBitmap (i_block[13], input,false);
    cout << "iBlock13 equals " <<dec << i_block[13] << " for this instance"<< endl;
    cout << "iblock13 at offset " << hex <<fsManager->getBlockOffset(i_block[13]) << endl;
    cout << "The number of blocks we still need are: " << dec << blocksNeeded << endl;

    unsigned int doublyBlocksAvail = spotsAvailableForAddresses*spotsAvailableForAddresses;
    blocksToAllocate = (blocksNeeded < doublyBlocksAvail) ? blocksNeeded : doublyBlocksAvail;
    cout << "blocks to allocated " <<blocksToAllocate << endl;
    QVector<unsigned int> newAddresses;
    addBlockPointers(i_block[13],1 + (blocksToAllocate-1)/spotsAvailableForAddresses,input, &newAddresses);
    cout << "number new address " << newAddresses.size() << endl;
    foreach (unsigned int  i, newAddresses) {
        blocksToAllocate = (blocksNeeded < spotsAvailableForAddresses)?blocksNeeded:spotsAvailableForAddresses;
        cout << "blocks to allocated " <<blocksToAllocate << endl;
        cout << "block number i " << i << endl;
        cout << "block i offset offset " << fsManager->getBlockOffset(i) << endl;
        addBlockPointers(i, blocksToAllocate, input);
        blocksNeeded -= blocksToAllocate;
        cout << "The number of blocks we still need are: " << dec << blocksNeeded << endl;
    }
    newAddresses.clear();
    cout << "end doubly allocate blocks" << endl;

    //triply
    if (blocksNeeded == 0) return;
    cout << "triply allocate blocks" << endl;
    cout << "blocks still needed " << blocksNeeded << endl;
    i_block[14] = findFreeBitmap(blockBitmap);
    updateBitmap (i_block[14], input,false);
    cout << "iBlock14 equals " <<dec << i_block[14] << " for this instance"<< endl;
    cout << "iblock14 at offset " << hex <<fsManager->getBlockOffset(i_block[14]) << endl;
    cout << "The number of blocks we still need are: " << dec << blocksNeeded << endl;

    unsigned int triplyBlocksAvail = doublyBlocksAvail*spotsAvailableForAddresses;
    cout << " singly pointers avail " << spotsAvailableForAddresses << endl;
    cout << " doubly pointers avail " << doublyBlocksAvail << endl;
    cout << " triply pointers avail " << triplyBlocksAvail << endl;

    blocksToAllocate = (blocksNeeded < triplyBlocksAvail) ? blocksNeeded : triplyBlocksAvail;
    cout << "DOUBLY blocks to allocated " << blocksToAllocate << endl;

    addBlockPointers(i_block[14],1 + (blocksToAllocate-1)/doublyBlocksAvail, input, &newAddresses);
    cout << "number new address " << newAddresses.size() << endl;
    foreach (unsigned int  i, newAddresses) {
        QVector<unsigned int> newAddresses2;
        blocksToAllocate = (blocksNeeded < doublyBlocksAvail)?blocksNeeded:doublyBlocksAvail;
        cout << "SINGLY blocks to allocated " << blocksToAllocate << endl;
        cout << "block number i " << i << endl;
        cout << "block i offset offset " << fsManager->getBlockOffset(i) << endl;
        addBlockPointers(i, 1 + (blocksToAllocate-1)/spotsAvailableForAddresses, input, &newAddresses2);
        cout << "number new address2 " << newAddresses2.size() << endl;
        foreach (unsigned int  i, newAddresses2) {
            blocksToAllocate = (blocksNeeded < spotsAvailableForAddresses)?blocksNeeded:spotsAvailableForAddresses;
            cout << "DIRECT blocks to allocated " << blocksToAllocate << endl;
            cout << "block number i " << i << endl;
            cout << "block i offset offset " << fsManager->getBlockOffset(i) << endl;
            addBlockPointers(i, blocksToAllocate, input);
            blocksNeeded -= blocksToAllocate;
        }

        newAddresses2.clear();
    }
    newAddresses.clear();
}

void VdiFile::addBlockPointers(unsigned int block_num, unsigned int numberOfBlocksToAllocate, fstream& input, QVector<unsigned int> *blocks){
    QVector <unsigned char> bytesToAdd;

    for(unsigned int i=0; i< numberOfBlocksToAllocate; i++){
        //findFreeBlock
        unsigned int buffer = findFreeBitmap(blockBitmap);
        if (blocks != NULL)
        blocks->push_back(buffer);
        updateBitmap (buffer, input,false);

        //appendvalues to vector of char in order to write
        addBytesToVector(bytesToAdd, buffer, sizeof(buffer));
    }
    long long offset = fsManager->getBlockOffset(block_num);
    addBytesToFile(&bytesToAdd,offset,input);
}

void VdiFile::writeToVDIFS(InodeTable* InodeTab, unsigned int size, unsigned int inodeIndexNum, fstream& input , ifstream& localFile){

    cout << "The size of this field is " << size << " bytes" << endl;
    cout << "Inode index num" << inodeIndexNum << endl;
    while(size!=0){
      if(inodeIndexNum<12){
        long long block_num = InodeTab->i_block[inodeIndexNum];
        unsigned long long offset = fsManager->getBlockOffset(block_num);
        //cout << "The current offset is: " << hex << offset << endl;
        QVector <unsigned char> addToFile;
        localFile.seekg(inodeIndexNum*block_size);
        //cout << "Our current offset is " << offset << endl;
        //cout << "Size "<< size << "  block size " << block_size << " differ" << (size-block_size) << endl;
        if(size >block_size){
            for(unsigned int i=0; i<block_size; i++){
                unsigned char getVal = localFile.get();
                //cout << hex <<(int) getVal << endl;
                addToFile.push_back(getVal);
                //cout << "Looping in here: " << i << endl;
                }
            size=size-block_size;
            cout << "The current size in direct is: " << size << endl;
            //cout << "Size "<< size << "  block size " << block_size << " differ" << (size-block_size) << endl;
            emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
            //cout << "We are out of the loop right now" << endl;
            }
        else{

            for (unsigned int i=0; i<size; i++){
                unsigned char getVal = localFile.get();
                //cout << hex <<(int) getVal << endl;
                addToFile.push_back(getVal);
                }
            size =0;
            cout << "The current size in direct is: " << size << endl;
            emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
            }
        cout << "We are in the direct with Inode Index num" << inodeIndexNum << endl;
        addBytesToFile(&addToFile,offset,input);
        addToFile.clear();
      }
    else if (inodeIndexNum ==12)    {
        cout << "The block pointer to the singly indirect is: " << hex << InodeTab->i_block[inodeIndexNum] << endl;
        cout << "Entering Singly Indirect..." << endl;
        size = singlyIndirectPointersValuesWrite(InodeTab->i_block[inodeIndexNum],input,localFile,size);
        emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
        }
    else if (inodeIndexNum ==13)    {
        cout << "Entering Doubly Indirect..." << endl;
        size = doublyIndirectPointersValuesWrite(InodeTab->i_block[inodeIndexNum],input,localFile,size);
        emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
        }
    else if (inodeIndexNum ==14)    {
        cout << "Entering Triply Indirect..." << endl;
        size = triplyIndirectPointersValuesWrite(InodeTab->i_block[inodeIndexNum],input,localFile,size);
        emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
        }

    inodeIndexNum++;
    cout << "The size of iNodeNum" <<inodeIndexNum<< endl;
    if(inodeIndexNum>14)
        break;
    }

  emit progressUpdate (100);



}

unsigned long long VdiFile::singlyIndirectPointersValuesWrite(unsigned long long blockNumberOfSinglyIndirect, fstream& input, ifstream& localFile, unsigned long long size){
    unsigned int offset = bootBlockLocation+(block_size * (blockNumberOfSinglyIndirect));
    cout << "The offset we are using for the singly indirect is: " << offset << endl;
    // we know that each entry is 4 bytes long due to what we used for inode reading in ext2
    //cout << "The offset we got for you" << hex << offset << endl;
    //cout << "Before the for loop" << endl;
    //cout << getStreamData(4,offset,input,"",false) << endl;
    QVector <unsigned char> addToFile;

    //unsigned long long lastSize = size;

    SinglyIndirectPointers->clear();
    for(unsigned int i=0; i<block_size; i=i+4){ //4 is the number of bytes in each entry
        SinglyIndirectPointers->push_back(getStreamData(4,offset+i,input,"",false));
        //cout << SinglyIndirectPointers->back() << endl;
        if(SinglyIndirectPointers->back() == 0)
            break;
    }
    cout << "The size of the Vector is now: " << SinglyIndirectPointers->size() << endl;
    int iterator=0;
    while(iterator < SinglyIndirectPointers->size() && size>0 && SinglyIndirectPointers->at(iterator) !=0){
        offset = bootBlockLocation+(block_size * (SinglyIndirectPointers->at(iterator)));
        if(size > block_size){
            //cout << "We got into the if " << endl;
            cout << "Our Current size is "<< size << endl;
            for(unsigned int i=0; i<block_size; i++){
                unsigned char getVal = localFile.get();
                //cout << hex <<(int) getVal << endl;
                addToFile.push_back(getVal);
                //cout << "Looping in here: " << i << endl;
            }
            size= size-block_size;
            cout << "The current size in singly indirect is: " << size << endl;

            //cout << "We are out of the loop right now" << endl;
        } else {
            //cout << "We got into the else " << endl;
            //cout << "Our Current size is "<< size << endl;
            for (unsigned int i=0; i<size; i++){
                unsigned char getVal = localFile.get();
                //cout << hex <<(int) getVal << endl;
                addToFile.push_back(getVal);
            }
            size =0;
            cout << "The current size in singly indirect is: " << size << endl;
        }

        iterator ++;
        //cout << "Singly Indirect..." << endl;
        addBytesToFile(&addToFile,offset,input);
        addToFile.clear();
        if(size == 0) break;

        emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
 }


    cout << "The size we returned was" << size<< endl;
    return size;
    // end of singly indirect
}

unsigned long long VdiFile::doublyIndirectPointersValuesWrite(unsigned long long blockNumberOfDoublyIndirect, fstream& input, ifstream& localFile, unsigned long long size){
    unsigned long long offsetStartDoublyIndirect = bootBlockLocation+(block_size * (blockNumberOfDoublyIndirect));
    // we know that each entry is 4 bytes long due to what we used for inode reading in ext
    DoublyIndirectPointers->clear();
    for(unsigned int i=0; i<block_size; i=i+4){ //4 is the number of bytes in each entry
        DoublyIndirectPointers->push_back(getStreamData(4,offsetStartDoublyIndirect+i,input,"",false));
        //cout << DoublyIndirectPointers->back() << endl;
        if(DoublyIndirectPointers->back() == 0)
            break;
    }
    //cout << "Done with for loop" << endl;
    int doublyIterator=0;
    while(doublyIterator < DoublyIndirectPointers->size() && DoublyIndirectPointers->at(doublyIterator) !=0 && size>0){
        //offset = bootBlockLocation+(block_size * (DoublyIndirectPointers->at(doublyIterator)));
              size = singlyIndirectPointersValuesWrite(DoublyIndirectPointers->at(doublyIterator),input,localFile,size);
        emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
        doublyIterator++;
        qDebug() << "Doubly Indirect..." << endl;
        if(size == 0) break;

        }
    return size;


// end of doublyindirect
}

unsigned long long VdiFile::triplyIndirectPointersValuesWrite(unsigned long long blockNumberOfTriplyIndirect, fstream& input, ifstream& localFile, unsigned long long size){
    unsigned long long offsetStartTriplyIndirect = bootBlockLocation+(block_size * (blockNumberOfTriplyIndirect));
    // we know that each entry is 4 bytes long due to what we used for inode reading in ext
    TriplyIndirectPointers->clear();
    for(unsigned int i=0; i<block_size; i=i+4){ //4 is the number of bytes in each entry
        TriplyIndirectPointers->push_back(getStreamData(4,offsetStartTriplyIndirect+i,input,"",false));
        //cout << TriplyIndirectPointers->back() << endl;
        if(TriplyIndirectPointers->back() == 0)
            break;
    }
    //cout << "Done with for loop" << endl;
    int triplyIterator=0;
    while(triplyIterator < TriplyIndirectPointers->size() && TriplyIndirectPointers->at(triplyIterator) !=0 && size>0){
        //offset = bootBlockLocation+(block_size * (DoublyIndirectPointers->at(doublyIterator)));
               size = doublyIndirectPointersValuesWrite(TriplyIndirectPointers->at(triplyIterator),input,localFile,size);
        emit progressUpdate ((FileSizeForProgressBar-size)/(FileSizeForProgressBar/100));
        triplyIterator++;
        qDebug() << "Triply Indirect..." << endl;
        if(size == 0) break;

        }
    return size;


// end of triplyindirect
}



