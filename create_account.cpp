#include "create_account.h"
#include "ui_create_account.h"

create_account::create_account(const QString &title, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::create_account)
{
    ui->setupUi(this);

    this->setWindowTitle(title);

    ui->changed_title->setText(title);
    ui->error->setStyleSheet("color: red;");
    ui->error->setVisible(false);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

create_account::~create_account()
{
    delete ui;
}

void create_account::SetErrorText(const QString &text)
{
    ui->error->setText(text);
}

void create_account::SetErrorVisible(bool status)
{
    ui->error->setVisible(status);
}

void create_account::SetError(const QString &text)
{
    SetErrorText(text);
    SetErrorVisible(true);
}

QString create_account::GetUsername()
{
    return ui->username_line->text();
}

int create_account::GetPassword()
{
    return ui->password_name->text().toInt();
}