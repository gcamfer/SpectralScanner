#include "operationmenu.h"
#include "ui_operationmenu.h"
#include "mainwindow.h"
#include "optionsmenu.h"
#include "QRect"
#include "QDesktopWidget"

OperationMenu::OperationMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OperationMenu)
{
    ui->setupUi(this);
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());
}

OperationMenu::~OperationMenu()
{
    delete ui;
}

void OperationMenu::on_BackButtonOperation_clicked()
{
    this->close();
    OptionsMenu *OptionsMenuWindow = new OptionsMenu();
    OptionsMenuWindow->show();

}

void OperationMenu::on_GoToButton_clicked()
{
    //Send stop lambda to UART0: file fp
     //fprintf(fp,"g100\n");
}
