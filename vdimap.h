#ifndef VDIMAP_H
#define VDIMAP_H

#include <QObject>

class VdiMap : public QObject
{
    Q_OBJECT
public:
    explicit VdiMap(QObject *parent = 0);

signals:

public slots:

private:
    QByteArray *map; //dyanmic array of char for the map
};

#endif // VDIMAP_H
