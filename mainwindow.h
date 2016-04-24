#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "localfilesystem.h"
#include "vdifilesystem.h"



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

private:
    Ui::MainWindow *ui;
    CSCI5806::LocalFileSystem *localFS;
    CSCI5806::VdiFileSystem *vdiFS;
    //CSCI5806::VdiFile *vdi;
};


#endif // MAINWINDOW_H
