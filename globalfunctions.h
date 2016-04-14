#ifndef GLOBALFUNCTIONS
#define GLOBALFUNCTIONS
#include <fstream>
#include <vector>


long long convertEndian(unsigned char C[], long long size);
unsigned char getCharFromStream(int size, long long seek_to, std::ifstream &input);
long long getStreamData(int size, long long seek_to, std::ifstream &input, std::string name = "", bool output = true);

#endif // GLOBALFUNCTIONS
