#ifndef LASERMENU_H
#define LASERMENU_H

#include <QWidget>

namespace Ui {
class LaserMenu;
}

class LaserMenu : public QWidget
{
    Q_OBJECT

public:
    explicit LaserMenu(QWidget *parent = 0);
    ~LaserMenu();


private slots:
    void on_LaserExitButton_clicked();

    void on_StartUPButton_clicked();

    void on_MonoCalibrationButton_clicked();

    void on_buttonBox_accepted();

    void on_ConnectButton_clicked();

    void on_ThermistorCalibrationButton_clicked();

    void on_SteinHartButton_clicked();

    void on_LDStartWavBox_valueChanged(double arg1);

private:
    Ui::LaserMenu *ui;
    class LaserParameters;
};


#endif // LASERMENU_H
