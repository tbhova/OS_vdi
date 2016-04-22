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
    localFS = new LocalFileSystem(ui->localFsTreeView, this); //local FS Model
    vdiFS = new VdiFileSystem(ui->vdiFsTreeView, this); //VDI FS Model

    connect(vdiFS, VdiFileSystem::vdiFileSelected, this, onVdiFileChosen); //update GUI text
    connect(this, MainWindow::browseVDIClicked, vdiFS, VdiFileSystem::onBrowseVDIClicked);
}

MainWindow::~MainWindow()
{
#warning double check for memory leaks
    delete ui;
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
    emit browseVDIClicked();
}
