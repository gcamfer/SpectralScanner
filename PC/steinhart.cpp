#include "steinhart.h"
#include "ui_steinhart.h"
#include "QRect"
#include "QDesktopWidget"
#include <math.h>
#include "globals.h"
#include <locale.h>

SteinHart::SteinHart(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SteinHart)
{
    ui->setupUi(this);
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());


}

SteinHart::~SteinHart()
{
    delete ui;
}

void SteinHart::on_pushButton_2_clicked()
{
    this->close();
}

void SteinHart::on_CalculateButton_clicked()
{
    float A,B,C;
    float X,Y,T,Rth;
    char title[256],output[256];
    FILE *SHvalues;
    FILE *pipe;

    A = ui->Acoeff->text().toFloat()/1000;
    B = ui->Bcoeff->text().toFloat()/10000;
    C = ui->Ccoeff->text().toFloat()/10000000;

    strftime(output, 128, "%y-%m-%d-%H:%M:%S", tlocal);

    sprintf(title,"Calibrations/SteinHartCalibration%c20%s",64,output);

    SHvalues = fopen(title,"w");
    setlocale(LC_ALL, "C");
    fprintf(SHvalues,"%s\n","#Temperature\tResistance");
        for(T=0.0;T<60.0;T=T+0.1)
        {
        X = (A-(1.0/(1.0*T+273.15)))/C;
        Y = sqrt((B*B*B/C/C/C/27.0)+(X*X/4.0));
        Rth = exp(pow( (Y-(X/2.0)) , (1.0/3.0) ) - pow( (Y+(X/2.0)) , (1.0/3.0) ));
        fprintf(SHvalues,"%.6f\t%.6f\n",T,Rth);
        }
        fclose(SHvalues);

        // Code for gnuplot pipe
        pipe = popen("gnuplot -persist", "w");
        fprintf(pipe, "set termoption enhanced\n");
        fprintf(pipe, "\n");
        fprintf(pipe, "set xrange[%d:%d]\n", 0, 60);
        fprintf(pipe, "set xlabel \"Temperature  (\260 C)\" \n");
        fprintf(pipe, "set ylabel \"Resistance ({/Symbol W})\" \n");
        fprintf(pipe, "set title \"SteinHart-Hart Calibration @ %s\" \n",output);
        fprintf(pipe, "unset key\n");
        fprintf(pipe, "set mxtics 5\n");
        fprintf(pipe, "set mytics 5\n");
        fprintf(pipe, "set grid mxtics mytics xtics ytics\n");
        fprintf(pipe, "plot \"%s\" using ($1):($2) w l\n", title);
        fprintf(pipe, "set terminal postscript color enhanced\n");
        fprintf(pipe, "set output \"%s.eps\"\n", title);
        fprintf(pipe, "replot\n");
        fclose(pipe);
}
