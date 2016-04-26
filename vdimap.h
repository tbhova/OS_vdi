#ifndef VDIMAP_H
#define VDIMAP_H
#include <QVector>
#include <fstream>

#include <QObject>

namespace CSCI5806 {
class VdiMap : public QObject
{
    Q_OBJECT
public:
    explicit VdiMap(QObject *parent, long long startOffset, long long stopOffset, std::ifstream &file);
    int getMappedLocation (int location);
signals:

public slots:

private:
    QVector<unsigned int> *map; //dynamic array of char for the map
};
}

#endif // VDIMAP_H
