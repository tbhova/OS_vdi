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
#include <QDebug>

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
    connect(this, MainWindow::transferToLocalFS, vdiFS, VdiFileSystem::transferToLocalFS);
    connect(this, MainWindow::transferToVDI, vdiFS, VdiFileSystem::transferToVDI);

/*
    QMessageBox WELCOME_BOX(this);

    WELCOME_BOX.setIconPixmap(QPixmap(":/images/Kramer_pic.PNG"));
    WELCOME_BOX.setButtonText(1,"Go to the filesystem GUI");
    QFont newFont("Courier", 12, QFont::Bold, false);
    WELCOME_BOX.setWindowTitle("VDI File Transfer - Morgan, Hovanec");
    WELCOME_BOX.setFont(newFont);
    WELCOME_BOX.setInformativeText("                   WELCOME! \n \n \n"
                                   "Thank you for choosing the Morgan/Hovanec VDI File Transfer System \n\n"
                                   "This program was built for YSU CSCI 5806: Operating Systems \n \n"
                                   "Instructor: Dr. Robert Kramer \n"
                                   "          - rwkramer@ysu.edu \n\n"
                                   "Students: Andrew Morgan \n"
                                   "          - asmorgan@student.ysu.edu \n \n"
                                   "          Tyler Hovanec \n"
                                   "          - tbhovanec@student.ysu.edu \n \n\n\n\n ");
    WELCOME_BOX.informativeText();
    WELCOME_BOX.setDetailedText("                   WELCOME! \n \n \n"
                                "Thank you for choosing the Morgan/Hovanec VDI File Transfer System \n\n"
                                "This program was built for YSU CSCI 5806: Operating Systems \n \n"
                                "Instructor: Dr. Robert Kramer \n"
                                "          - rwkramer@ysu.edu \n\n"
                                "Students: Andrew Morgan \n"
                                "          - asmorgan@student.ysu.edu \n \n"
                                "          Tyler Hovanec \n"
                                "          - tbhovanec@student.ysu.edu \n \n\n\n\n "
                                "More Information About Course:\n"
                                "   CSCI 5806: Operating Systems \n"
                                "   Department of Computer Science Information Systems\n"
                                "   Youngstown State University\n"
                                "   1 University Plaza\n"
                                "   Youngstown, OH 44555\n"
                                "Built in QT for Windows using MinGW Compiler\n\n\n"
                                "Objective: \n\n"
                                "   - Read in Virtualbox VDI File containing an EXT2 Filesytem\n"
                                "   - Read and Write files into and out of the VDI files\n"
                                "   - Be able to update tables and bitmaps without causing system error\n");
    WELCOME_BOX.setParent(this);

    WELCOME_BOX.exec();
*/

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

void MainWindow::on_copyToLocalFsButton_clicked()
{
    //validate selected file in vdi
    QModelIndexList list = ui->vdiFsTreeView->selectionModel()->selectedIndexes();
    int row = -1;
    ext2FSEntry* fsEntry = NULL;
    foreach (QModelIndex index, list) {
        if (index.row() != row && index.column() == 0) {
            fsEntry = vdiFS->getExt2Entry(index);
            if (fsEntry->isFolder()) {
                fsEntry = NULL;
            } else
                break;
        }
    }
    if (fsEntry == NULL) {
        QMessageBox::information(this, tr("Error"), tr("Please choose a file to copy, folder copying is not yet supported."));
        return;
    }

    //validate selected folder in local FS
    bool found = false;
    QDir dir;
    list = ui->localFsTreeView->selectionModel()->selectedIndexes();
    row = -1;
    QFileSystemModel *model = localFS->getFS();
    foreach (QModelIndex index, list) {
        if (index.row() != row && index.column() == 0) {
            QFileInfo info = model->fileInfo(index);
            found = true;
            if (model->isDir(index)) {
                dir = QDir(info.absoluteFilePath());
            } else {
                dir = info.absoluteDir();

            }
        }
    }

    if (!found) {
        QMessageBox::information(this, tr("Error"), tr("Please choose a destination folder to copy to."));
        return;
    }

    //emit signal with the 2 paths
    emit this->transferToLocalFS(static_cast<ext2File*>(fsEntry), &dir);
}

void MainWindow::on_copyToVdiPushButton_clicked()
{
    QMessageBox::information(this,tr("Funny right?"), tr("Did you really expect this feature to work?"));

    emit this->transferToVDI(tr(""), tr(""));
}
