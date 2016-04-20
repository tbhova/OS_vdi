#ifndef LOCALFILESYSTEM_H
#define LOCALFILESYSTEM_H

#include <QObject>
#include <QTreeView>
#include <QFileSystemModel>

namespace CSCI5806 {

class LocalFileSystem : public QObject
{
    Q_OBJECT
public:
    explicit LocalFileSystem(QTreeView *initialTree, QObject *parent = 0);
    ~LocalFileSystem();
signals:

public slots:
    void onRootPathChanged(QString newRootPath);

private:
    QTreeView *tree;
    QFileSystemModel *fileSystem;
};
}

#endif // LOCALFILESYSTEM_H
