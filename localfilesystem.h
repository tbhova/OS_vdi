#ifndef LOCALFILESYSTEM_H
#define LOCALFILESYSTEM_H

#include <QObject>
#include <QTreeView>
#include <QFileSystemModel>

/* The gui tree model on the right side of the gui
 * Contains a fileSystem model which is similar to our vdiFileSystem class
 * The model does most of the work so there is very little code here
 * */

namespace CSCI5806 {

class LocalFileSystem : public QObject
{
    Q_OBJECT
public:
    explicit LocalFileSystem(QTreeView *initialTree, QObject *parent = 0);
    ~LocalFileSystem();
    QFileSystemModel* getFS();
signals:

public slots:
    void onRootPathChanged(QString newRootPath);

private:
    QTreeView *tree;
    QFileSystemModel *fileSystem;
};
}

#endif // LOCALFILESYSTEM_H
