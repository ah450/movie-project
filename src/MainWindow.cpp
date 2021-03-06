/* Copyright (C) 2014 Movie-Project
 * (Olivier CHURLAUD, Sébastien TOUZÉ)
 *
 * This file is part of Movie-Project.
 *
 * Movie-Project is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Movie-Project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Movie-Project.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    m_app = qobject_cast<Application *>(qApp);
    m_app->debug("[MainWindow] Constructor called");

    m_ui->setupUi(this);
    this->setWindowTitle(m_app->getAppName());
    this->setWindowIcon(m_app->getAppIcon());
    m_ui->metadataTop->setWordWrap(true);

    connect(m_ui->mainPannel, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(on_customContextMenuRequested(const QPoint &)));
    connect(m_ui->leftPannel, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(on_customContextMenuRequested(const QPoint &)));
    connect(this, SIGNAL(toUpdate()), this, SLOT(selfUpdate()));

    m_moviesList = m_app->getDatabaseManager()->getAllMovies();
    m_leftPannelSelectedId = 0;
    fillMainPannel();
    fillLeftPannel(isPeople, Director);
    m_app->debug("[MainWindow] Construction done");
}

MainWindow::~MainWindow()
{
    delete m_ui;
    m_app->debug("[MainWindow] Destructed");
}

/**
 * @brief Call and shows the settings window.
 * Calls the SettingsWindow class and waits until it closes to handle the results
 */
void MainWindow::on_actionEdit_Settings_triggered()
{
    m_app->debug("[MainWindow] Enters showSettingsWindow()");
    SettingsWindow *l_settingsWindow = new SettingsWindow;
    l_settingsWindow->show();
    QObject::connect(l_settingsWindow,SIGNAL(closeAndSave()), this, SLOT(addNewMovies()));
    m_app->debug("[MainWindow] Exits showSettingsWindow()");
}

void MainWindow::fillLeftPannel(int typeElement, int typePeople = 0)
{
    m_ui->leftPannel->clear();
    m_typeElement = typeElement;
    m_typePeople = typePeople;

    if (typeElement != isPlaylist)
    {
        QListWidgetItem *l_item = new QListWidgetItem("All");
        l_item->setData(Qt::UserRole, 0);
        m_ui->leftPannel->addItem(l_item);
        if (m_leftPannelSelectedId == 0)
        {
            l_item->setSelected(true);
        }
    }

    if (typeElement == isPeople)
    {
        QListWidgetItem *l_item = new QListWidgetItem("Unknown");
        l_item->setData(Qt::UserRole, -1);
        l_item->setData(Qt::UserRole+1, isPeople);
        m_ui->leftPannel->addItem(l_item);
        if (m_leftPannelSelectedId == -1)
        {
            l_item->setSelected(true);
        }

        QList<People> l_peopleList = m_app->getDatabaseManager()->getPeopleByType(typePeople);
        foreach (People l_people, l_peopleList)
        {
            QString l_name(l_people.getLastname());
            if (l_people.getFirstname() != "")
            {
                l_name = l_name + ", " + l_people.getFirstname();
            }
            l_name = l_name  + " (" + QString::number(l_people.getBirthday().year()) + ")";

            QListWidgetItem *l_item = new QListWidgetItem(l_name);
            l_item->setData(Qt::UserRole, l_people.getId());
            l_item->setData(Qt::UserRole+1, isPeople);
            l_item->setData(Qt::UserRole+2, typePeople);
            if (m_leftPannelSelectedId == l_people.getId())
            {
                l_item->setSelected(true);
            }

            m_ui->leftPannel->addItem(l_item);
        }
    }
    else if(typeElement == isTag)
    {
        QListWidgetItem *l_item = new QListWidgetItem("No Tag");
        l_item->setData(Qt::UserRole, -1);
        l_item->setData(Qt::UserRole+1, isTag);
        m_ui->leftPannel->addItem(l_item);
        if (m_leftPannelSelectedId == -1)
        {
            l_item->setSelected(true);
        }

        QList<Tag> l_tagsList = m_app->getDatabaseManager()->getAllTags();
        foreach (Tag l_tag, l_tagsList)
        {
            QString l_name(l_tag.getName());

            QListWidgetItem *l_item = new QListWidgetItem(l_name);
            l_item->setData(Qt::UserRole, l_tag.getId());
            l_item->setData(Qt::UserRole+1, isTag);

            m_ui->leftPannel->addItem(l_item);
            if (m_leftPannelSelectedId == l_tag.getId())
            {
                l_item->setSelected(true);
            }
        }
    }
    else if(typeElement == isPlaylist)
    {
        QListWidgetItem *l_item = new QListWidgetItem("To Watch");
        l_item->setData(Qt::UserRole, 1);
        l_item->setData(Qt::UserRole+1, isPlaylist);
        m_ui->leftPannel->addItem(l_item);
        if (m_leftPannelSelectedId == 1)
        {
            l_item->setSelected(true);
        }

        QList<Playlist> l_playlistList = m_app->getDatabaseManager()->getAllPlaylists();
        foreach (Playlist l_playlist, l_playlistList)
        {
            QString l_name(l_playlist.getName());

            QListWidgetItem *l_item = new QListWidgetItem(l_name);
            l_item->setData(Qt::UserRole, l_playlist.getId());
            l_item->setData(Qt::UserRole+1, isPlaylist);

            m_ui->leftPannel->addItem(l_item);
            if (m_leftPannelSelectedId == l_playlist.getId())
            {
                l_item->setSelected(true);
            }
        }
    }
    this->setLeftPannelLabel();
}

