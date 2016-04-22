#ifndef EXT2FSENTRY_H
#define EXT2FSENTRY_H

#include <QObject>
#include <QString>

namespace CSCI5806 {
struct InodeTable {
    unsigned short i_mode;
    unsigned short i_uid;
    unsigned int i_size;
    unsigned int i_atime;
    unsigned int i_ctime;
    unsigned int i_mtime;
    unsigned int i_dtime;
    unsigned short i_gid;
    unsigned short i_links_count;
    unsigned int i_blocks;
    unsigned int i_flags;
    unsigned int i_osd1;
    unsigned int i_block[15];
    unsigned int i_generation;
    unsigned int i_file_acl;
    unsigned int i_dir_acl;
    unsigned int i_faddr;
    unsigned char i_osd2[12];

};
class ext2FSEntry
{
public:
    ext2FSEntry(InodeTable tab, unsigned int iNodeNum, QString entryName);
    InodeTable& getInodeTable();
    virtual bool isFolder() = 0;
    //bool isFile();
    void setName(QString name);
    QString getName() const;
    unsigned int getInodeNumber() const;

protected:
    InodeTable table;
    unsigned int iNodeNumber;
    QString name;
};

}

#endif // EXT2FSENTRY_H
