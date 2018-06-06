#include "installation.h"
#include "ui_installation.h"
#include "lasermenu.h"
#include "QRect"
#include "QDesktopWidget"

Installation::Installation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Installation)
{
    ui->setupUi(this);
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());
}

Installation::~Installation()
{
    delete ui;
}

void Installation::on_ExitButton_clicked()
{
    this->close();
    LaserMenu *LaserMenuWindow = new LaserMenu();
    LaserMenuWindow->show();
}
