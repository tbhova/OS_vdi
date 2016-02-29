#ifndef VDIFILE_H
#define VDIFILE_H

#include <QObject>
#include <QFile>
#include "vdimap.h"

class VdiFile : public QObject
{
    Q_OBJECT
public:
    VdiFile(QObject *parent = 0);
    ~VdiFile();

public slots:
    void onVdiFileChanged(QString newFile); //slot when browse button produces a signal

private:
    QFile *vdi; //whatever filetype we intend to use
    VdiMap *map;
};

#endif // VDIFILE_H
