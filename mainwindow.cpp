#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "create_note.h"
#include "create_account.h"

#include <QFileDialog>
#include <QTextStream>
#include <QTimer>

#include <iostream>
#include <functional>

void MainWindow::MakeListItem(const note_data &data)
{
    // создание виджета записи
    note* wgt = new note();

    wgt->set_title(data.name);
    wgt->set_desc(data.desc);
    wgt->set_time(data.time);

    QListWidget* list;

    switch (data.type)
    {
        case 0:
        {
            list = ui->workout_list;
            break;
        }
        case 1:
        {
            list = ui->food_list;
            break;
        }
    }
    // настройка виджета
    QDateTime current_time = QDateTime::currentDateTime();

    QListWidgetItem* item = new QListWidgetItem(list);
    QColor color = current_time < data.qtime ? QColor(Qt::GlobalColor::green) : QColor(Qt::GlobalColor::red);

    item->setBackgroundColor(color);
    item->setSizeHint(wgt->geometry().size());

    // добавление виджета в список
    list->setItemWidget(item, wgt);
    note_list.push_back(std::pair<QListWidgetItem*, note_data>(item, data));
}

void MainWindow::CreateNoteDialog()
{
    // создание окна создания записи
    create_note* dialog = new create_note(ui->tabWidget->currentIndex());

    QTimer::singleShot(QAPP_DEFAULT_WIN_TIMEOUT, dialog, SLOT(close()));

    dialog->show();
    // ожидание завершения работы пользователя
    if (dialog->exec() != QDialog::Accepted)
    {
        _debug() << "dialog not accepted, return";
        return;
    }

    note_data data(dialog->get_name(),
                   dialog->get_desc(),
                   dialog->get_time(),
                   dialog->get_type());
    // добавление записи в список
    MakeListItem(data);

    return;
}

void MainWindow::DeleteNoteDialog(QListWidget* list)
{
    QListWidgetItem* item = list->currentItem();

    if (item == nullptr)
    {
        _debug() << "ptr is null";
        return;
    }

    note_list.remove_if([item](const std::pair<QListWidgetItem*, note_data> &pair){ return pair.first == item; });

    list->removeItemWidget(item);
    delete item;
}

void MainWindow::DeleteNoteWorkout()
{
    DeleteNoteDialog(ui->workout_list);
}

void MainWindow::DeleteNoteFood()
{
    DeleteNoteDialog(ui->food_list);
}

void MainWindow::CreateAccount()
{
    create_account* dialog = new create_account();

    dialog->show();

    if (dialog->exec() != QDialog::Accepted)
    {
        _debug() << "dialog not accepted, return";
        return;
    }

    if (base == nullptr)
    {
        _debug() << "databse not loaded, return";
        return;
    }

    QSqlQuery query(*base);

    QString entered_username = dialog->GetUsername();
    int entered_password     = dialog->GetPassword();

    if (entered_password == 0 ||
        entered_username == "")
    {
        _debug() << "incorrect login or password";
        return;
    }

    _debug() << "username: " << entered_username << ", password: " << entered_password;

    query.prepare("INSERT INTO " QAPP_DEFAULT_DB_TABLE_NAME "(" QAPP_DEFAULT_FIELD_LOGIN_NAME ", " QAPP_DEFAULT_FIELD_PASSWORD_NAME ")"
                   "VALUES (:username, :password);");

    query.bindValue(":username", entered_username);
    query.bindValue(":password", entered_password);

    query.exec();

    if (!query.isActive())
    {
        _debug() << "sql error: " << query.lastError();
        return;
    }

    ui->login_title->setText("Вы вошли как: " + entered_username);
}

