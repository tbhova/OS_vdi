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
using namespace std;

int convertEndian(char C[], int size){
    int  j = 0;
    char temp [size] ;
    for (int i = size-1; i>=0; i--){
        C[i] = temp[j];
        j++;

        cout << hex << setw(2) << setfill('0') << (int)temp[j] << " ";
    }
    cout << endl;

    return 0;

}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    localFS = new LocalFileSystem(ui->localFsTreeView, this);
    vdiFS = new VdiFileSystem(ui->vdiFsTreeView, this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_browseVDIPushButton_clicked()
{

}

void MainWindow::on_browseVDIPushButton_clicked()
{
    //Open File
    QString fileName = QFileDialog::getOpenFileName(this, tr("Please open a .vdi file"), "C://", ".VDI File (*.*);;All Files (*.*)");

    QMessageBox::information(this,tr("FileNameOut"),fileName);



//
// Put in Open File Method
//
    string fileString = fileName.toStdString();

    char *fileChar = new char[fileString.length() + 1];

    std::strcpy(fileChar, fileString.c_str());


    ifstream input;
    input.open(fileChar,std::ios::in);



    input >> noskipws;


            //
            // Put in getHeader Method
            //---------------------------------------------------
            //Size of Header
            unsigned char header_size[4];
            input.seekg(72);
            for (int i=0; i<4;i++){
                input >> header_size[i];
                //cout << hex << setw(2) << setfill('0') << (int)header_size[i] << " ";
                int size_of_header=convertEndian(header_size[4],4);
            }
            //cout << endl;

            //---------------------------------------------
            //Image Type
            unsigned char image_type[4];
            input.seekg(76);
            for (int i=0; i<4;i++){
                input >> image_type[i];
                //cout << hex << setw(2) << setfill('0') << (int)image_type[i] << " ";
            }
            //cout << endl;

            //---------------------------------------------
            //Offset Blocks
            unsigned char offsetBlocks[4];
            input.seekg(340);
            for (int i=0; i<4;i++){
                input >> offsetBlocks[i];
               // cout << hex << setw(2) << setfill('0') << (int)offsetBlocks[i] << " ";
            }
            //cout << endl;


            //---------------------------------------------
            //Offset Data
            unsigned char offsetData[4];
            input.seekg(344);
            for (int i=0; i<4;i++){
                input >> offsetData[i];
              //  cout << hex << setw(2) << setfill('0') << (int)offsetData[i] << " ";
            }
            // cout << endl;

            //---------------------------------------------
            //Sector Size
            unsigned char sectorSize[4];
            input.seekg(360);
            for (int i=0; i<4;i++){
                input >> sectorSize[i];
              //  cout << hex << setw(2) << setfill('0') << (int)sectorSize[i] << " ";
            }
            // cout << endl;

            //---------------------------------------------
            //Disk Size (Bytes)
            unsigned char discSize[8];
            input.seekg(368);
            for (int i=0; i<8;i++){
                input >> discSize[i];
              //  cout << hex << setw(2) << setfill('0') << (int)discSize[i] << " ";
            }
            // cout << endl;

            //---------------------------------------------
            //Block Size
            unsigned char blockSize[4];
            input.seekg(376);
            for (int i=0; i<4;i++){
                input >> blockSize[i];
            //    cout << hex << setw(2) << setfill('0') << (int)blockSize[i] << " ";
            }
            //cout << endl;

            //---------------------------------------------
            //Blocks in HDD
            unsigned char blocksInHDD[4];
            input.seekg(384);
            for (int i=0; i<4;i++){
                input >> blocksInHDD[i];
             //   cout << hex << setw(2) << setfill('0') << (int)blocksInHDD[i] << " ";
            }
            // cout << endl;

            //---------------------------------------------
            //Blocks Allocated
            unsigned char blocksAllocated[4];
            input.seekg(388);
            for (int i=0; i<4;i++){
                input >> blocksAllocated[i];
             //   cout << hex << setw(2) << setfill('0') << (int)blocksAllocated[i] << " ";
            }
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


}


















//
//
// Some Bullshit Stufff
//

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






