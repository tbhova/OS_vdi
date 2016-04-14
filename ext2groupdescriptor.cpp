#include "ext2groupdescriptor.h"
#include "globalfunctions.h"
#include <iostream>
#include <iomanip>
#include <ios>
#include <QVector>
using namespace std;

ext2GroupDescriptor::ext2GroupDescriptor(QObject *parent, int group_count, long long offset, ifstream &file) : QObject(parent)
{
    groupDescriptors = new QVector <ext2_group_desc>;

    cout << "Read in data from group descriptor table" << endl;
    for(int i=offset; i< (offset + (group_count * 32)); i+=32){
       blockGroup.bg_block_bitmap = getStreamData(4,i,file);
       blockGroup.bg_inode_bitmap = getStreamData(4,i+4, file);
       blockGroup.bg_inode_table = getStreamData(4,i+8, file);
       blockGroup.bg_free_blocks_count = getStreamData(2,i+12, file);
       blockGroup.bg_free_inodes_count = getStreamData(2, i+14, file);
       blockGroup.bg_used_dirs_count = getStreamData(2,i+16, file);
       blockGroup.bg_pad = getStreamData(2,i+18,file);

       groupDescriptors->push_back(blockGroup);
    }



}

int ext2GroupDescriptor::getBlockBitmap(int block) {
    cout << "Block Bitmap Location (block#): " << groupDescriptors->at(0).bg_block_bitmap << endl;
    return groupDescriptors->at(0).bg_block_bitmap;
}

int ext2GroupDescriptor::getInodeBitmap(int block) {
    cout << "Inode Bitmap Location (block#): " << groupDescriptors->at(0).bg_inode_bitmap << endl;
    return groupDescriptors->at(0).bg_inode_bitmap;
}

int ext2GroupDescriptor::getInodeTable(int block) {
    cout << "InodeTable Location (block#): " << dec << groupDescriptors->at(0).bg_inode_table << endl;
    return groupDescriptors->at(0).bg_inode_table;
}


