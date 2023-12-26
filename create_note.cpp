#include "create_note.h"
#include "ui_create_note.h"

create_note::create_note(int curr_note, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::create_note)
{
    ui->setupUi(this);

    this->setWindowTitle("Создать запись");

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    ui->date->setDateTime(QDateTime::currentDateTime());

    ui->type->setCurrentIndex(curr_note);
}

create_note::~create_note()
{
    delete ui;
}

QString create_note::get_name() const
{
    return ui->name->text();
}

QString create_note::get_desc() const
{
    return ui->desc->toPlainText();
}

int create_note::get_type() const
{
    return ui->type->currentIndex();
}

QDateTime create_note::get_time() const
{
    return ui->date->dateTime();
}

