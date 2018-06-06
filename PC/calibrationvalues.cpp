#include "calibrationvalues.h"
#include "ui_calibrationvalues.h"
#include "QRect"
#include "QDesktopWidget"
#include "lasermenu.h"
#include <QString>
#include <stdio.h>
#include <QMessageBox>


CalibrationValues::CalibrationValues(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalibrationValues)
{
    /*Get all calibration values*/
    /*Set all calibration values*/
    ui->setupUi(this);
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());
}

CalibrationValues::~CalibrationValues()
{
    delete ui;
}

void CalibrationValues::on_ExitButton_clicked()
{
    this->close();
    LaserMenu *LaserMenuWindow = new LaserMenu();
    LaserMenuWindow->show();
}

void CalibrationValues::on_SaveButton_clicked()
{
    FILE *fs;
    QString gratings[10];
    gratings[0] = ui->grat1ET->text();
    gratings[1] = ui->grat2ET->text();
    gratings[2] = ui->grat3ET->text();
    gratings[3] = ui->grat4ET->text();
    gratings[4] = ui->grat5ET->text();
    gratings[5] = ui->grat6ET->text();
    gratings[6] = ui->grat7ET->text();
    gratings[7] = ui->grat8ET->text();
    gratings[8] = ui->grat9ET->text();

    QString Offsets[10];
    Offsets[0] = ui->off1ET->text();
    Offsets[1] = ui->off2ET->text();
    Offsets[2] = ui->off3ET->text();
    Offsets[3] = ui->off4ET->text();
    Offsets[4] = ui->off5ET->text();
    Offsets[5] = ui->off6ET->text();
    Offsets[6] = ui->off7ET->text();
    Offsets[7] = ui->off8ET->text();
    Offsets[8] = ui->off9ET->text();

    QString gadjusts[10];
    gadjusts[0] = ui->gad1ET->text();
    gadjusts[1] = ui->gad2ET->text();
    gadjusts[2] = ui->gad3ET->text();
    gadjusts[3] = ui->gad4ET->text();
    gadjusts[4] = ui->gad5ET->text();
    gadjusts[5] = ui->gad6ET->text();
    gadjusts[6] = ui->gad7ET->text();
    gadjusts[7] = ui->gad8ET->text();
    gadjusts[8] = ui->gad9ET->text();

    fs = fopen("Calibration.txt", "wt");
        if(!fs) {
           QMessageBox::critical(this,"File error","Error opening the file");
           this->close();
        }


    fprintf(fs,"Gratting \tOffset \t\tGadjust\n");
    fprintf(fs,"%s\t\t%s\t\t%s\n",gratings[0].toStdString().c_str(),Offsets[0].toStdString().c_str(),gadjusts[0].toStdString().c_str());
    fprintf(fs,"%s\t\t%s\t\t%s\n",gratings[1].toStdString().c_str(),Offsets[1].toStdString().c_str(),gadjusts[1].toStdString().c_str());
    fprintf(fs,"%s\t\t%s\t\t%s\n",gratings[2].toStdString().c_str(),Offsets[2].toStdString().c_str(),gadjusts[2].toStdString().c_str());
    fprintf(fs,"%s\t\t%s\t\t%s\n",gratings[3].toStdString().c_str(),Offsets[3].toStdString().c_str(),gadjusts[3].toStdString().c_str());
    fprintf(fs,"%s\t\t%s\t\t%s\n",gratings[4].toStdString().c_str(),Offsets[4].toStdString().c_str(),gadjusts[4].toStdString().c_str());
    fprintf(fs,"%s\t\t%s\t\t%s\n",gratings[5].toStdString().c_str(),Offsets[5].toStdString().c_str(),gadjusts[5].toStdString().c_str());
    fprintf(fs,"%s\t\t%s\t\t%s\n",gratings[6].toStdString().c_str(),Offsets[6].toStdString().c_str(),gadjusts[6].toStdString().c_str());
    fprintf(fs,"%s\t\t%s\t\t%s\n",gratings[7].toStdString().c_str(),Offsets[7].toStdString().c_str(),gadjusts[7].toStdString().c_str());
    fprintf(fs,"%s\t\t%s\t\t%s\n",gratings[8].toStdString().c_str(),Offsets[8].toStdString().c_str(),gadjusts[8].toStdString().c_str());

    QMessageBox::information(this,"Save","successfully stored calibration in Calibration.txt");
    fclose(fs);
    this->close();
    LaserMenu *LaserMenuWindow = new LaserMenu();
    LaserMenuWindow->show();
    //Save to a file all calibration Values
}
