#include "startupval.h"
#include "ui_startupval.h"
#include "QRect"
#include "QDesktopWidget"
#include "lasermenu.h"


StartUPVal::StartUPVal(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StartUPVal)
{
    /*Get all parameters ?nm ?nm/min ?grating */
    ui->setupUi(this);
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());
}

StartUPVal::~StartUPVal()
{
    delete ui;
}

void StartUPVal::on_ExitButton_clicked()
{
    this->close();
    LaserMenu *LaserMenuWindow = new LaserMenu();
    LaserMenuWindow->show();
}

void StartUPVal::on_SaveButton_clicked()
{
    /* Save new parameters */
    this->close();
    LaserMenu *LaserMenuWindow = new LaserMenu();
    LaserMenuWindow->show();
}
