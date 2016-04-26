#include "ext2superblock.h"
#include "vdifunctions.h"
#include <iostream>
#include <iomanip>
#include <ios>
#include <cmath>

using namespace std;
using namespace CSCI5806;

ext2SuperBlock::ext2SuperBlock(QObject *parent, long long startOffset,  ifstream &file) : QObject(parent)
{

    cout << "The start of the superblock was " << hex << startOffset << endl;
    //All values are read in according to their sizes. These values do not change per super
    //block. The superblock is 1024 bytes in length, but does not use the last 936 bytes
    //for our use.

    //Note the names of these are standardized, and were found online to keep it easy to read
    cout << "Start of SuperBlock" <<endl;
    s_inodes_count = getStreamData(4,startOffset,file, "Number of Inodes");            /* Inodes count */
    s_blocks_count = getStreamData(4,startOffset+4,file, "Number of Blocks");          /* Blocks count */
    s_r_blocks_count = getStreamData(4,startOffset+8,file, " Reserved Blocks");        /* Reserved blocks count */
    s_free_blocks_count = getStreamData(4,startOffset+12,file, "Blocks Free");    /* Free blocks count */
    s_free_inodes_count = getStreamData(4,startOffset+16,file, "Free Inodes");    /* Free inodes count */
    s_first_data_block = getStreamData(4,startOffset+20,file, "First Data Block");     /* First Data Block */
    s_log_block_size = getStreamData(4,startOffset+24,file, "Block Size");       /* Block size */
    s_log_frag_size = getStreamData(4,startOffset+28,file, "Fragment Size");        /* Fragment size */
    s_blocks_per_group = getStreamData(4,startOffset+32,file, "Blocks per Group");     /* # Blocks per group */
    s_frags_per_group = getStreamData(4,startOffset+36,file, "Frags per Group");      /* # Fragments per group */
    s_inodes_per_group = getStreamData(4,startOffset+40,file, "Inodes per Group");     /* # Inodes per group */
    s_mtime = getStreamData(4,startOffset+44,file);                /* Mount time */
    s_wtime = getStreamData(4,startOffset+48,file);                /* Write time */
    s_mnt_count= getStreamData(2,startOffset+52,file);             /* Mount count */
    s_max_mnt_count= getStreamData(2,startOffset+54,file);         /* Maximal mount count */
    s_magic= getStreamData(2,startOffset+56,file, "Magic Number") ;                /* Magic signature */
    s_state= getStreamData(2,startOffset+58,file) ;                /* File system state */
    s_errors= getStreamData(2,startOffset+60,file) ;               /* Behaviour when detecting errors */
    s_pad= getStreamData(2,startOffset+62,file) ;
    s_lastcheck = getStreamData(4,startOffset+66,file);            /* time of last check */
    s_checkinterval = getStreamData(4,startOffset+70,file);        /* max. time between checks */
    s_creator_os = getStreamData(4,startOffset+74,file);           /* OS */
    s_rev_level = getStreamData(4,startOffset+78,file);            /* Revision level */
    s_def_resuid = getStreamData(2,startOffset+82,file);           /* Default uid for reserved blocks */
    s_def_resgid = getStreamData(2,startOffset+84,file);           /* Default gid for reserved blocks */


    if(s_magic == 0xEF53){
        cout << "Super Block location correct" <<endl;
    }

    /* calculate number of block groups on the disk */
    group_count = 1 + ((s_blocks_count-1) / s_blocks_per_group);
    unsigned int altGroup_count = 1 + ((s_inodes_count-1) / s_inodes_per_group);
    if (group_count != altGroup_count)
        cout << "Bad! calculated group count does not match for both methods" << endl;

    block_size = 1024*pow (2,s_log_block_size);
    cout << "The block size for the file system is: " << block_size << endl;

    cout << "The block multiplier is" << getGroupDescriptorBlockNumber() << endl;
}

int ext2SuperBlock::getGroupCount() {
    return group_count;

}

int ext2SuperBlock::getGroupDescriptorBlockNumber() {
    if(block_size == 1024){
        return 2;}
    else
        return 1;
}

int ext2SuperBlock::getBlockSize() {
    return block_size;
}

int ext2SuperBlock::getBlocksPerGroup() {
    return s_blocks_per_group;
}

int ext2SuperBlock::getInodesPerGroup() {
    return s_inodes_per_group;
}




