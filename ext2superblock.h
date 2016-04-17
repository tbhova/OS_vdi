#ifndef EXT2SUPERBLOCK_H
#define EXT2SUPERBLOCK_H
#include <fstream>
#include <QObject>

namespace CSCI5806 {
class ext2SuperBlock : public QObject
{
    Q_OBJECT
public:
    explicit ext2SuperBlock(QObject *parent, long long startOffset,  std::ifstream &file);
    int getGroupCount();
    int getGroupDescriptorBlockNumber();
    int getBlockSize();
    int getBlocksPerGroup();

signals:

public slots:

private:
           unsigned int   s_inodes_count;         /* Inodes count */
           unsigned int   s_blocks_count;         /* Blocks count */
           unsigned int   s_r_blocks_count;       /* Reserved blocks count */
           unsigned int   s_free_blocks_count;    /* Free blocks count */
           unsigned int   s_free_inodes_count;    /* Free inodes count */
           unsigned int   s_first_data_block;     /* First Data Block */
           unsigned int   s_log_block_size;       /* Block size */
           int   s_log_frag_size;                 /* Fragment size */
           unsigned int   s_blocks_per_group;     /* # Blocks per group */
           unsigned int   s_frags_per_group;      /* # Fragments per group */
           unsigned int   s_inodes_per_group;     /* # Inodes per group */
           unsigned int   s_mtime;                /* Mount time */
           unsigned int   s_wtime;                /* Write time */
           unsigned short   s_mnt_count;          /* Mount count */
           short   s_max_mnt_count;               /* Maximal mount count */
           unsigned short   s_magic;              /* Magic signature */
           unsigned short   s_state;              /* File system state */
           unsigned short   s_errors;             /* Behaviour when detecting errors */
           unsigned short   s_pad;
           unsigned int   s_lastcheck;            /* time of last check */
           unsigned int   s_checkinterval;        /* max. time between checks */
           unsigned int   s_creator_os;           /* OS */
           unsigned int   s_rev_level;            /* Revision level */
           unsigned short   s_def_resuid;           /* Default uid for reserved blocks */
           unsigned short   s_def_resgid;           /* Default gid for reserved blocks */

           unsigned int group_count;
           int block_size;
};
}

#endif // EXT2SUPERBLOCK_H
