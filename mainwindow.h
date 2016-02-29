#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "localfilesystem.h"
#include "vdifilesystem.h"
#include "vdifile.h"

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

private slots:
    void on_browseVDIPushButton_clicked();

private:
    Ui::MainWindow *ui;
    LocalFileSystem *localFS;
    VdiFileSystem *vdiFS;
    VdiFile *vdi;
};

#endif // MAINWINDOW_H
