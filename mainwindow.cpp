#include "mainwindow.h"
#include "ui_mainwindow.h"


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
