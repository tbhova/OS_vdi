#include "ext2groupdescriptor.h"
#include "vdifunctions.h"
#include <iostream>
#include <iomanip>
#include <ios>
#include <QVector>
using namespace std;
using namespace CSCI5806;

ext2GroupDescriptor::ext2GroupDescriptor(QObject *parent, int group_count, long long offset, ifstream &file) : QObject(parent)
{
    groupDescriptors = new QVector <ext2_group_desc>;

    cout << "Read in data from group descriptor table" << endl;
    for(int i=offset; i< (offset + (group_count * 32)); i+=32){
       blockGroup.bg_block_bitmap = getStreamData(4,i,file,"Block Bitmap Block");
       blockGroup.bg_inode_bitmap = getStreamData(4,i+4, file,"Inode Bitmap Block");
       blockGroup.bg_inode_table = getStreamData(4,i+8, file,"Inode Table Block");
       blockGroup.bg_free_blocks_count = getStreamData(2,i+12, file,"Free Blocks Count");
       blockGroup.bg_free_inodes_count = getStreamData(2, i+14, file, "Free Inodes Count");
       blockGroup.bg_used_dirs_count = getStreamData(2,i+16, file, "Used Directories");
       blockGroup.bg_pad = getStreamData(2,i+18,file);

       groupDescriptors->push_back(blockGroup);
    }
}

int ext2GroupDescriptor::getBlockBitmap(int group) {
    cout << "Block Bitmap Location (block#): " <<dec << groupDescriptors->at(group).bg_block_bitmap << endl;
    return groupDescriptors->at(group).bg_block_bitmap;
}

int ext2GroupDescriptor::getInodeBitmap(int group) {
    cout << "Inode Bitmap Location (block#): " <<dec << groupDescriptors->at(group).bg_inode_bitmap << endl;
    return groupDescriptors->at(group).bg_inode_bitmap;
}

int ext2GroupDescriptor::getInodeTable(int group) {
    cout << "InodeTable Location (block#): " << dec << groupDescriptors->at(group).bg_inode_table << endl;
    return groupDescriptors->at(group).bg_inode_table;
}


