#include "autenticate.h"
#include "ui_autenticate.h"
#include "lasermenu.h"
#include "QRect"
#include "QDesktopWidget"
#include <string>
#include <QMessageBox>

Autenticate::Autenticate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Autenticate)
{
    ui->setupUi(this);
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());
}

Autenticate::~Autenticate()
{
    delete ui;
}



void Autenticate::on_CancellButton_clicked()
{
    this->close();
}

void Autenticate::on_OKButton_clicked()
{
    if((ui->UsernameTextBox->text().toStdString() == "Professor")&&(ui->PasswordTextBox->text().toStdString()=="1234"))
    {
        QMessageBox::information(this,"Password","Password acepted");
        authenticated=true;
        this->close();
    }
    else QMessageBox::critical(this,"Error","Autentication error");


}