void MainWindow::fillMainPannel()
{
    m_app->debug("[MainWindow] Enters fillMainPannel()");

    m_ui->mainPannel->clear();
    m_ui->mainPannel->setColumnCount(4);

    QStringList l_headers;
    l_headers << "Title" << "Original Title" << "Release Date" << "Path of the file";
    m_ui->mainPannel->setHorizontalHeaderLabels(l_headers);
    m_ui->mainPannel->setRowCount(m_moviesList.size());
    int l_row = 0;

    foreach (Movie l_movie, m_moviesList)
    {
        int l_column = 0;
        QStringList l_movieData;
        l_movieData << l_movie.getTitle()
                    << l_movie.getOriginalTitle()
                    << l_movie.getReleaseDate().toString("dd MMM yyyy")
                    << l_movie.getFilePath();
        QVector<QTableWidgetItem*> l_itemList(4);
        foreach(QTableWidgetItem *l_item, l_itemList)
        {
            l_item = new QTableWidgetItem(l_movieData.at(l_column));
            l_item->setData(Qt::UserRole, l_movie.getId());
            l_item->setData(Qt::UserRole+1, isMovie);
            m_ui->mainPannel->setItem(l_row, l_column, l_item);
            l_column++;
        }
        l_row++;
    }

    m_app->debug("[MainWindow] Exits fillMainPannel()");
}

void MainWindow::on_peopleBox_activated(int type)
{
    m_app->debug("[MainWindow] Enters on_peopleBox_activated()");

    int peopleType = type + 1;
    fillLeftPannel(isPeople, peopleType);
    m_leftPannelSelectedId = 0;
}

void MainWindow::on_playlistsButton_clicked()
{
    m_app->debug("[MainWindow] playlistsButton clicked");
    fillLeftPannel(isPlaylist);
    m_leftPannelSelectedId = 0;
}

void MainWindow::on_toWatchButton_clicked()
{

}

void MainWindow::on_tagsButton_clicked()
{
    m_app->debug("[MainWindow] tagsButton clicked");
    fillLeftPannel(isTag);
    m_leftPannelSelectedId = 0;
}

