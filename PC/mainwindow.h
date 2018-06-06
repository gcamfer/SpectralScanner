#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
/*#include <stdio.h>
#include <unistd.h>     // UNIX standard function definitions
#include <sys/ioctl.h>
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>*/



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private slots:
    void on_LEDButton_clicked();

    void on_LDButton_clicked();


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
