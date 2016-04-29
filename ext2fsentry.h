#ifndef EXT2FSENTRY_H
#define EXT2FSENTRY_H

#include <QObject>
#include <QString>

namespace CSCI5806 {
struct InodeTable {
    unsigned short i_mode;          /*File/directory type and permissions */
    unsigned short i_uid;           /* Owner Uid */
    unsigned int i_size;            /* Size (bytes) */
    unsigned int i_atime;           /* access time (timestamp) */
    unsigned int i_ctime;           /* creation time */
    unsigned int i_mtime;           /* modify time */
    unsigned int i_dtime;           /* delete time */
    unsigned short i_gid;           /* group id */
    unsigned short i_links_count;   /* hard link count */
    unsigned int i_blocks;          /* blocks count */
    unsigned int i_flags;           /* file flags */
    unsigned int i_osd1;            /* OS Specific value 1 */
    unsigned int i_block[15];       /* pointers to data blocks, 12 direct */
    unsigned int i_generation;      /* Genmeration number (for NFS) */
    unsigned int i_file_acl;        /* File ACL */
    unsigned int i_dir_acl;         /* Directory ACL */
    unsigned int i_faddr;           /* address of fragment */
    unsigned char i_osd2[12];       /* OS Specfic value 2 */

};
class ext2FSEntry
{
public:
    ext2FSEntry(InodeTable tab, unsigned int iNodeNum, QString entryName);
    InodeTable* getInodeTable();
    virtual bool isFolder() = 0;
    QString getName() const;
    unsigned int getInodeNumber() const;
    virtual bool operator==(const ext2FSEntry &other) const;

protected:
    InodeTable table;
    unsigned int iNodeNumber;
    QString name;
};

}

#endif // EXT2FSENTRY_H