void MainWindow::on_customContextMenuRequested(const QPoint &point)
{
    m_app->debug("[MainWindow] customContextMenuRequested()");
    QMenu *l_menu = new QMenu(this);

    // The left pannel must have focus, one item selected which id is not 0
    // (not to be "All" or "Unknown")
    if(m_ui->leftPannel->hasFocus()
            && m_ui->leftPannel->selectedItems().count() != 0
            && m_ui->leftPannel->selectedItems().at(0)->data(Qt::UserRole) != 0)
    {
        l_menu->addAction(m_ui->actionEdit_leftPannelMetadata);
        l_menu->exec(m_ui->leftPannel->mapToGlobal(point));
    }
    else if (m_ui->mainPannel->hasFocus() &&
             m_ui->mainPannel->selectedItems().count() != 0)
    {
        QMenu *l_addPlaylistMenu = new QMenu("Add to playlist");
        QAction *l_actionAddInNewPlaylist = new QAction("New playlist",
                                                        l_addPlaylistMenu);
        l_actionAddInNewPlaylist->setData(-1);
        QAction *l_actionAddInToWatch = new QAction("To Watch",
                                                    l_addPlaylistMenu);
        l_actionAddInToWatch->setData(1);

        QList<Playlist> l_playlistList = m_app->getDatabaseManager()->getAllPlaylists();
        foreach (Playlist l_playlist, l_playlistList)
        {
            QAction *l_actionAddInPlaylist = new QAction(l_playlist.getName(),
                                                         l_addPlaylistMenu);
            l_actionAddInPlaylist->setData(l_playlist.getId());
            l_addPlaylistMenu->addAction(l_actionAddInPlaylist);
        }
        if (l_playlistList.count() != 0)
        {
            l_addPlaylistMenu->addSeparator();
        }

        l_addPlaylistMenu->addAction(l_actionAddInToWatch);
        l_addPlaylistMenu->addSeparator();
        l_addPlaylistMenu->addAction(l_actionAddInNewPlaylist);
        QObject::connect(l_addPlaylistMenu, SIGNAL(triggered(QAction*)),
                         this, SLOT(addPlaylistMenu_triggered(QAction*)));
        l_menu->addMenu(l_addPlaylistMenu);
        l_menu->addAction(m_ui->actionEdit_mainPannelMetadata);
        l_menu->exec(m_ui->mainPannel->mapToGlobal(point));
    }
}

void MainWindow::on_actionEdit_mainPannelMetadata_triggered()
{
    m_app->debug("[MainWindow] actionEdit_Metadata_triggered()");
    int l_id = m_ui->mainPannel->selectedItems().at(0)->data(Qt::UserRole).toInt();

    MovieDialog *l_movieDialog = new MovieDialog(l_id);
    connect(l_movieDialog, SIGNAL(destroyed()), this, SLOT(selfUpdate()));
    l_movieDialog->show();
}

void MainWindow::on_actionEdit_leftPannelMetadata_triggered()
{
    int l_id = m_ui->leftPannel->selectedItems().at(0)->data(Qt::UserRole).toInt();

    // It's editable only if id is not 0
    if(l_id != 0)
    {
        int l_typeElement = m_ui->leftPannel->selectedItems().at(0)->data(Qt::UserRole+1).toInt();
        if (l_typeElement == isPeople)
        {
            PeopleDialog *l_movieDialog = new PeopleDialog(l_id);
            connect(l_movieDialog, SIGNAL(destroyed()), this, SLOT(selfUpdate()));
            l_movieDialog->show();
        }
        else if (l_typeElement == isTag)
        {
            qDebug() << "Tag !";
        }
        else if (l_typeElement == isPlaylist)
        {
            qDebug() << "Playlist !";
        }
    }
}

