#ifndef EXT2FOLDER_H
#define EXT2FOLDER_H

//#include <QObject>
#include <QString>
#include <QVector>
#include "ext2file.h"
#include "ext2fsentry.h"

namespace CSCI5806 {
class ext2Folder : public ext2FSEntry
{
public:
    ext2Folder(InodeTable tab, QString entryName);

    QVector<ext2Folder*>* getFolders();
    QVector<ext2File*>* getFiles();

private:
    QVector<ext2Folder*> *folders;
    QVector<ext2File*> *files;
    QString path;
};
}

#endif // EXT2FOLDER_H
