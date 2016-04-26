#ifndef EXT2FOLDER_H
#define EXT2FOLDER_H

#include <QString>
#include <QVector>
#include "ext2file.h"
#include "ext2fsentry.h"

namespace CSCI5806 {
class ext2Folder : public ext2FSEntry
{
public:
    ext2Folder(InodeTable tab, unsigned int iNodeNum, QString entryName);

    QVector<ext2Folder*>* getFolders();
    QVector<ext2File*>* getFiles();
    virtual bool isFolder();

private:
    QVector<ext2Folder*> *folders;
    QVector<ext2File*> *files;
};
}

#endif // EXT2FOLDER_H