void MainWindow::addPlaylistMenu_triggered(QAction* action)
{
    int l_actionId = action->data().toInt();
    int l_movieId = m_ui->mainPannel->selectedItems().at(0)->data(Qt::UserRole).toInt();
    Movie l_movie = m_app->getDatabaseManager()->getOneMovieById(l_movieId);

    if (l_actionId == -1)
    {
        bool l_isNewPlaylist;
        QString l_playlistName = QInputDialog::getText(this, "New Playlist", "Name of the new Playlist", QLineEdit::Normal, QString(), &l_isNewPlaylist);
        if (l_isNewPlaylist && !l_playlistName.isEmpty())
        {
            Playlist l_playlist(l_playlistName);
            m_app->getDatabaseManager()->insertNewPlaylist(l_playlist);
            l_playlist.addMovie(l_movie);
            m_app->getDatabaseManager()->updatePlaylist(l_playlist);
        }
    }
    else
    {
        Playlist l_playlist = m_app->getDatabaseManager()->getOnePlaylistById(l_actionId);
        l_playlist.addMovie(l_movie);
        m_app->getDatabaseManager()->updatePlaylist(l_playlist);
    }
    emit(toUpdate());
}

void MainWindow::on_mainPannel_itemDoubleClicked(QTableWidgetItem *item)
{
    m_app->debug("[MainWindow] itemDoubleClicked on mainPannel()");

    int l_movieId = item->data(Qt::UserRole).toInt();
    Movie l_movie = m_app->getDatabaseManager()->getOneMovieById(l_movieId);

    m_app->debug("[MainWindow.startMovie()] Opened movie: " + l_movie.getFilePath());

    QDesktopServices::openUrl(QUrl("file://" + l_movie.getFilePath(), QUrl::TolerantMode));
}

void MainWindow::on_leftPannel_clicked(const QModelIndex &item)
{
    m_app->debug("[MainWindow] itemClicked on leftPannel()");
    m_leftPannelSelectedId = item.data(Qt::UserRole).toInt();
    this->prepareMoviesToDisplay(m_leftPannelSelectedId);
}

void MainWindow::on_mainPannel_clicked(const QModelIndex &index)
{
    m_app->debug("[MainWindow] mainPannel clicked");
    int l_idMovie = index.data(Qt::UserRole).toInt();
    Movie l_movie = m_app->getDatabaseManager()->getOneMovieById(l_idMovie);
    this->fillMetadataPannel(l_movie);

}

void MainWindow::prepareMoviesToDisplay(int id)
{
    m_app->debug("[MainWindow] prepareMoviesToDisplay()");

    m_leftPannelSelectedId = id;
    if(m_leftPannelSelectedId == 0)
    {
        m_moviesList = m_app->getDatabaseManager()->getAllMovies();
    }
    else if(m_typeElement == isPeople)
    {
        if (m_leftPannelSelectedId == -1)
        {
            m_moviesList = m_app->getDatabaseManager()->getMoviesWithoutPeople(m_typePeople);
        }
        else
        {
            m_moviesList = m_app->getDatabaseManager()->getMoviesByPeople(m_leftPannelSelectedId, m_typePeople);
        }
    }
    else if (m_typeElement == isTag)
    {
        if (m_leftPannelSelectedId == -1)
        {
            m_moviesList = m_app->getDatabaseManager()->getMoviesWithoutTag();
        }
        else
        {
            m_moviesList = m_app->getDatabaseManager()->getMoviesByTag(m_leftPannelSelectedId);
        }
    }
    else if (m_typeElement == isPlaylist)
    {
        Playlist l_playlist = m_app->getDatabaseManager()->getOnePlaylistById(m_leftPannelSelectedId);
        m_moviesList = l_playlist.getMovieList();
    }
    filterPannels();
}

void MainWindow::selfUpdate()
{
    m_app->debug("[MainWindow] selfUpdate()");
    m_moviesList.clear();
    fillLeftPannel(m_typeElement, m_typePeople);

    for (int i = 0 ; i < m_ui->leftPannel->count() ; i++)
    {
        QListWidgetItem *l_item = m_ui->leftPannel->item(i);
        if (l_item->data(Qt::UserRole).toInt() == m_leftPannelSelectedId)
        {

            l_item->setSelected(true);
            this->prepareMoviesToDisplay(l_item->data(Qt::UserRole).toInt());
            break;
        }
    }
}

