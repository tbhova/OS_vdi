#include "mbrdata.h"
#include "vdifunctions.h"
#include <iostream>
#include <iomanip>
#include <ios>

using namespace std;
using namespace CSCI5806;

//get data from Stream
unsigned char getStreamDataChar(int size, long long seek_to, fstream &input){

    unsigned char data[size];
    input.clear();
    input.seekg(seek_to);
    for (int i=0; i<size;i++){
        input >> data[i];
        cout << hex << setw(2) << setfill('0') << (int)data[i] << " ";
    }
    cout <<"" <<endl;
    return data[size];

}

mbrData::mbrData(QObject *parent, long long startOffset,  fstream &file) : QObject(parent)
{
    partitionTable1[16] = getStreamDataChar(16, startOffset+446,file);
    partitionTable2[16] = getStreamDataChar(16, startOffset+462,file);
    partitionTable3[16] = getStreamDataChar(16, startOffset+478,file);
    partitionTable4[16] = getStreamDataChar(16, startOffset+494,file);

    cout <<"Done getting data from Partitions" <<endl;
    cout << "The Magic Number for mbr is:" <<endl;
    eofMagicNumber = getStreamData(2, startOffset+510, file);

    if (eofMagicNumber == 0xAA55){
        cout << "Correct Magic Numbers" << endl;
    }

    mbrDataTable[0] = getStreamData(4,startOffset+454,file);
    mbrDataTable[1] = getStreamData(4,startOffset+470,file);
    mbrDataTable[2] = getStreamData(4,startOffset+486,file);
    mbrDataTable[3] = getStreamData(4,startOffset+502,file);

}

int mbrData::getMbrDataTable(int pos) {
    return mbrDataTable[pos];

}
