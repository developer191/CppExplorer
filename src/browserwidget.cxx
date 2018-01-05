#include <QDir>
#include <QVector>
#include <QIcon>
#include <QCursor>
#include <QDesktopServices>
#include <QMimeDatabase>

#include "browserwidget.hh"
#include "tabwidget.hh"
#include "menu/folder_contextmenu.hh"
#include "menu/file_contextmenu.hh"
#include "menu/background_contextmenu.hh"
#include "trash.hh"

BrowserWidget::BrowserWidget()
    : layout(new QVBoxLayout),
      listWidget(new ListWidget(this))
{
    layout->setContentsMargins(0,0,0,0);
    this->setLayout(layout);
    layout->addWidget(listWidget);

    defaultGridSize = listWidget->gridSize();
    setIconView();
    connect(listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(onItemDoubleClicked(QListWidgetItem*)));
    connect(listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(onItemClicked(QListWidgetItem*)));
}

void BrowserWidget::setIconView() {
    listWidget->setViewMode(QListWidget::IconMode);
    listWidget->setFlow(QListWidget::LeftToRight);
    listWidget->setWrapping(true);
    listWidget->setMovement(QListWidget::Snap);
    listWidget->setResizeMode(QListWidget::Adjust);
    listWidget->setGridSize(QSize(80,80));
    listWidget->setWordWrap(true);
}

void BrowserWidget::setListView() {
    listWidget->setViewMode(QListWidget::ListMode);
    listWidget->setFlow(QListWidget::TopToBottom);
    listWidget->setWrapping(true);
    listWidget->setMovement(QListWidget::Snap);
    listWidget->setResizeMode(QListWidget::Adjust);
    listWidget->setGridSize(defaultGridSize);
    listWidget->setWordWrap(true);
}

void BrowserWidget::loadDir(QString path, bool recordHistory, bool firstLoad) {
    if (recordHistory) {
        emit dirChanged(path);
        if (!currentPath.isEmpty()) {
            historyList.push_front(currentPath);
            emit historyChanged();
        }
    }
    currentPath = path;
    listWidget->clear();
    QDir dir(path);

    QStringList folders = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);

    QVector<QListWidgetItem *> folderItems, fileItems;

    for (int i = 0; i<folders.size(); i++) {
        QListWidgetItem *item = new QListWidgetItem(folders.at(i));
        item->setIcon(QIcon::fromTheme("folder"));
        folderItems.push_back(item);
    }

    for (int i = 0; i<files.size(); i++) {
        QListWidgetItem *item = new QListWidgetItem(files.at(i));
        QIcon defaultIcon = QIcon::fromTheme("text-plain");
        QMimeDatabase db;
        QIcon icon = QIcon::fromTheme(db.mimeTypeForFile(fsCurrentPath()+files.at(i)).iconName(),defaultIcon);
        item->setIcon(icon);
        fileItems.push_back(item);
    }

    for (int i = 0; i<folderItems.size(); i++) {
        listWidget->addItem(folderItems.at(i));
    }

    for (int i = 0; i<fileItems.size(); i++) {
        listWidget->addItem(fileItems.at(i));
    }

    if (firstLoad==false) {
        TabWidget::updateTabName();
        if (fsCurrentPath()==Trash::folderPath) {
            TabWidget::displayTrashBar(true);
        } else {
            TabWidget::displayTrashBar(false);
        }
    }
}

void BrowserWidget::loadDir(QString path, bool recordHistory) {
    loadDir(path,recordHistory,false);
}

void BrowserWidget::loadDir(QString path) {
    loadDir(path,true);
}

void BrowserWidget::loadDir() {
    loadDir(QDir::homePath());
}

QStringList BrowserWidget::history() {
    return historyList;
}

void BrowserWidget::reload() {
    QList<QListWidgetItem *> items = listWidget->selectedItems();
    QStringList oldNames;
    for (int i = 0; i<items.size(); i++) {
        oldNames.push_back(items.at(i)->text());
    }
    loadDir(currentPath,false);

    for (int i = 0; i<listWidget->count(); i++) {
        QListWidgetItem *item = listWidget->item(i);
        for (int j = 0; j<oldNames.size(); j++) {
            if (item->text()==oldNames.at(j)) {
                item->setSelected(true);
            }
        }
    }
}

QString BrowserWidget::fsCurrentPath() {
    QString path = currentPath;
    if (!path.endsWith("/")) {
        path+="/";
    }
    return path;
}

QString BrowserWidget::currentDirName() {
    QDir dir(currentPath);
    QString name = dir.dirName();
    if (name=="") {
        name = "/";
    }
    return name;
}

void BrowserWidget::startRefresh() {
    thread = new FileSystemWatcher(this);
    thread->start(1000);
}

void BrowserWidget::stopRefresh() {
    thread->stop();
}

QString BrowserWidget::currentItemName() {
    return currentItemTxt;
}

void BrowserWidget::onItemDoubleClicked(QListWidgetItem *item) {
    QString path = currentPath;
    if (!path.endsWith("/")) {
        path+="/";
    }
    path+=item->text();
    if (QFileInfo(path).isDir()) {
        loadDir(path);
        emit selectionState(false);
    } else {
        QDesktopServices::openUrl(QUrl(path));
    }
}

void BrowserWidget::onItemClicked(QListWidgetItem *item) {
    currentItemTxt = item->text();
    emit selectionState(true);
}

//ListWidget clss
//We had to create separate list widget so we could inherit the mouse press event
ListWidget::ListWidget(BrowserWidget *b) {
    bWidget = b;
}

void ListWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button()==Qt::LeftButton) {
        QListWidgetItem *item = this->itemAt(event->x(),event->y());
        if (item==nullptr) {
            QList<QListWidgetItem *> selectedItems = this->selectedItems();
            for (int i = 0; i<selectedItems.size(); i++) {
                selectedItems.at(i)->setSelected(false);
            }
            emit bWidget->selectionState(false);
        }
    } else if (event->button()==Qt::RightButton) {
        QListWidgetItem *item = this->itemAt(event->x(),event->y());
        if (item!=nullptr) {
            bWidget->currentItemTxt = item->text();
            QString complete = bWidget->fsCurrentPath()+bWidget->currentItemTxt;
            if (QFileInfo(complete).isDir()) {
                FolderContextMenu menu(bWidget);
                menu.exec(QCursor::pos());
            } else {
                FileContextMenu menu(bWidget);
                menu.exec(QCursor::pos());
            }
        } else {
            BackgroundContextMenu menu(bWidget);
            menu.exec(QCursor::pos());
        }
    }
    QListWidget::mousePressEvent(event);
}

//FileSystemWatcher class
//This is class is an extended QTimer.
//Its purpose is to refresh the BrowserWidget every second so that files/folders created outside
//the program are included in our view.
FileSystemWatcher::FileSystemWatcher(BrowserWidget *widget) {
    bWidget = widget;
    connect(this,&QTimer::timeout,this,&FileSystemWatcher::onRefresh);
}

void FileSystemWatcher::onRefresh() {
    bWidget->reload();
}
