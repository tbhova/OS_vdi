#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "localfilesystem.h"
#include "vdifilesystem.h"
#include "ext2file.h"
#include <QDir>
#include <QFile>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onVdiFileChosen(QString fileName);
    void on_browseVDIPushButton_clicked();

signals:
    void browseVDIClicked();
    void transferToLocalFS(CSCI5806::ext2File *sourceFile, QDir *destDir);
    void transferToVDI(CSCI5806::ext2Folder *VDIFolder, QFileInfo *sourceFile);

private slots:
    void on_copyToLocalFsButton_clicked();

    void on_copyToVdiPushButton_clicked();

private:
    Ui::MainWindow *ui;
    CSCI5806::LocalFileSystem *localFS;
    CSCI5806::VdiFileSystem *vdiFS;
};


#endif // MAINWINDOW_H
