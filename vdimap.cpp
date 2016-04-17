#include "vdimap.h"
#include "globalfunctions.h"
#include <iostream>
#include <iomanip>
#include <ios>
using namespace std;

VdiMap::VdiMap(QObject *parent, long long startOffset, long long stopOffset, ifstream &file) : QObject(parent)
{
    map = new QVector <unsigned int>;

    for (long long i= startOffset; i< stopOffset; i+=4){
        map->push_back((unsigned long long)getStreamData(4, i, file));
        //cout << hex << setw(2) << setfill('0') << i << " "<<endl;
        //cout << map->at(map->size()-1) << endl;
    }

    cout << map->at(12) << endl;

}

int VdiMap::getMappedLocation(int location) {
    return map->at(location);

}
