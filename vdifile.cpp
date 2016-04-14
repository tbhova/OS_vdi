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
#include "globalfunctions.h"

using namespace std;

//convert byte data to integers
long long convertEndian(unsigned char C[], long long size){

    unsigned char temp [size];
    long long b =size-1;
    for (long long a=0; a<size; a++){
        temp[b]= C[a];
       // cout << hex << setw(2) << setfill('0') << (int)temp[b] << " ";
        b=b-1;
    }
    //cout << endl;

    long long power=size-1;
    long long total=0;
    stringstream charNums;
    for(long long i=0; i<size;i++){
        //cout << hex << setw(2) << setfill('0') << (int)temp[i] << " "<<endl;
        charNums << hex << setw(2) << setfill('0') << (int)temp[i];
       // cout << "in charNums: " << charNums.str() << endl;
        power--;
    }
    QString chars = QString::fromStdString(charNums.str());
    //cout << chars.toStdString() <<endl;
    bool ok;
    total=chars.toLongLong(&ok,16);

    //cout << total <<endl;
    if(!ok)
        cout << "Unable to convert String"<< endl;

    return total;

}

//get data from Stream
//long long getStreamData(int size, long long seek_to, std::ifstream &input, std::string name = "", bool output = true);
long long getStreamData(int size, long long seek_to, ifstream &input, string name, bool output){


    unsigned char data[size];
    input.clear();
    input.seekg(seek_to);
    for (int i=0; i<size;i++){
        input >> data[i];
        if (output)
            cout << hex << setw(2) << setfill('0') << (int)data[i] << " ";
    }
    long long size_of_part=convertEndian(data,size);
    if (output) {
        cout <<"Size of "<< name << ":" << dec<< size_of_part << endl;
        cout << endl;
    }
    return size_of_part;

}

void addBitsFromStreamData(vector<bool> *bits, int numBits, long long seek_to, ifstream &input){
    long long temp;

    input.clear();
    input.seekg(seek_to);

    for(int i = 0; i < numBits; i++) {
        //if we have used all the bits in the long long, lets get another
        if(i % (sizeof(long long)*8) == 0) {
            static int seeks = 0;
            seeks++;
            temp = getStreamData(sizeof(long long), seek_to+(sizeof(long long))*seeks, input, "", false);
            cout << endl << hex << temp << endl;
        }
        bits->push_back((temp & ((long long)1 << (sizeof(long long)*8-1-(i % (sizeof(long long)*8))))) > 0);
        if (bits->back())
            cout << '1';
        else cout << '0';
    }
    cout << endl << endl;
}

//get data from Stream
unsigned char getCharFromStream(int size, long long seek_to, ifstream &input){

    unsigned char data[size];
    input.clear();
    input.seekg(seek_to);
    for (int i=0; i<size;i++){
        input >> data[i];
       // cout << hex << setw(2) << setfill('0') << (int)data[i] << " ";
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
    unsigned int bootBlockLocation = header.offsetDataSize+mbrDataTable0*512;
    unsigned int superBlockLocation = bootBlockLocation +1024;

    superBlock = new ext2SuperBlock (this, superBlockLocation, input);

    unsigned int group_count = superBlock->getGroupCount();
    unsigned int block_size = superBlock->getBlockSize();

    unsigned int group_size = block_size*superBlock->getBlocksPerGroup();
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
    //cout << "Inode Table Location: " << hex << setw(2) << setfill('0') << groupDescriptors->getInodeTable(1) << endl;
    unsigned int inode_table_address = bootBlockLocation + block_size*groupDescriptors->getInodeTable(1);

    //cout << hex << setw(2) << setfill('0') << inode_table_address <<" "<<endl << endl<< endl;
    //unsigned int block_bitmap = getStreamData(4,inode_bitmap_address, input);



    //DataBlockBitmap = new QVector <unsigned char>;
   // fillDataBlockBitmap(DataBlockBitmap, block_bitmap_address, inode_bitmap_address, input);
    cout << 'This is the adress of block bitmap' << block_bitmap_address << endl;
    blockBitmap = new vector<bool>;
    inodesBitmap = new vector<bool>;
    addBitsFromStreamData(blockBitmap, block_size*8, block_bitmap_address, input);
    addBitsFromStreamData(inodesBitmap, block_size*8, inode_bitmap_address, input);

}

void VdiFile::closeAndReset() {
    input.close();

}


void VdiFile:: fillDataBlockBitmap(QVector<unsigned char>* DataBlockBitmap, unsigned int block_bitmap_address,unsigned int inode_bitmap_address,ifstream& input){
    for (unsigned int i=block_bitmap_address; i++; i <inode_bitmap_address){
         DataBlockBitmap->push_back(getCharFromStream(1,i,input));
    }
}
