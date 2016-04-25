#ifndef VDIFUNCTIONS
#define VDIFUNCTIONS
#include <fstream>
#include <vector>
#include <QString>
#include <vdifile.h>
#include <vdimap.h>

namespace CSCI5806 {

struct VdiHeaderGlobal {
long long offsetDataSize;
long long blockSize;
long long blocksInHDD;
};

extern VdiMap *globalMap;
extern VdiHeaderGlobal globalHeader;

//QVector < unsigned int > *globalVDIMap; //dyanmic array of char for the map


void addBitsFromStreamData(std::vector<bool> *bits, int numBits, long long seek_to, std::ifstream &input);
unsigned long long convertEndian(unsigned char C[], int size, bool littleEndian = false);
unsigned char getCharFromStream(int size, long long seek_to, std::ifstream &input);
unsigned long long getStreamData(int size, long long seek_to, std::ifstream &input, std::string name = "", bool output = true, bool littleEndian = false, bool afterVDIMap = true);
QString FileSizeToString(unsigned long long size);




}

#endif // VDIFUNCTIONS
