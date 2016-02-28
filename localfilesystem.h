#ifndef LOCALFILESYSTEM_H
#define LOCALFILESYSTEM_H

#include <QObject>
#include <QTreeWidget>

class LocalFileSystem : public QObject
{
    Q_OBJECT
public:
    explicit LocalFileSystem(QObject *parent = 0);
    void setTreeWidget(QTreeWidget *initialTree);

signals:

public slots:

private:
    QTreeWidget *tree;
};

#endif // LOCALFILESYSTEM_H
