#ifndef VDIFILE_H
#define VDIFILE_H

#include <QObject>
#include <QFile>
#include "vdimap.h"
#include <fstream>

class VdiFile : public QObject
{
    Q_OBJECT
public:
    VdiFile(QObject *parent = 0);
    ~VdiFile();

    void selectVdiPrompt();
    void openFile(QString fileName);

public slots:

signals:
    void vdiFileSelected(QString fileName);

private:
    void closeAndReset();

    QFile *vdi; //whatever filetype we intend to use
    VdiMap *map;
    std::ifstream input;
};

#endif // VDIFILE_H
