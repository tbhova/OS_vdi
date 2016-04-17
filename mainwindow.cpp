#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog.h>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <qmessagebox.h>
#include <iostream>
#include <String>
#include <iostream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <cmath>
#include <sstream>

using namespace std;
using namespace CSCI5806;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    localFS = new LocalFileSystem(ui->localFsTreeView, this);
    vdi = new VdiFile();
    vdiFS = new VdiFileSystem(ui->vdiFsTreeView, vdi, this);

    connect(vdi, VdiFile::vdiFileSelected, this, onVdiFileChosen);
}

MainWindow::~MainWindow()
{
#warning double check for memory leaks
    delete ui;
    delete vdi;
    delete vdiFS;
}

void MainWindow::onVdiFileChosen(QString fileName) {
    //find index of last / character
    int vdiNameIndex = fileName.lastIndexOf("/")+1; //+1 to skip the '/'

    //set the selectVDI label to state the selected VDI file
    ui->selecVDILabel->setText(QString("Selected VDI: ").append(fileName.midRef(vdiNameIndex)));
}

void MainWindow::on_browseVDIPushButton_clicked()
{
    vdi->selectVdiPrompt();
}











//
//
// Some Bullshit Stuff
//
/*
 * //Open File
    QString fileName = QFileDialog::getOpenFileName(this, tr("Please open a .vdi file"), "C://", ".VDI File (*.*);;All Files (*.*)");

    QMessageBox::information(this,tr("FileNameOut"),fileName);



//
// Put in Open File Method
//
    string fileString = fileName.toStdString();

    cout << fileString << endl;
    char *fileChar = new char[fileString.length() + 1];

    std::strcpy(fileChar, fileString.c_str());


    ifstream input;
    input.open(fileChar,std::ios::in);


    // Check to see if file is open
    if(!input.is_open())
        cout << "File not open" << endl;
    input >> noskipws;

/*
            //
            // Put in getHeader Method
            //---------------------------------------------------
            //Size of Header
            unsigned char header_size[4];
            input.seekg(72);
            for (int i=0; i<4;i++){
                input >> header_size[i];
                //cout << hex << setw(2) << setfill('0') << (int)header_size[i] << " ";
            }
            long long size_of_header=convertEndian(header_size,4);
           // cout << dec<< size_of_header << endl;
            //cout << endl;

            //---------------------------------------------
            //Image Type
            unsigned char image_type[4];
            input.seekg(76);
            for (int i=0; i<4;i++){
                input >> image_type[i];
                //cout << hex << setw(2) << setfill('0') << (int)image_type[i] << " ";
            }
            long long image_type_size=convertEndian(image_type,4);
           // cout << image_type_size <<endl;




            //---------------------------------------------
            //Offset Blocks
            unsigned char offsetBlocks[4];
            input.seekg(340);
            for (int i=0; i<4;i++){
                input >> offsetBlocks[i];
               // cout << hex << setw(2) << setfill('0') << (int)offsetBlocks[i] << " ";
            }
            long long offsetBlocks_size=convertEndian(offsetBlocks,4);
            //cout << endl;


            //---------------------------------------------
            //Offset Data
            unsigned char offsetData[4];
            input.seekg(344);
            for (int i=0; i<4;i++){
                input >> offsetData[i];
              //  cout << hex << setw(2) << setfill('0') << (int)offsetData[i] << " ";
            }
            long long offsetData_size=convertEndian(offsetData,4);
            // cout << endl;

            //---------------------------------------------
            //Sector Size
            unsigned char sectorSize[4];
            input.seekg(360);
            for (int i=0; i<4;i++){
                input >> sectorSize[i];
              //  cout << hex << setw(2) << setfill('0') << (int)sectorSize[i] << " ";
            }
            long long sectorSize_size=convertEndian(sectorSize,4);
            // cout << endl;

            //---------------------------------------------
            //Disk Size (Bytes)
            unsigned char discSize[8];
            input.seekg(368);
            for (int i=0; i<8;i++){
                input >> discSize[i];
                //cout << hex << setw(2) << setfill('0') << (int)discSize[i] << " ";
            }
           // cout << endl;
            long long discSize_size=convertEndian(discSize,8);
            //cout << discSize_size << endl;
            //cout << endl;

            //---------------------------------------------
            //Block Size
            unsigned char blockSize[4];
            input.seekg(376);
            for (int i=0; i<4;i++){
                input >> blockSize[i];
            //    cout << hex << setw(2) << setfill('0') << (int)blockSize[i] << " ";
            }
            long long blockSize_size=convertEndian(blockSize,4);
            //cout << endl;

            //---------------------------------------------
            //Blocks in HDD
            unsigned char blocksInHDD[4];
            input.seekg(384);
            for (int i=0; i<4;i++){
                input >> blocksInHDD[i];
             //   cout << hex << setw(2) << setfill('0') << (int)blocksInHDD[i] << " ";
            }
            long long blocksInHDD_size=convertEndian(blocksInHDD,4);
            // cout << endl;

            //---------------------------------------------
            //Blocks Allocated
            unsigned char blocksAllocated[4];
            input.seekg(388);
            for (int i=0; i<4;i++){
                input >> blocksAllocated[i];
             //   cout << hex << setw(2) << setfill('0') << (int)blocksAllocated[i] << " ";
            }
            long long blocksAllocated_size=convertEndian(blocksAllocated,4);
            //cout << endl;

            //---------------------------------------------
            //UUID of this VDI
            unsigned char UUIDofVDI[16];
            input.seekg(392);
            for (int i=0; i<16;i++){
                input >> UUIDofVDI[i];
             //   cout << hex << setw(2) << setfill('0') << (int)UUIDofVDI[i] << " ";
            }
            //cout << endl;


            //---------------------------------------------
            //UUID of last SNAP
            unsigned char UUIDofSNAP[16];
            input.seekg(408);
            for (int i=0; i<16;i++){
                input >> UUIDofSNAP[i];
              //  cout << hex << setw(2) << setfill('0') << (int)UUIDofSNAP[i] << " ";
            }
            //cout << endl;

/*      //83 is decimal for 53 in hex, 239 for EF
    unsigned char a;
    unsigned char b;
    long long count =1;
    input.seekg(3853712);
    input >> a;
    input >> b;

    cout << "done seeking" << endl;
    while ((int)a!= 83 || (int)b !=239){
        count ++;
        input.unget();
        input >> a;
        input >> b;
    }
        cout << count << endl;

  input.seekg(5);
    unsigned char y;
    input>>y;
    std::cout << std::hex << std::setw(2) << std::setfill('0')
          << (int)y << " ";
    input>>y;
    std::cout << std::hex << std::setw(2) << std::setfill('0')
          << (int)y << " ";
    input>>y;
    std::cout << std::hex << std::setw(2) << std::setfill('0')
          << (int)y << " ";
    input>>y;
    std::cout << std::hex << std::setw(2) << std::setfill('0')
          << (int)y << " ";
    cout << endl;
input>>y;
    std::cout << std::hex << std::setw(2) << std::setfill('0')
          << (int)y << " ";

    input.unget();
input>>y;
    std::cout << std::hex << std::setw(2) << std::setfill('0')
          << (int)y << " ";
    cout << endl;



    for (int i=0; i<1000;i++){
        for(int j=0; j<10;j++){
            input >> x[j];
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (int)x[j] << " ";
        }
    cout << endl;
    }



   // QFile file(fileName);


        //file.open;

    //Check File with error checker
    //if(!file.open(QIODevice::ReadOnly))
      //  QMessageBox::information(0,"Invalid File",file.errorString());

    //QString hexText = QByteArray::fromHex(file.read(8));

    //cout <<  hexText.toStdString()<<endl;



    //QTextStream in (&file);
    //in.setIntegerBase(16);

    //QString input = in.read(4);
    //cout <<  input.toStdString()<<endl;
    //int hello;
    //in >> hex>> hello;
   // cout << hex<< hello <<endl;
    /*

    QTextStream in (&file);
    in.setIntegerBase(16);

    QString input = in.read(5);

    cout <<  input.toStdString()<<endl;

    while (!input.isEmpty()){
        cout << input.toStdString()<< endl;
        QString input = in.read(64);
*/






