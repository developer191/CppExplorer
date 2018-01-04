#pragma once

#include <QMenu>
#include <QAction>

#include "../browserwidget.hh"

class FolderContextMenu : public QMenu {
    Q_OBJECT
public:
    explicit FolderContextMenu(BrowserWidget *b);
    ~FolderContextMenu();
private:
    BrowserWidget *bWidget;
    QAction *openTab, *cut, *copy, *rename, *deleteFolder;
private slots:
    void onOpenTabClicked();
    void onCutClicked();
    void onCopyClicked();
    void onRenameClicked();
    void onDeleteFolderClicked();
};
