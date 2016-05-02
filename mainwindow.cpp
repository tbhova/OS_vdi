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
#include <QTimer>

using namespace std;
using namespace CSCI5806;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    localFS = new LocalFileSystem(ui->localFsTreeView, this); //local FS Model
    vdiFS = new VdiFileSystem(ui->vdiFsTreeView, this); //VDI FS Model

    progressLabel = new QLabel(tr("Progress: "));
    progress = new QProgressBar();
    progress->setMinimum(0);
    progress->setMaximum(100);

    ui->statusBar->addWidget(progressLabel);
    ui->statusBar->addWidget(progress);
    progress->setVisible(false);
    progressLabel->setVisible(false);

    connect(vdiFS, VdiFileSystem::vdiFileSelected, this, onVdiFileChosen); //update GUI text
    connect(this, MainWindow::browseVDIClicked, vdiFS, VdiFileSystem::onBrowseVDIClicked);
    connect(this, MainWindow::transferToLocalFS, vdiFS, VdiFileSystem::transferToLocalFS);
    connect(this, MainWindow::transferToVDI, vdiFS, VdiFileSystem::transferToVDI);

    connect(vdiFS, VdiFileSystem::progressUpdate, this, MainWindow::processProgressUpdate);


    QMessageBox WELCOME_BOX(this);

    WELCOME_BOX.setIconPixmap(QPixmap(":/images/Kramer_pic.PNG"));
    WELCOME_BOX.setButtonText(1,"Go to the filesystem GUI");
    QFont newFont("Courier", 12, QFont::Bold, false);
    WELCOME_BOX.setWindowTitle("VDI File Transfer - Morgan, Hovanec");
    WELCOME_BOX.setFont(newFont);
    WELCOME_BOX.setBaseSize(1900,550);
    WELCOME_BOX.setMinimumWidth(700);
    WELCOME_BOX.setMinimumHeight(500);
    WELCOME_BOX.setMaximumWidth(700);
    WELCOME_BOX.setMaximumHeight(500);
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
                                "Thank you for choosing the Morgan/Hovanec VDI File Transfer System! \n\n"
                                "This program was built for YSU CSCI 5806: Operating Systems \n"
                                "We will now be able to transfer information in AND out of a ext2 filesystem \n"
                                "contained within a vdi file.\n\n"
                                "Instructor: Dr. Robert Kramer \n"
                                "          - rwkramer@ysu.edu \n\n"
                                "Students: Andrew Morgan \n"
                                "          - asmorgan@student.ysu.edu \n\n"
                                "          Tyler Hovanec \n"
                                "          - tbhovanec@student.ysu.edu \n \n\n\n\n"
                                "More Information About Course:\n"
                                "   CSCI 5806: Operating Systems \n"
                                "   Department of Computer Science Information Systems\n"
                                "   Youngstown State University\n"
                                "   1 University Plaza\n"
                                "   Youngstown, OH 44555\n"
                                "Built in QT 5.51 for Windows using MinGW Compiler\n\n\n"
                                "Objective: \n\n"
                                "   - Read in Virtualbox VDI File containing an EXT2 Filesytem\n"
                                "   - Read and Write files into and out of the VDI files\n"
                                "   - Be able to update tables and bitmaps without causing system error\n\n"
                                "Documentation: \n"
                                "This is an interactive GUI that will update the .vdi file as we move files\n"
                                "in and out of it. We will also have access to all files on the local machine. \n"
                                "Number of corrupted .vdi files making this: 46\n"
                                "Number of working hours: 240+\n"
                                "Number of lines of code: ~3600\n"
                                "Number of OS students who had a great time: 2 \n");
    WELCOME_BOX.setParent(this);

    WELCOME_BOX.exec();

}

MainWindow::~MainWindow() {
    delete ui;
    delete vdiFS;
    delete localFS;
    delete progressLabel;
    delete progress;
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
        QMessageBox::information(this, tr("Error"), tr("Please choose a VDI file to copy, folder copying is not yet supported."));
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
        QMessageBox::information(this, tr("Error"), tr("Please choose a local folder to copy VDI file to."));
        return;
    }

    //emit signal with the ext2File and the destination folder
    progressLabel->setVisible(true);
    progress->setVisible(true);
    progress->setValue(0);
    emit this->transferToLocalFS(static_cast<ext2File*>(fsEntry), &dir);
}

void MainWindow::on_copyToVdiPushButton_clicked()
{
    QModelIndex returnIndex;
    //validate selected file in vdi
    bool found = false;
    QModelIndexList list = ui->vdiFsTreeView->selectionModel()->selectedIndexes();
    int row = -1;
    ext2FSEntry* fsEntry = NULL;
    foreach (QModelIndex index, list) {
        if (index.row() != row && index.column() == 0) {
            fsEntry = vdiFS->getExt2Entry(index);
            if (!fsEntry->isFolder()) {
                QModelIndex parent = vdiFS->parent(index); //get file's parent (folder)
                fsEntry = vdiFS->getExt2Entry(parent);
                if (fsEntry->isFolder()) {
                    returnIndex = parent;
                    found = true;
                }
            } else {
                returnIndex = index;
                found = true;
                break;
            }
        }
    }
    if (!found) {
        QMessageBox::information(this, tr("Error"), tr("Please choose a VDI folder to copy to."));
        return;
    }

    //validate selected file in local FS
    found = false;
    QFileInfo file;
    list = ui->localFsTreeView->selectionModel()->selectedIndexes();
    row = -1;
    QFileSystemModel *model = localFS->getFS();
    foreach (QModelIndex index, list) {
        if (index.row() != row && index.column() == 0) {
            file = model->fileInfo(index);
            if (!model->isDir(index)) {
                found = true;
            }
        }
    }

    if (!found) {
        QMessageBox::information(this, tr("Error"), tr("Please choose a local file to copy to the VDI."));
        return;
    }

    //emit signal with the ext2File and the destination folder

    //QMessageBox::information(this,tr("Funny right?"), tr("Did you really expect this feature to work?"));
    progressLabel->setVisible(true);
    progress->setVisible(true);
    progress->setValue(0);
    emit this->transferToVDI(static_cast<ext2Folder*>(fsEntry), &returnIndex, &file);
}

void MainWindow::processProgressUpdate(int value) {
    static int oldValue = -1;
    if (oldValue != value)
        progress->setValue(value);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    oldValue = value;
    if (value == 100) {
        QTimer::singleShot(8000, this, SLOT(hideStatusBar()));
    }
}

void MainWindow::hideStatusBar() {
    this->progress->setVisible(false);
    this->progressLabel->setVisible(false);
}
