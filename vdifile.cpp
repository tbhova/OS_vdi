#include "vdifile.h"

VdiFile::VdiFile(QObject *parent) : QObject(parent)
{
    vdi = new QFile();
    map = new VdiMap(this);
}

VdiFile::~VdiFile() {
    delete vdi;
    delete map;
}

void VdiFile::onVdiFileChanged(QString newFile) {

}
