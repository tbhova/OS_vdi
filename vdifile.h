#ifndef VDIFILE_H
#define VDIFILE_H

#include <QObject>
#include <QFile>
#include "vdimap.h"
#include "mbrdata.h"
#include "ext2superblock.h"
#include "ext2groupdescriptor.h"
#include <fstream>
#include <vector>
#include <cstring>
#include "ext2filesystemmanager.h"
#include <QDir>

namespace CSCI5806 {
struct VdiHeader {
long long headerSize;
long long imageType;
long long offsetBlocksSize;
long long offsetDataSize;
long long sectorSize;
long long diskSize;
long long blockSize;
long long blocksInHDD;
long long blocksAllocatedSize;
long long UUIDofVDI;
long long UUIDofSNAP;
};



class VdiFile : public QObject
{
    Q_OBJECT
public:
    VdiFile(QObject *parent = 0);
    ~VdiFile();
    void openFile(QString fileName);


public slots:
    void selectVdiPrompt();
    void transferToLocalFS(CSCI5806::ext2File *sourceFile, QDir *destDir);
    void transferToVDI(CSCI5806::ext2Folder *VDIFolder, QFileInfo *sourceFile);

signals:
    void vdiFileSelected(QString fileName);
    void FSManagerConstructed(ext2FileSystemManager *fs);
    void progressUpdate(int value);

private:
    void closeAndReset();
    void getHeaderValues();
    //In globalfunctions.h//unsigned char getCharFromStream(int size, long long seek_to, std::ifstream &input);
    void fillDataBlockBitmap(QVector<unsigned char>* DataBlockBitmap, unsigned int block_bitmap_address,unsigned int inode_bitmap_address,std::ifstream& input);
    void loadLocalFile(InodeTable* InodeTab,unsigned int size,unsigned int inodeIndexSize, std::ifstream& input, std::ofstream& localFile );
    unsigned long long singlyIndirectPointersValues(unsigned long long blockNumberOfSinglyIndirect, std::ifstream& input, std::ofstream& localFile, unsigned long long size);
    unsigned long long doublyIndirectPointersValues(unsigned long long blockNumberOfDoublyIndirect, std::ifstream& input, std::ofstream& localFile, unsigned long long size);
    unsigned long long triplyIndirectPointersValues(unsigned long long blockNumberOfTriplyIndirect, std::ifstream& input, std::ofstream& localFile, unsigned long long size);


    VdiMap *map;
    mbrData *mbr;
    ext2SuperBlock *superBlock;
    std::ifstream input;
    std::ofstream OutputFileIntoLocalFS;
    ext2GroupDescriptor *groupDescriptors;
    VdiHeader header;
    InodeTable tab;
    Inode_info InodeIn;
    QVector<unsigned char> *DataBlockBitmap;
    QVector <Inode_info> *InodeInfo;
    QVector <unsigned int> *SinglyIndirectPointers;
    QVector <unsigned int> *DoublyIndirectPointers;
    QVector <unsigned int> *TriplyIndirectPointers;
    ext2FileSystemManager *fsManager;

    //std::vector optimized bool storage to take 1 bit per boolean value when there are multiple bools
    std::vector<bool> *blockBitmap, *inodesBitmap;


    unsigned long long bootBlockLocation, superBlockLocation, block_size, group_size;
};
}

#endif // VDIFILE_H
