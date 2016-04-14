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
            cout << endl<<endl;
            cout << hex << setw(2) << setfill('0') << (int)data[i] << " ";
        }
    }
    unsigned long long size_of_part;
    size_of_part=convertEndian(data, size, littleEndian);
    if (output) {
        cout <<"Size of "<< name << ":" << dec << size_of_part << endl;
        //cout << endl;
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
    cout << "Hellooo::: Inode Bitmap Location: " << hex << setw(2) << setfill('0') << bootBlockLocation + block_size*groupDescriptors->getInodeBitmap(1) << endl;
    unsigned int inode_table_address = bootBlockLocation + block_size*groupDescriptors->getInodeTable(1);
    cout << "Hellooo::: Inode Table Location: " << hex << setw(2) << setfill('0') << bootBlockLocation + inode_table_address << endl;

    //cout << hex << setw(2) << setfill('0') << inode_table_address <<" "<<endl << endl<< endl;
    //unsigned int block_bitmap = getStreamData(4,inode_bitmap_address, input);



    //DataBlockBitmap = new QVector <unsigned char>;
   // fillDataBlockBitmap(DataBlockBitmap, block_bitmap_address, inode_bitmap_address, input);
    cout << "This is the adress of block bitmap" << hex<< block_bitmap_address << endl;
    blockBitmap = new vector<bool>;
    inodesBitmap = new vector<bool>;
    addBitsFromStreamData(blockBitmap, block_size*8, block_bitmap_address, input);

    addBitsFromStreamData(inodesBitmap, block_size*8, inode_bitmap_address, input);
    cout << dec <<blockBitmap->size() << " " << sizeof(*blockBitmap) << endl;
    cout << blockBitmap->max_size() << endl;
    cout << "Bit reading/ converting complete" << endl;



}

void VdiFile::closeAndReset() {
    input.close();

}


void VdiFile:: fillDataBlockBitmap(QVector<unsigned char>* DataBlockBitmap, unsigned int block_bitmap_address,unsigned int inode_bitmap_address,ifstream& input){
    for (unsigned int i=block_bitmap_address; i++; i <inode_bitmap_address){
         DataBlockBitmap->push_back(getCharFromStream(1,i,input));
    }
}
