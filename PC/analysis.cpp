#include "analysis.h"
#include "ui_analysis.h"
#include "QRect"
#include "QDesktopWidget"
#include <QMessageBox>
#include <QString>
#include "ledmenu.h"
#include "globals.h"
#include <stdio.h>
#include <math.h>
#include <unistd.h>     // UNIX standard function definitions
#include <sys/ioctl.h>
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>


/*COLOR DEFINITIONS*/
#define UVEnd 390
#define VioletEnd 455
#define BlueEnd 492
#define GreenEnd 577
#define YellowEnd 597
#define OrangeEnd 620
#define RedEnd 770
#define FirstWindowStart 850
#define FirstWindowEnd 950
#define SecondWindow 1300
#define ThirdWindow 1550



/*
Material *AlGaAs= new Material();
Material *GaP= new Material();
Material *GaAsP= new Material();
Material *AlGaInP= new Material();
Material *GaInAsP= new Material();
Material *Nitrides= new Material();
*/



/*MATERIALS*/
/*
GaAs
AlGaAs/GaAs
GaP
GaP:N
GaAsP/GaAs
GaAsP/GaP
AlGaInP/GaAs
GaInAsP/InP
Nitrides AlGaInN
*/


/*CONSTANTS*/
//#define planc 6.62607*10^34 (J*s)


/*Global Variables for Analyze*/
FILE *colorFile;
char header[256];
char buf[256];
char str[26];
static float Iph,Resp;
float SpectralWidthInitial,SpectralWidthFinal;
double Popt,Plum,Efflum;
float MaxVisPeakX;
double MaxVisPeakY;
char luminousEfFile[128]="Calibrations/LuminousEfficiency.txt";
char RGBFile[128]="Calibrations/RGBtab.txt";
float RED=0.0,GREEN=0.0,BLUE=0.0;
float sumR,sumG,sumB;