void MainWindow::setLeftPannelLabel()
{
    switch (m_typeElement)
    {
    case isTag:
        m_ui->leftPannelLabel->setText("Tags");
        break;
    case isPlaylist:
        m_ui->leftPannelLabel->setText("Playlists");
        break;
    case isPeople:
        switch (m_typePeople)
        {
        case Director:
            m_ui->leftPannelLabel->setText("Directors");
            break;
        case Producer:
            m_ui->leftPannelLabel->setText("Producers");
            break;
        case Actor:
            m_ui->leftPannelLabel->setText("Actors");
            break;
        }
    }
}

void MainWindow::on_searchEdit_returnPressed()
{
    m_app->debug("[MainWindow] editing finished on searchEdit");

    QString l_text = m_ui->searchEdit->text();
    filterPannels();
}

void MainWindow::filterPannels()
{
    m_app->debug("[MainWindow] Enters filterPeople()");

    fillMainPannel();
    fillLeftPannel(m_typeElement, m_typePeople);

    QString l_text = m_ui->searchEdit->text();

    if (l_text != "")
    {
        // mainPannel
        QList<Movie> l_authorizedMoviesList = m_app->getDatabaseManager()->getMoviesByAny(l_text);
        for (int i = 0 ; i < m_ui->mainPannel->rowCount(); i++)
        {
            bool l_isPresent(false);
            QTableWidgetItem *l_item = m_ui->mainPannel->item(i, 0);
            foreach(Movie l_movie, l_authorizedMoviesList)
            {
                 if (l_movie.getId() == l_item->data(Qt::UserRole))
                 {
                     l_isPresent = true;
                 }
            }
            if (l_isPresent == false)
            {
                m_ui->mainPannel->removeRow(i);
                i--; // We remove a line, so we have to decrement i...
            }
        }

        // leftPannel
        if (m_typeElement == isTag)
        {
            QList<Tag> l_authorizedTagList = m_app->getDatabaseManager()->getTagsByAny(l_text);
            for (int i = 2 ; i < m_ui->leftPannel->count(); i++)
            {
                bool l_isPresent(false);
                QListWidgetItem *l_item = m_ui->leftPannel->item(i);
                foreach(Tag l_tag, l_authorizedTagList)
                {
                     if (l_tag.getId() == l_item->data(Qt::UserRole))
                     {
                         l_isPresent = true;
                     }
                }
                if (l_isPresent == false)
                {
                    delete l_item;
                    i--; // We remove an item so we have to decrement i
                }
            }
        }
        else if (m_typeElement == isPeople)
        {
            QList<People> l_authorizedPeopleList = m_app->getDatabaseManager()->getPeopleByAny(l_text, m_typePeople);

            // We begin at 2 because the first item is "All", second is "Unknown"
            for (int i = 2 ; i < m_ui->leftPannel->count() ; i++)
            {
                bool l_isPresent(false);
                QListWidgetItem *l_item = m_ui->leftPannel->item(i);
                foreach(People l_people, l_authorizedPeopleList)
                {
                     if (l_people.getId() == l_item->data(Qt::UserRole))
                     {
                         l_isPresent = true;
                     }
                }
                if (l_isPresent == false)
                {
                    delete l_item;
                    i--; // We remove an item so we have to decrement i
                }
            }
        }
        else if (m_typeElement == isPlaylist)
        {
            // Don't do anything here, we want to see the name of the playlists
            // also when empty
        }
    }
    for (int i = 0 ; i < m_ui->leftPannel->count() ; i ++)
    {
        QListWidgetItem *l_item = m_ui->leftPannel->item(i);
        if (m_leftPannelSelectedId == l_item->data(Qt::UserRole))
        {
            l_item->setSelected(true);
        }
    }
}

