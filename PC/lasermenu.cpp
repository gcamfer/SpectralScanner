#include "lasermenu.h"
#include "ui_lasermenu.h"
#include "mainwindow.h"
#include "steinhart.h"
#include "calibrationvalues.h"
#include "startupval.h"
#include "QRect"
#include "QDesktopWidget"
#include <string.h>
#include <QString>
#include <QMessageBox>
#include <locale.h>


#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <unistd.h>     // UNIX standard function definitions
#include <sys/ioctl.h>
#include "globals.h"


LaserMenu::LaserMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LaserMenu)
{
    ui->setupUi(this);
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());

    ///////////////////////////////////////////////////////
    ///         CONECTION WITH uC
    ///         FIRST TRY TO CONNECT
    ///////////////////////////////////////////////////////
LaserMenu::on_ConnectButton_clicked();

}

LaserMenu::~LaserMenu()
{
    delete ui;
}

void LaserMenu::on_LaserExitButton_clicked()
{
    this->close();

}


void LaserMenu::on_StartUPButton_clicked()
{
    this->close();
    StartUPVal *StartUpValWindow = new StartUPVal();
    StartUpValWindow->show();
}


void LaserMenu::on_MonoCalibrationButton_clicked()
{
    this->close();
    CalibrationValues *CalibrationValuesWindow = new CalibrationValues();
    CalibrationValuesWindow->show();
}


void LaserMenu::on_buttonBox_accepted()
{
    if((ui->UsernameTextBox->text().toStdString() == "monocrom")&&(ui->PasswordTextBox->text().toStdString()=="SP300i@L022"))
    {
        QMessageBox::information(this,"Password","Password acepted");
        ui->InstallGratButton->setEnabled(true);
        ui->InstallTurrButton->setEnabled(true);


        ui->GainComboBox_3->setEnabled(true);
        ui->RecalculateValueButton_2->setEnabled(true);
        ui->Browse2Button_2->setEnabled(true);
        ui->ImportNewCurveButton_2->setEnabled(true);
    }
    else QMessageBox::critical(this,"Error","Autentication error");
}


void LaserMenu::on_ConnectButton_clicked()
{
    int state;

    state = ConnectNow();
    if(state<0)QMessageBox::information(this,"Connection ","Connection failed\nPlease, connect before, to allow scans");

     if(connected==true)
     {
         ui->ConnectButton->setText("Connected");
         ui->progressBar->setValue(100);
         ui->groupBox_3->setEnabled(true);
         ui->groupBox_4->setEnabled(true);
         ui->ScanLDButton->setEnabled(true);
         ui->ScangroupBox->setEnabled(true);
         ui->LDStartWavBox->setEnabled(true);
         ui->LDStopWavBox->setEnabled(true);
         ui->LDDelayBox->setEnabled(true);
         ui->LDGainComboBox->setEnabled(true);
         ui->LDNumScansBox->setEnabled(true);
         ui->LDResSpinBox->setEnabled(true);

     }

}

void LaserMenu::on_ThermistorCalibrationButton_clicked()
{
    FILE *pipe;
    FILE *thermistorCurve;


    thermistorCurve = fopen("Termistor.txt", "r");
    fscanf(thermistorCurve,"%s\n",thermistorName);
    fclose(thermistorCurve);


    // Code for gnuplot pipe
        pipe = popen("gnuplot -persist", "w");
        fprintf(pipe, "\n");
        fprintf(pipe, "set xrange[%d:%d]\n",0,60);
        fprintf(pipe, "set xlabel \"Temperature (%cC)\" \n",248);
        fprintf(pipe, "set ylabel \"Resistivity (ohms)\" \n");
        fprintf(pipe, "set title \"TYPICAL RESPONSE FOR %s\" \n",thermistorName);
        fprintf(pipe, "unset key\n");
        fprintf(pipe, "set mxtics 5\n");
        fprintf(pipe, "set mytics 5\n");
        fprintf(pipe, "set grid mxtics mytics xtics ytics\n");
        fprintf(pipe, "plot \"%s\" with lines\n", responsivityCurveFileName);
        fprintf(pipe, "set terminal postscript color enhanced\n");
        fprintf(pipe, "set output \"Thermistor.eps\"\n");
        fclose(pipe);
}

void LaserMenu::on_SteinHartButton_clicked()
{
   SteinHart *SteinHartWindow = new SteinHart();
   SteinHartWindow->show();

}

void LaserMenu::on_LDStartWavBox_valueChanged(double arg1)
{
    if(arg1>=ui->LDStopWavBox->value())
    {
        ui->LDStopWavBox->setValue(arg1+1);
    }
}