Analysis::Analysis(QWidget *parent):
    QWidget(parent),
    ui(new Ui::Analysis)
{
    ui->setupUi(this);
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());
    sprintf(buf,"Analysis Results for:%s",ScanBoxName);
    this->setWindowTitle(buf);

    int found;
    FILE *scanFile;
    FILE *destFile;


    found=0;
    SpectralWidthFinal=0;
    SpectralWidthInitial=0;
    float X[2],Y[2];
    double value,Y2,Y3;
    double R=0.0,G=0.0,B=0.0;
    Popt=0;
    Plum=0;
    X[0]=0;
    X[1]=0;
    Y[0]=0;
    Y[1]=0;
    Y2=0;
    Y3=0;

    PoptMAX=0;

    scanFile = fopen(ScanBoxName,"r");
    destFile = fopen("Scans/LuminousSpectreTemp","w");

       MaxVisPeakX=0;
       MaxVisPeakY=0;

       fgets(header,sizeof(header),scanFile);
        while(!feof(scanFile))
        {

            X[0]=X[1];//Actualization  the first lambda
            Y[0]=Y[1];
            Y2=Y3;
            setlocale(LC_ALL, "C");

            fscanf(scanFile,"%f %f %f %f\n",&X[1],&Iph,&Resp,&Y[1]);
            Popt = Popt + (Y[0]*(X[1]-X[0])) + ((Y[1]-Y[0])*(X[1]-X[0])/2.0);

            if(Y[1]>PoptMAX)
            {
                lambdaMAX = X[1];
                PoptMAX = Y[1];
            }
            value = getEFF(X[1]);
            Y3 = value*Y[1];        // Popt value
            Plum = Plum + (Y2*(X[1]-X[0])) + ((Y3-Y2)*(X[1]-X[0])/2.0);


            if(MaxVisPeakY<Y3)
            {
                MaxVisPeakX=X[1];
                MaxVisPeakY=Y3;
            }
            fprintf(destFile,"%f %e\n",X[1],Y3);
        }
        ui->MaximumVisionBox->setText(QString::number(MaxVisPeakX));
        ui->MaximumPeakBox->setText(QString::number(lambdaMAX));
        ui->EnergyGapBox->setText(QString::number(1.24/(lambdaMAX/1000.0)));
        ui->PowerBox->setText(QString::number(Popt));
        ui->LuminousFluxBox->setText(QString::number(Plum));
        fclose(destFile);
        ///////////////////////////////////////////////
        ///
        ///         DECODE COLOR WAVELENGTH
        ///
        ///////////////////////////////////////////////
        X[0]=0;
        X[1]=0;
        Y[0]=0;
        Y[1]=0;
        rewind(scanFile);
        fgets(header,sizeof(header),scanFile);
        while(!feof(scanFile))
        {
        X[0]=X[1];//Actualization  the first lambda
        Y[0]=Y[1];
        Y2=Y3;
        setlocale(LC_ALL, "C");

        fscanf(scanFile,"%f %f %f %f\n",&X[1],&Iph,&Resp,&Y[1]);

        getRGB(X[1]);

        //First normalize colors & get values RGB
        Y[1] = Y[1]/PoptMAX; //Popt normalized
        R = R + (Y[1])*RED;
        G = G + (Y[1])*GREEN;
        B = B + (Y[1])*BLUE;

        }
        //Normalize RGB file to a 256 bits value
        R = (R/sumR)*255.0;    //MAX 120.0 in full range
        if (R>255) R=255;
        G = (G/sumG)*255.0;    //MAX 102.0 in full range
        if (G>255) G=255;
        B = (B/sumB)*255.0;     //MAX 55.2 in full range
        if (B>255) B=255;


       // QPalette *palette = new QPalette();
       // palette->setColor(ui->ColorBox->backgroundRole(),QColor(int(R),int(G),int(B)));
        QPalette palette;
        palette.setColor(QPalette::Base,QColor(int(R),int(G),int(B)));
        ui->ColorBox->setPalette(palette);

        if (lambdaMAX<UVEnd)ui->ColorBox->setText("UltraViolet");
        else if ((lambdaMAX>UVEnd)&&(lambdaMAX<=VioletEnd))ui->ColorBox->setText("Violet");
        else if ((lambdaMAX>VioletEnd)&&(lambdaMAX<=BlueEnd))ui->ColorBox->setText("Blue");
        else if ((lambdaMAX>BlueEnd)&&(lambdaMAX<=GreenEnd))ui->ColorBox->setText("Green");
        else if ((lambdaMAX>GreenEnd)&&(lambdaMAX<=YellowEnd))ui->ColorBox->setText("Yellow");
        else if ((lambdaMAX>YellowEnd)&&(lambdaMAX<=OrangeEnd))ui->ColorBox->setText("Orange");
        else if ((lambdaMAX>OrangeEnd)&&(lambdaMAX<=RedEnd))ui->ColorBox->setText("Red");
        else if ((lambdaMAX>FirstWindowStart)&&(lambdaMAX<=FirstWindowEnd))ui->ColorBox->setText("First Window");
        else if ((lambdaMAX>(SecondWindow-10))&&(lambdaMAX<=(SecondWindow+10)))ui->ColorBox->setText("Second Window");
        else if ((lambdaMAX>(ThirdWindow-10))&&(lambdaMAX<=(ThirdWindow+10)))ui->ColorBox->setText("Third Window");
        else ui->ColorBox->setText("Infra Red");


        X[0]=0;
        X[1]=0;
        Y[0]=0;
        Y[1]=0;
        rewind(scanFile);
        fgets(header,sizeof(header),scanFile);
        while((!feof(scanFile))&&(found<2))
        {
            if(((PoptMAX/2)>Y[1])&&(found==0))//search the first
                {
                    X[0]=X[1];//Actualization  the first lambda
                    Y[0]=Y[1];
                    setlocale(LC_ALL, "C");
                    fscanf(scanFile,"%f %f %f %f\n",&X[1],&Iph,&Resp,&Y[1]);
                }
            else if(((PoptMAX/2)<Y[1])&&(found==1))//Search for second
            {
                X[0]=X[1];//Actualization  the first lambda
                Y[0]=Y[1];
                setlocale(LC_ALL, "C");
                fscanf(scanFile,"%f %f %f %f\n",&X[1],&Iph,&Resp,&Y[1]);
            }

            else if(((PoptMAX/2)>Y[0])&&((PoptMAX/2)<Y[1])&&(found==0))//Lambda is in the midle
            {
                SpectralWidthInitial = ((X[1]-X[0])*(((PoptMAX/2)-Y[0])/(Y[1]-Y[0])))+X[0];
                found++;
                X[0]=X[1];//Actualization  the first lambda
                Y[0]=Y[1];
                setlocale(LC_ALL, "C");
                fscanf(scanFile,"%f\t%f\t%f\t%f\n",&X[1],&Iph,&Resp,&Y[1]);
            }
            else if(((PoptMAX/2)>Y[1])&&((PoptMAX/2)<=Y[0])&&(found==1))//Lambda is in the midle
            {
                SpectralWidthFinal = ((X[1]-X[0])*(((PoptMAX/2)-Y[0])/(Y[1]-Y[0])))+X[0];
                found++;
            }
            else if((PoptMAX/2)==Y[1])
            {
                if(found==0)SpectralWidthInitial=X[1];
                else if(found==1)SpectralWidthFinal=X[1];
                found++;
            }
        }
        //Reached end of file. Last test:
        if(((PoptMAX/2)>Y[1])&&((PoptMAX/2)<=Y[0])&&(found==1))
        {
            SpectralWidthFinal = ((X[1]-X[0])*(((PoptMAX/2)-Y[0])/(Y[1]-Y[0])))+X[0];
            found++;
        }
        else if((PoptMAX/2)==Y[1])
        {
            if(found==0)SpectralWidthInitial=X[1];
            else if(found==1)SpectralWidthFinal=X[1];
            found++;
        }
        ui->SpectralWidthBox->setText(QString::number(SpectralWidthFinal-SpectralWidthInitial));
        ui->SpectralWidthIneVbox->setText(QString::number((1.8/1.45)*(SpectralWidthFinal-SpectralWidthInitial)*1000.0/(pow(lambdaMAX,2))));
        fclose(scanFile);


        QMessageBox msgBox;
        msgBox.setWindowTitle("Luminosity spectre");
        msgBox.setText("Would you also display the luminosity curve?");
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        if(msgBox.exec() == QMessageBox::Yes){
            FILE *pipe;

            pipe = popen("gnuplot -persist", "w");
            fprintf(pipe, "\n");
            fprintf(pipe, "set xlabel \"Wavelength (nm)\" \n");
            fprintf(pipe, "set ylabel \"Luminosity (lm)\" \n");
            fprintf(pipe, "set title \"Luminous efficiency x Spectral analysis @ http://www.cvrl.org/\" \n");
            fprintf(pipe, "unset key\n");
            fprintf(pipe, "set mxtics 5\n");
            fprintf(pipe, "set mytics 5\n");
            fprintf(pipe, "plot \"%s\" using ($1):($2) w l lc 1 lt 1 lw 2 \n","Scans/LuminousSpectreTemp");
            fclose(pipe);
          }
    }


