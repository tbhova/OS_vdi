#include "vdimap.h"

VdiMap::VdiMap(QObject *parent) : QObject(parent)
{
    map = new QByteArray;
}

