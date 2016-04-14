#ifndef GLOBALFUNCTIONS
#define GLOBALFUNCTIONS
#include <fstream>
#include <vector>


unsigned long long convertEndian(unsigned char C[], long long size, bool littleEndian = false);
unsigned char getCharFromStream(int size, long long seek_to, std::ifstream &input);
unsigned long long getStreamData(int size, long long seek_to, std::ifstream &input, std::string name = "", bool output = true, bool littleEndian = false);

#endif // GLOBALFUNCTIONS
