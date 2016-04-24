#include "vdifunctions.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <QString>
#include <fstream>
#include <vector>
#include <QObject>
#include <QDebug>


using namespace std;

namespace CSCI5806 {

//convert byte data to integers
unsigned long long convertEndian(unsigned char C[], int size, bool littleEndian){

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
    if(!ok) {
        cout << "Unable to convert String"<< endl;

        /*cout << "lets crash this boat" << endl;
        int *i = NULL;
        i++;
        (*i)++;*/
    }

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
       // cout << hex << setw(2) << setfill('0') << (int)data[i] << " ";
    }
    //cout <<"" <<endl;
    return data[size];

}

QString FileSizeToString(unsigned long long size) {
    QString ret = "";
    if (size == 0)
        return ret;

    int divisions = 0, tempSize = size;

    while(tempSize > 0) {
        tempSize /= 1024;
        divisions++;
    }
    tempSize = size;
    for (int i = 0; i < divisions-1; i++) {
        tempSize /= 1024;
    }
    ret += QString::number(tempSize);

    switch (divisions) {
        case(1) :
            ret += QObject::tr(" bytes");
            break;
        case(2) :
            ret += QObject::tr(" KB");
            break;
        case(3) :
            ret += QObject::tr(" MB");
            break;
        case(4) :
            ret += QObject::tr(" GB");
            break;
    }
    return ret;
}
}