double Analysis::getEFF(float lambda)
{
    FILE *effFile;
    int found;
    float value,X[2],Y[2];
    found = 0;
    effFile = fopen(luminousEfFile,"r");

   X[0]=0;
   X[1]=0;
   Y[0]=0;
   Y[1]=0;

    while((!feof(effFile))&&(!found))
    {
        if((lambda>X[0])&&(lambda>X[1]))//Not reached the lambda --> get new lambdas
            {
                X[0]=X[1];//Actualization the first lambda
                Y[0]=Y[1];
                setlocale(LC_ALL, "C");
                fscanf(effFile,"%f %e\n",&X[1], &Y[1]);
            }
        else if((lambda>X[0])&&(lambda<X[1]))//Lambda is in the midle
        {
            found=true;
            value = ((Y[1]-Y[0])*((lambda-X[0])/(X[1]-X[0])))+Y[0];
            break;
        }
        else if(lambda==X[0])
        {
            value=Y[0];
            found=true;
            break;
        }
        else if(lambda==X[1])
        {
            value=Y[1];
            found=true;
            break;
        }
    }
    fclose(effFile);

    if (!found)
    {
        return 0;
    }
    return (value);
}


Analysis::~Analysis()
{
    delete ui;
}

void Analysis::on_pushButton_clicked()
{
    this->close();
}

void Analysis::on_reanalyzeButton_clicked()
{

    double Vf = ui->VfSpinBox->value();

}

void Analysis::on_LenOnButton_clicked()
{
    if(connected)
    {
        strcpy(str,"LEDON\n");
        n = write(fd, str, strlen(str));                        //SEND FUNCTION
        if (n < 0)
        {
            fputs("write() LEDSC failed!\n", stderr);
        }
        else n=ugets(fd, buf);                              //READ "AOK"
        if(strcmp(buf,"AOK")==0)
        {
            QMessageBox::information(this,"LED state",
            "The LED is now on ON\nswitch to OFF after the measurement, to prevent overheating");
        }
    }
    else QMessageBox::critical(this,"Error","Connection Failed");

}

void Analysis::on_LedOffButton_clicked()
{
    if(connected)
    {
        strcpy(str,"LEDOF\n");
        n = write(fd, str, strlen(str));                        //SEND FUNCTION
        if (n < 0)
        {
            fputs("write() LEDSC failed!\n", stderr);
        }
        else n=ugets(fd, buf);                              //READ "AOK"
        if(strcmp(buf,"AOK")==0)
        {
            QMessageBox::information(this,"LED state",
            "The LED is now on OFF");
        }
    }
    else QMessageBox::critical(this,"Error","Connection Failed");
}


void Analysis::getRGB(float lambdax)
{
    bool found;
    FILE *RGBptr;
    int wavelength;
    sumR=0;
    sumG=0;
    sumB=0;
    RGBptr = fopen(RGBFile,"r");
    while((!feof(RGBptr)&&(found==false)))
    {
        setlocale(LC_ALL, "C");
        fscanf(RGBptr,"%d\t%e\t%e\t%e\n",&wavelength, &RED, &GREEN, &BLUE);
        sumR = sumR + RED;
        sumG = sumG + GREEN;
        sumB = sumB + BLUE;
        if(int(lambdax)==wavelength)
            found = true;
    }
    //if (!feof(RGBptr)){RED=0;GREEN=0;BLUE=0;}

    fclose(RGBptr);
}
