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


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //andy fork
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_browseVDIPushButton_clicked()
{
    //Open File
    QString fileName = QFileDialog::getOpenFileName(this, tr("Please open a .vdi file"), "C://", ".VDI File (*.*);;All Files (*.*)");

    QMessageBox::information(this,tr("FileNameOut"),fileName);

    //string file2 = fileName.toStdString();



    unsigned char x;
    ifstream input;
    input.open("Test.vdi",std::ios::in);
    if(!input.is_open())
        cout << "tru" << endl;
    input >> noskipws;
    while (input >> x) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (int)x;
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
}
