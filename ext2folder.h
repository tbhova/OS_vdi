#ifndef EXT2FOLDER_H
#define EXT2FOLDER_H

#include <QVector>
#include "ext2file.h"
#include "ext2fsentry.h"

/* much like ext2File, most code is inherited from ext2FSEntry
 * these are non leaf nodes of the tree held by ext2FSManager
 * */

namespace CSCI5806 {
class ext2Folder : public ext2FSEntry
{
public:
    ext2Folder(InodeTable tab, unsigned int iNodeNum, QString entryName);
    virtual ~ext2Folder();

    QVector<ext2Folder*>* getFolders();
    QVector<ext2File*>* getFiles();
    virtual bool isFolder();
    virtual bool operator==(const ext2Folder &other) const;

private:
    //pointers to children nodes
    QVector<ext2Folder*> *folders;
    QVector<ext2File*> *files;
};
}

#endif // EXT2FOLDER_H
