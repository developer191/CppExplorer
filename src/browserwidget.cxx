// Copyright 2018 Patrick Flynn
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//	this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this
//	list of conditions and the following disclaimer in the documentation and/or
//	other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may
//	be used to endorse or promote products derived from this software
//	without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include <QDir>
#include <QVector>
#include <QIcon>
#include <QCursor>
#include <QDesktopServices>
#include <QImageReader>
#include <QPixmapCache>
#include <QPixmap>
#include <QMimeDatabase>
#ifdef _WIN32
    //Windows: use the registry
#else
#include <cpplib/settings.hh>
#endif
#include <iostream>

#include "browserwidget.hh"
#include "tabwidget.hh"
#include "menu/folder_contextmenu.hh"
#include "menu/file_contextmenu.hh"
#include "menu/background_contextmenu.hh"
#include "menu/multi_contextmenu.hh"
#include "trash.hh"

#ifndef _WIN32
using namespace CppLib;
#endif

BrowserWidget::BrowserWidget()
    : layout(new QVBoxLayout),
      listWidget(new ListWidget(this))
{
    layout->setContentsMargins(0,0,0,0);
    this->setLayout(layout);

    trashbar = new TrashBar;
    trashbar->setBrowserWidget(this);
    trashbar->hide();

    searchbar = new SearchBar(this);
    searchbar->hide();

    layout->addWidget(trashbar,0,Qt::AlignTop);
    layout->addWidget(searchbar,0,Qt::AlignTop);
    layout->addWidget(listWidget);

    defaultGridSize = listWidget->gridSize();
    setIconView();
    connect(listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(onItemDoubleClicked(QListWidgetItem*)));
    connect(listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(onItemClicked(QListWidgetItem*)));
}

BrowserWidget::~BrowserWidget() {
    delete layout;
    delete listWidget;
    delete trashbar;
    delete searchbar;
    delete thread;
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
        //emit dirChanged(path);
        if (!currentPath.isEmpty()) {
            historyList.push_front(currentPath);
            emit historyChanged();
        }
        emit dirChanged(path);
    }
    currentPath = path;
    listWidget->clear();
    QDir dir(path);

    QStringList folders, files;

#ifdef _WIN32
    //Windows: use the registry
    bool hidden = false;
#else
    bool hidden = QVariant(Settings::getSetting("view/hidden","false")).toBool();
#endif
    if (hidden) {
        folders = dir.entryList(searchPatterns,QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden, QDir::Name | QDir::IgnoreCase);
        files = dir.entryList(searchPatterns,QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden, QDir::Name | QDir::IgnoreCase);
    } else {
        folders = dir.entryList(searchPatterns,QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
        files = dir.entryList(searchPatterns,QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
    }

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
        if (!QImageReader(fsCurrentPath()+files.at(i)).format().isEmpty()) {
            QString key = fsCurrentPath()+files.at(i);
            QPixmap pm;
            if (!QPixmapCache::find(key,&pm)) {
                pm.load(key);
                QPixmapCache::insert(key,pm);
            }
            if (pm.isNull()) {
                item->setIcon(defaultIcon);
            } else {
                item->setIcon(pm);
            }
        } else {
            QIcon icon = QIcon::fromTheme(db.mimeTypeForFile(fsCurrentPath()+files.at(i)).iconName(),defaultIcon);
            item->setIcon(icon);
        }
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
    }

    if (fsCurrentPath()==Trash::folderPath) {
        trashbar->show();
    } else {
        trashbar->hide();
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
    //Get all currently selected items so we can re-select them
    QList<QListWidgetItem *> items = listWidget->selectedItems();
    QStringList oldNames;
    for (int i = 0; i<items.size(); i++) {
        oldNames.push_back(items.at(i)->text());
    }

    //Get all items currently in the view
    QStringList currentItems;
    for (int i = 0; i<listWidget->count(); i++) {
        QListWidgetItem *item = listWidget->item(i);
        currentItems.push_back(item->text());
    }

    //Load items in the current directory
    bool hidden = QVariant(Settings::getSetting("view/hidden","false")).toBool();
    QStringList dirItems;
    QDir dir(fsCurrentPath());
    if (hidden) {
        dirItems = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    } else {
        dirItems = dir.entryList(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    }

    //Check to see if there are any items in the directory stringlist that are not
    //in the current view list
    bool found = false;
    for (int i = 0; i<dirItems.size(); i++) {
        if (!currentItems.contains(dirItems.at(i))) {
            found = true;
            break;
        }
    }
    if (found==false) {
        for (int i = 0; i<currentItems.size(); i++) {
            if (!dirItems.contains(currentItems.at(i))) {
                found = true;
                break;
            }
        }
    }

    //If found, reload
    if (found) {
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

QStringList BrowserWidget::currentItemNames() {
    QStringList list;
    auto selected = listWidget->selectedItems();
    for (int i = 0; i<selected.size(); i++) {
        list.push_back(selected.at(i)->text());
    }
    return list;
}

void BrowserWidget::selectAll() {
    listWidget->selectAll();
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
    this->setDragEnabled(false);
    this->setSelectionMode(QListWidget::ExtendedSelection);

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
            if (this->selectedItems().size()>1) {
                MultiContextMenu menu(bWidget);
                menu.exec(QCursor::pos());
            } else {
                bWidget->currentItemTxt = item->text();
                QString complete = bWidget->fsCurrentPath()+bWidget->currentItemTxt;
                if (QFileInfo(complete).isDir()) {
                    FolderContextMenu menu(bWidget);
                    menu.exec(QCursor::pos());
                } else {
                    FileContextMenu menu(bWidget);
                    menu.exec(QCursor::pos());
                }
            }
        } else {
            BackgroundContextMenu menu(bWidget);
            menu.exec(QCursor::pos());
        }
    }
    QListWidget::mousePressEvent(event);
}

void ListWidget::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers()==Qt::ControlModifier) {
        if (event->key()==Qt::Key_S) {
            if (bWidget->searchbar->isVisible()) {
                bWidget->searchbar->hide();
            } else {
                bWidget->searchbar->show();
            }
        }
    }
    QListWidget::keyPressEvent(event);
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
