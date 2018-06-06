#ifndef LEDMENU_H
#define LEDMENU_H

#include <QWidget>
#include "mainwindow.h"
#include "globals.h"


namespace Ui {
class LEDMenu;
}

class LEDMenu : public QWidget
{
    Q_OBJECT

public:
    explicit LEDMenu(QWidget *parent = 0);
    ~LEDMenu();

private slots:
    void on_ExitButton_clicked();

    void on_GoToButton_clicked();

    void on_ConnectButton_clicked();

    void on_terminalButton_clicked();

    void keyPressEvent(QKeyEvent *event);

    void on_EndTerminalButton_clicked();

    void on_SendTerminalButton_clicked();

    void on_StartScanButton_clicked();

    void on_SaveFileNameButton_clicked();

    void on_SeeCurveButton_clicked();

    void on_RecalculateValueButton_clicked();

    void on_AnalyzeButton_clicked();

    void on_ScanWindow_tabBarClicked();

    void on_ImportScanButton_clicked();

    void on_StartSpinBox_valueChanged(double arg1);

    void on_Browse1Button_clicked();

    void on_Browse2Button_clicked();

    void on_buttonBox_2_accepted();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::LEDMenu *ui;
};

#endif // LEDMENU_H
