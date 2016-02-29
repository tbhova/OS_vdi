#include "vdifile.h"
#include <QMessageBox>
#include <QFileDialog>
#include <string>
#include <iostream>

using namespace std;

VdiFile::VdiFile(QObject *parent) : QObject(parent)
{
    vdi = new QFile();
    map = new VdiMap(this);
}

VdiFile::~VdiFile() {
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
        cout << "tru" << endl;
    input >> noskipws;

    initialized = true;
}

void VdiFile::closeAndReset() {
    input.close();

}
