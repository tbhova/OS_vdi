#ifndef VDIFUNCTIONS
#define VDIFUNCTIONS
#include <fstream>
#include <vector>
#include <QString>

namespace CSCI5806 {

void addBitsFromStreamData(std::vector<bool> *bits, int numBits, long long seek_to, std::ifstream &input);
unsigned long long convertEndian(unsigned char C[], int size, bool littleEndian = false);
unsigned char getCharFromStream(int size, long long seek_to, std::ifstream &input);
unsigned long long getStreamData(int size, long long seek_to, std::ifstream &input, std::string name = "", bool output = true, bool littleEndian = false);
QString FileSizeToString(unsigned long long size);

}

#endif // VDIFUNCTIONS
