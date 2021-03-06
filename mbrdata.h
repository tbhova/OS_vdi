#ifndef MBRDATA_H
#define MBRDATA_H
#include <QVector>
#include <fstream>

#include <QObject>

/* This class holds the mbr data. We don't really do much with it.
 *
 * */

namespace CSCI5806 {
class mbrData : public QObject
{
    Q_OBJECT
public:
    explicit mbrData(QObject *parent , long long startOffset, std::fstream &file);
    int getMbrDataTable(int pos);

signals:

public slots:

private:
    unsigned char partitionTable1[16];
    unsigned char partitionTable2[16];
    unsigned char partitionTable3[16];
    unsigned char partitionTable4[16];
    long long eofMagicNumber;
    long long mbrDataTable[4];
};
}

#endif // MBRDATA_H