void MainWindow::addNewMovies()
{
    m_app->debug("[MainWindow] Enter addNewMovies");

    QString l_directoryName = m_app->getFilesPath();
    QDirIterator l_path(l_directoryName, QDir::NoDotAndDotDot | QDir::Files,QDirIterator::Subdirectories);
    while (l_path.hasNext())
    {
        l_path.next();
        QString l_filePath = l_path.fileInfo().absoluteFilePath();
        QString l_fileSuffix = l_path.fileInfo().suffix();
        if (l_fileSuffix == "mkv" || l_fileSuffix == "avi" || l_fileSuffix == "mp4" || l_fileSuffix == "mpg" || l_fileSuffix == "flv" || l_fileSuffix == "mov")
        {
            m_app->debug("[MainWindow.updateApp()] Suffix accepted");
            bool l_movieExists = m_app->getDatabaseManager()->existMovie(l_filePath);
            if(!l_movieExists)
            {
                m_app->debug("[MainWindow.updateApp()] Movie not already known");
                Movie l_movie;
                l_movie.setTitle(l_path.fileInfo().completeBaseName());
                l_movie.setFilePath(l_path.fileInfo().absoluteFilePath());
                l_movie.setSuffix(l_fileSuffix);
                m_app->getDatabaseManager()->insertNewMovie(l_movie);

                MetadataFetcher l_metadataFetcher(l_movie);

                // Currently a wainting loop is created in MetadataFetcher,
                // multithreading would be better
                l_metadataFetcher.fetchMetadata(l_movie.getTitle());
            }
            else
            {
                m_app->debug("[MainWindow.updateApp()] Movie already known. Skipped");
            }
        }
    }

    emit(toUpdate());
    m_app->debug("[MainWindow] Exit addNewMovies");
}

void MainWindow::fillMetadataPannel(Movie movie)
{
    m_app->debug("[MainWindow] Enter fillMetadataPannel");

    QString l_title = "<html>"+movie.getTitle()+"<br />";
    QString l_originalTitle = "<i>"+movie.getOriginalTitle()+"</i></br /><br />";
    QString l_directors = "<i>Directed by</i><br />";
    QString l_producers = "<i>Produced by</i><br />";
    QString l_actors = "<i>With</i><br />";

    foreach (People l_people, movie.getPeopleList())
    {
        switch (l_people.getType())
        {
        case Director:
            if (l_people.getFirstname() != "")
            {
                l_directors += l_people.getFirstname() + " ";
            }
            l_directors += l_people.getLastname() + ", ";
            break;
        case Producer:
            if (l_people.getFirstname() != "")
            {
                l_producers += l_people.getFirstname() + " ";
            }
            l_producers += l_people.getLastname() + ", ";
            break;
        case Actor:
            if(l_people.getFirstname() != "")
            {
                l_actors += l_people.getFirstname() + " ";
            }
            l_actors += l_people.getLastname() + ", ";
            break;
        }
    }
    // Need to remove ", " = 2 chars
    if (l_directors.right(2) == ", ")
    {
        l_directors = l_directors.remove(l_directors.size(),-2);
    }
    l_directors += "<br /><br />";
    if (l_producers.right(2) == ", ")
    {
        l_producers = l_producers.remove(l_producers.size(),-2);
    }
    l_producers += "<br /><br />";
    if (l_actors.right(2) == ", ")
    {
        l_actors = l_actors.remove(l_actors.size(),-2);
    }
    l_actors += "<br /><br />";
    l_actors += "</html>";

    m_ui->metadataTop->setText(l_title+l_originalTitle + l_directors +l_producers + l_actors);
    m_ui->metadataPlot->setText(movie.getSynopsis());
    m_app->debug("[MainWindow] Exit fillMetadataPannel");
}
