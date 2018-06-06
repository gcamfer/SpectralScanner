#include "optionsmenu.h"
#include "ui_optionsmenu.h"
#include "lasermenu.h"
#include "autenticate.h"
#include "calibrationvalues.h"
#include "startupval.h"
#include "QRect"
#include "QDesktopWidget"

OptionsMenu::OptionsMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionsMenu)
{
    ui->setupUi(this);
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());

}

OptionsMenu::~OptionsMenu()
{
    delete ui;
}

void OptionsMenu::on_exitButton_clicked()
{
    this->close();
}

void OptionsMenu::on_operationButton_clicked()
{
    this->close();
    LaserMenu *LaserMenuWindow = new LaserMenu();
    LaserMenuWindow->show();


}

void OptionsMenu::on_installButton_clicked()
{
    this->close();
    Autenticate *AutenticateWindow = new Autenticate();
    AutenticateWindow->show();
}

void OptionsMenu::on_StartUPButton_clicked()
{
    this->close();
    StartUPVal *StartUpValWindow = new StartUPVal();
    StartUpValWindow->show();
}

void OptionsMenu::on_calibrationButton_clicked()
{
    this->close();
    CalibrationValues *CalibrationValuesWindow = new CalibrationValues();
    CalibrationValuesWindow->show();

}