void MainWindow::LoginAccount()
{
    // создание диалогового окна
    create_account* dialog = new create_account("Войти в аккаунт");

    if (base == nullptr)
    {
        _debug() << "database not loaded";
        dialog->SetError("База данных не подключена");
    }

    dialog->show();

    if (dialog->exec() != QDialog::Accepted)
    {
        _debug() << "dialog not accepted, return";
        return;
    }

    if (base == nullptr)
    {
        _debug() << "database not loaded";
        return;
    }

    QSqlQuery query(*base);

    QString entered_username = dialog->GetUsername();
    int entered_password     = dialog->GetPassword();

    if (entered_password == 0 ||
        entered_username == "")
    {
        _debug() << "incorrect login or password";
        return;
    }

    _debug() << "username: " << entered_username << ", password: " << entered_password;

    query.prepare("SELECT * FROM " QAPP_DEFAULT_DB_TABLE_NAME " WHERE "
                  QAPP_DEFAULT_FIELD_LOGIN_NAME " = :username" );

    query.bindValue(":username", entered_username);

    query.exec();

    if (!query.first())
    {
        _debug() << "query.first() failed";
        return;
    }

    if (!query.isActive())
    {
        _debug() << "sql error: " << query.lastError();
        return;
    }

    QString database_username = query.value(0).toString();
    int database_password     = query.value(1).toInt();

    _debug() << "username: " << database_username << ", password: " << database_password;

    if (database_password != entered_password ||
        database_username != entered_username)
    {
        _debug() << "Incorrect login or password";
        return;
    }

    ui->login_title->setText("Вы вошли как: " + database_username);
}

void MainWindow::ExitAccount()
{
    ui->login_title->setText("Вы не вошли в аккаунт");
}

void MainWindow::SaveSession(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly))
    {
        _debug() << filename << " open failed";
        return;
    }

    for (auto item = note_list.cbegin(); item != note_list.cend(); item++)
    {
        item->second.SaveToFile(file);
    }
}

void MainWindow::LoadSession(QString filename)
{
    ui->workout_list->clear();
    ui->food_list->clear();
    note_list.clear();

    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
    {
        _debug() << filename << " open failed";
        return;
    }

    while(!file.atEnd())
    {
        note_data data;

        if (data.LoadFromFile(file))
        {
            _debug() << "data.LoadFromFile failed";
            continue;
        }

        MakeListItem(data);
    }
}

void MainWindow::BuiltinFileHandle(void(MainWindow::*handler)(QString))
{
    QString filename = QFileDialog::getOpenFileName();

    if (filename.isEmpty())
    {
        _debug() << "filename is empty";
        return;
    }

    (this->*handler)(filename);
}

void MainWindow::SaveToFile()
{
    BuiltinFileHandle(&MainWindow::SaveSession);
}

void MainWindow::LoadFromFile()
{
    BuiltinFileHandle(&MainWindow::LoadSession);
}

void MainWindow::DataBasePrepare(QString name)
{
    base = new QSqlDatabase(QSqlDatabase::addDatabase(QAPP_DEFAULT_DB_TYPE));

    base->setDatabaseName(name);

    if (!base->open())
    {
        _debug() << "failed to open database";
        return;
    }

    QSqlQuery query(*base);

    query.exec("CREATE TABLE " QAPP_DEFAULT_DB_TABLE_NAME "("
               QAPP_DEFAULT_FIELD_LOGIN_NAME " TEXT, "
               QAPP_DEFAULT_FIELD_PASSWORD_NAME " INT);");

    if (!query.isActive())
    {
        switch (query.lastError().number())
        {
            case 1:
            {
                _debug() << "Database is exist, skip";
                break;
            }
            default:
            {
                _debug() << query.lastError().text();
                break;
            }
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Дневник тренировок и питания");

    ui->tabWidget->setCurrentIndex(0);
    // загрузка последней сессии
    LoadSession(QAPP_LAST_SESSION_FILENAME);
    // загрузка базы данных
    DataBasePrepare(QAPP_DEFAULT_DB_NAME);
    ExitAccount();
    // подключение кнопок
    connect(ui->create_button_workout, SIGNAL(clicked()), this, SLOT(CreateNoteDialog()));
    connect(ui->create_button_food,    SIGNAL(clicked()), this, SLOT(CreateNoteDialog()));
    connect(ui->delete_button_workput, SIGNAL(clicked()), this, SLOT(DeleteNoteWorkout()));
    connect(ui->delete_button_food,    SIGNAL(clicked()), this, SLOT(DeleteNoteFood()));

    connect(ui->account_create, SIGNAL(triggered()), this, SLOT(CreateAccount()));
    connect(ui->account_login,  SIGNAL(triggered()), this, SLOT(LoginAccount()));
    connect(ui->account_exit,   SIGNAL(triggered()), this, SLOT(ExitAccount()));
    connect(ui->save,           SIGNAL(triggered()), this, SLOT(SaveToFile()));
    connect(ui->load,           SIGNAL(triggered()), this, SLOT(LoadFromFile()));
}

MainWindow::~MainWindow()
{
    SaveSession(QAPP_LAST_SESSION_FILENAME);

    delete ui;
}

