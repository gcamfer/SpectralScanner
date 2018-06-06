#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ledmenu.h"
#include "lasermenu.h"
#include "QRect"
#include "QDesktopWidget"
#include <QMessageBox>
#include <stdio.h>
//#include <unistd.h>     // UNIX standard function definitions
//#include <sys/ioctl.h>
//#include <fcntl.h>      // File control definitions
//#include <errno.h>      // Error number definitions
//#include <termios.h>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_LEDButton_clicked()
{
    LEDMenu * LEDMenuWindow = new LEDMenu();
        LEDMenuWindow->show();
        this->close();

}

void MainWindow::on_LDButton_clicked()
{
    LaserMenu* LaserMenuWindow = new LaserMenu();
    LaserMenuWindow->show();
    this->close();
   }

