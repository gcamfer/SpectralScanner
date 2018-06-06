#include "ledmenu.h"
#include "ui_ledmenu.h"
#include "mainwindow.h"
#include "QRect"
#include "QDesktopWidget"
#include "globals.h"
//#include <stdio.h>
#include <string.h>
#include <QString>
#include <QMessageBox>
#include <QKeyEvent>
#include <locale.h>
#include "analysis.h"

#include "qfiledialog.h"

#include <unistd.h>     // UNIX standard function definitions
#include <sys/ioctl.h>
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions



LEDMenu::LEDMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LEDMenu)
{
    ui->setupUi(this);
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());
    ///////////////////////////////////////////////////////
    ///         CONECTION WITH uC
    ///         FIRST TRY TO CONNECT
    ///////////////////////////////////////////////////////
    LEDMenu::on_ConnectButton_clicked();
}

LEDMenu::~LEDMenu()
{
    delete ui;
}

void LEDMenu::on_ExitButton_clicked()
{
    this->close();
}


///////////////////////////////////////////////////////
/// \brief LEDMenu::on_ConnectButton_clicked
///////////////////////////////////////////////////////
///
///         CONECTION WITH uC
///////////////////////////////////////////////////////


void LEDMenu::on_ConnectButton_clicked()
{
    int state;

    state = ConnectNow();
    if(state<0)QMessageBox::information(this,"Connection ","Connection failed\nPlease, connect before, to allow scans");

    if(connected)
      {
      ui->ConnectButton->setText("Connected");
      ui->progressBar->setValue(100);
      ui->terminalImput->setEnabled(false);
      ui->GoToButton->setEnabled(true);
      ui->StartScanButton->setEnabled(true);
      ui->GoToSpinBox->setEnabled(true);
      ui->StartSpinBox->setEnabled(true);
      ui->StopSpinBox->setEnabled(true);
      ui->GainComboBox->setEnabled(true);
      ui->ResSpinBox->setEnabled(true);
      ui->terminalButton->setEnabled(true);
      ui->EndTerminalButton->setEnabled(false);
      ui->SendTerminalButton->setEnabled(false);
      ui->RecalculateValueButton->setEnabled(true);
      ui->GainComboBox_2->setEnabled(true);
        }

  }




///////////////////////////////////////////////////
///////////////////////////////////////////////////
///
///             FUNCTIONS BELOW
///////////////////////////////////////////////////

void LEDMenu::on_GoToButton_clicked()
{
    float lambda;
    int n;
    char str[20]="\0";
    char zero[3]="";
    char *ret;
    static char buf[256];
   // QString str;

    if(connected==true)
    {
    lambda=ui->GoToSpinBox->value();

    //if((lambda<1000)&&(lambda>=100))strcpy(zero,"0");
    //else if((lambda<100)&&(lambda>=10))strcpy(zero,"00");
    //else if(lambda<10)strcpy(zero,"000");

    sprintf(str,"LEDGT-%.2f\n",lambda);
    ui->debugBrowser->append(str);
    n = write(fd, str, strlen(str));                        //SEND FUNCTION & PARAMETERS
    if (n < 0)fputs("write() LEDGT failed!\n", stderr);

    do{
        n=ugets(fd, buf);                              //READ FUNCTION DESCRIPTION
        ui->debugBrowser->append(buf);//Echo debug
        ret = strstr(buf, "OK");
    }while(ret==NULL);

        if(strcmp(buf,"AOK")==0)
        {
        ui->GoToSpinBox->setValue(lambda);
        }
        else QMessageBox::critical(this,"GoTo","Bad response from GoTo statement");



    }
    else QMessageBox::information(this,"Connection ","Please first establish communication with the controller\n Press Connect Button");
}


void LEDMenu::on_terminalButton_clicked()
{
    int n;
    char str[20]="\0";
    static char buf[256];
    ui->terminalImput->setEnabled(true);
    ui->GoToButton->setEnabled(false);
    ui->StartScanButton->setEnabled(false);
    ui->GoToSpinBox->setEnabled(false);
    ui->StartSpinBox->setEnabled(false);
    ui->StopSpinBox->setEnabled(false);
    ui->GainComboBox->setEnabled(false);
    ui->ResSpinBox->setEnabled(false);
    ui->EndTerminalButton->setEnabled(true);
    ui->SendTerminalButton->setEnabled(true);
    ui->terminalButton->setEnabled(false);
    ui->RecalculateValueButton->setEnabled(false);




    ui->terminalButton->setText("Terminal mode: Press ESQ to exit");

    sprintf(str,"LEDTR\n");
    n = write(fd, str, strlen(str));
    if (n < 0)fputs("write() LEDTR\n", stderr);
    ui->debugBrowser->append("Send");
    ui->debugBrowser->append(str);
    n=ugets(fd, buf);
    ui->debugBrowser->append("Received");
    ui->debugBrowser->append(buf);


}



void LEDMenu::keyPressEvent(QKeyEvent *event)
{

    char str[20]="\0";
    static char buf[256];

     if((event->key()==Qt::Key_Escape)&&(ui->terminalImput->isEnabled()))
    {

        ui->terminalImput->setEnabled(false);
        ui->GoToButton->setEnabled(true);
        ui->StartScanButton->setEnabled(true);
        ui->GoToSpinBox->setEnabled(true);
        ui->StartSpinBox->setEnabled(true);
        ui->StopSpinBox->setEnabled(true);
        ui->GainComboBox->setEnabled(true);
        ui->ResSpinBox->setEnabled(true);
        ui->EndTerminalButton->setEnabled(false);
        ui->SendTerminalButton->setEnabled(false);
        ui->terminalButton->setEnabled(true);
        ui->RecalculateValueButton->setEnabled(true);



        ui->terminalButton->setText("Terminal mode");
        sprintf(str,"LEDTE\n");//Led mode terminal END
        n = write(fd, str, strlen(str));

        ui->debugBrowser->append("Send");
        ui->debugBrowser->append(str);
        n=ugets(fd, buf);
        ui->debugBrowser->append("Received");
        ui->debugBrowser->append(buf);


        if (n < 0)fputs("Terminal mode end ERROR!", stderr);
        ui->terminalImput->text().clear();
        }

     else if( event->key() == Qt::Key_Return )
          {
             if(ui->terminalImput->text().isEmpty())
             {
                 sprintf(str,"%s",ui->terminalImput->text().toStdString().c_str());
                 ui->debugBrowser->append("Send Empty");
                 ui->debugBrowser->append(str);
             }
             else
             {
             sprintf(str,"%s",ui->terminalImput->text().toStdString().c_str());
             ui->terminalImput->text().clear();
             n = write(fd, str, strlen(str));

             ui->debugBrowser->append("Send");
             ui->debugBrowser->append(str);
             if (n < 0)fputs("Terminal mode send buf ERROR!", stderr);
             ui->terminalImput->text().clear();
             n=ugets(fd, buf);
             ui->debugBrowser->append("Received");
             ui->debugBrowser->append(buf);
             }

           }


     else if (ui->terminalImput->text().size()>=32)
        {
        if(strcmp(ui->terminalImput->text().toStdString().c_str(),"\n"))
        {
            sprintf(str,"%s",ui->terminalImput->text().toStdString().c_str());
            ui->debugBrowser->append("Send");
            ui->debugBrowser->append(str);
        }
        else
        {
        sprintf(str,"%s",ui->terminalImput->text().toStdString().c_str());
        ui->terminalImput->text().clear();
        n = write(fd, str, strlen(str));

        ui->debugBrowser->append("Send");
        ui->debugBrowser->append(str);
        if (n < 0)fputs("Terminal mode send buf ERROR!", stderr);
        ui->terminalImput->text().clear();
        n=ugets(fd, buf);
        ui->debugBrowser->append("Received");
        ui->debugBrowser->append(buf);
        }

    }
    else event->ignore();
}

void LEDMenu::on_EndTerminalButton_clicked()
{
    char str[20]="\0";
    static char buf[256];
    ui->terminalImput->setEnabled(false);
    ui->GoToButton->setEnabled(true);
    ui->StartScanButton->setEnabled(true);
    ui->GoToSpinBox->setEnabled(true);
    ui->StartSpinBox->setEnabled(true);
    ui->StopSpinBox->setEnabled(true);
    ui->GainComboBox->setEnabled(true);
    ui->ResSpinBox->setEnabled(true);
    ui->EndTerminalButton->setEnabled(false);
    ui->SendTerminalButton->setEnabled(false);
    ui->terminalButton->setEnabled(true);

    ui->terminalButton->setText("Terminal mode");
    sprintf(str,"LEDTE\n");//Led mode terminal END
    n = write(fd, str, strlen(str));
    if (n < 0)fputs("Terminal mode end ERROR!", stderr);

    ui->debugBrowser->append("Send");
    ui->debugBrowser->append(str);
    n=ugets(fd, buf);
    ui->debugBrowser->append("Received");
    ui->debugBrowser->append(buf);

    ui->terminalImput->text().clear();

}

void LEDMenu::on_SendTerminalButton_clicked()
{
    char str[20]="\0";
    static char buf[256];


    sprintf(str,"%s\n",ui->terminalImput->text().toStdString().c_str());
    ui->terminalImput->text().clear();
    n = write(fd, str, strlen(str));

    ui->debugBrowser->append("Send");
    ui->debugBrowser->append(str);
    if (n < 0)fputs("Terminal mode send buf ERROR!", stderr);

    n=ugets(fd, buf);
    ui->debugBrowser->append("Received");
    ui->debugBrowser->append(buf);


  //  if(strcmp(ui->terminalImput->text().toStdString().c_str(),"\n"))
  //  {
  //      sprintf(str,"%s\n",ui->terminalImput->text().toStdString().c_str());
  //      ui->debugBrowser->append("Send");
  //      ui->debugBrowser->append(str);
  //  }
  //  else
  //  {
  //  sprintf(str,"%s\n",ui->terminalImput->text().toStdString().c_str());
  //  ui->terminalImput->text().clear();
  //  n = write(fd, str, strlen(str));

  //  ui->debugBrowser->append("Send");
  //  ui->debugBrowser->append(str);
  //  if (n < 0)fputs("Terminal mode send buf ERROR!", stderr);

  //  n=ugets(fd, buf);
  //  ui->debugBrowser->append("Received");
  //  ui->debugBrowser->append(buf);
  //  }

}


void LEDMenu::on_StartScanButton_clicked()
{

    float Startlambda, Stoplambda,resollution,lambda;
    double measure,responsivity,zero,Popt;
    double gain;
    int n;
    char str[26]="\0";
    char *ret;
    static char buf[256];
    int selection;
    int SelectGains[4] = {00,01,10,11};//47,94,194,414
    static double gainVector[15]={47.0,94.0,194.0,414.0,10000.0,100000.0,1000000.0,10000000.0,
                               100000000.0,1000000.0,10000000.0,100000000.0,1000000000.0,10000000000.0};
    FILE *ScanFile;
    FILE *Scans;
    FILE *pipe;

    strftime(output, 128, "%y-%m-%d_%H:%M:%S", tlocal);

    if((connected==true)&&((ui->StartSpinBox->value())<(ui->StopSpinBox->value())))
        {
        Startlambda=ui->StartSpinBox->value();
        Startlambda=Startlambda*100;//For a complete integer value
        Stoplambda=ui->StopSpinBox->value();
        Stoplambda=Stoplambda*100;
        resollution=ui->ResSpinBox->value();
        resollution=resollution*100;
        gain=gainVector[ui->GainComboBox->currentIndex()];

        if(gain==47)selection=0;
        else if(gain==94)selection=1;
        else if(gain==194)selection=2;
        else if(gain==414)selection=3;
        else {selection=0;
            QMessageBox::information(this,"GAIN","Make sure the HMS564 is in the slected gain");
        }
        if(changedName==false)
        {
        sprintf(fileName, "Scans/20%s-Start%.2f-Stop%.2f-R%.2f-Gx%.0f", output,Startlambda/100,Stoplambda/100,resollution/100,gain);
        }

        ScanFile = fopen(fileName, "w");
        Scans = fopen("Scans/LastScans","a+");//Open file to add new scans

        fprintf(ScanFile,"%s%s%s%s","#Wl(nm)\t", "Iph(mA)\t", "Resp(A/W)\t", "Popt(mW)\n"); // Writing the header

        sprintf(str,"LEDSC%.6d%.6d%.6d%.2d\n",int(Startlambda),int(Stoplambda),int(resollution),SelectGains[selection]);
        ui->debugBrowser->append(str);
        n = write(fd, str, strlen(str));                        //SEND FUNCTION & PARAMETERS
        if (n < 0)fputs("write() LEDSC failed!\n", stderr);

        n=ugets(fd, buf);                              //READ "Scanning LED mode"

        do{

            memset(buf, '\0', sizeof(buf));
            n=ugets(fd, buf);                              //READ VALUE
            ui->debugBrowser->append(buf);//Echo debug
            ret = strstr(buf, "OK");
            if((ret==NULL)&&(n!=0))
            {

                strncpy(str,buf,7);//This is the lambda part next \t and measure
                lambda=atof(str);
                memset(str, '\0', sizeof(str));//Clear string
                strncpy(str,buf+8,4);//Part of value obtained
                measure=atof(str);
                zero=zeroChecker(lambda,ui->GainComboBox->currentIndex());
                measure = ((measure-zero)*(3.3/(1023.0*gain))*1000.0);//Real current photogenerated in milliampere
                responsivity=responsivityChecker(lambda);
                Popt=measure/responsivity;
                fprintf(ScanFile, "%.2f\t%.4f\t%.9f\t%f\n", lambda, measure, responsivity, Popt); // Writing the file
            }
            n=write(fd,"ACK\n",strlen("ACK\n"));  //Acknowledge to value and solicite next

        }while(ret==NULL);

            if(strcmp(buf,"AOK")==0)
            {
            fclose(ScanFile);
            fprintf(Scans,"%s\n",fileName);
            fclose(Scans);
            changedName=false;
            sprintf(buf,"Scann results saved in: %s\nPress OK to see results",fileName);
            QMessageBox::information(this,"Scan",buf);
            // Code for gnuplot pipe
            pipe = popen("gnuplot -persist", "w");
            fprintf(pipe, "\n");
            fprintf(pipe, "set xrange[%lf:%lf]\n", (Startlambda/100.0)-20, (Stoplambda/100.0)+20);
            fprintf(pipe, "set xlabel \"Wavelength (nm)\" \n");
            fprintf(pipe, "set ylabel \"Optical Power(mW)\" \n");
            fprintf(pipe, "set title \"SCAN RESULTS\" \n");
            fprintf(pipe, "unset key\n");
            fprintf(pipe, "set mxtics 5\n");
            fprintf(pipe, "set mytics 5\n");
            //fprintf(pipe, "set grid\n");
            fprintf(pipe, "set grid mxtics mytics xtics ytics\n");
            fprintf(pipe, "plot \"%s\" using ($1):($4) w l\n", fileName);
            fprintf(pipe, "set terminal postscript color enhanced\n");
            fprintf(pipe, "set output \"%s.eps\"\n", fileName);
            fprintf(pipe, "replot\n");
            fclose(pipe);
            strcpy(ScanBoxName,fileName);
            Analysis * NewAnalysis = new Analysis();
            NewAnalysis->show();
            errors=0;
            }


            else
            {
                if(errors<1)
                {
                 QMessageBox::information(this,"Scan","Bad response from Scan statement\n retry 1 more time");
                 ui->GoToSpinBox->setValue(ui->StartSpinBox->value());
                 on_GoToButton_clicked();
                 on_StartScanButton_clicked();
                }
                else
                {
                 QMessageBox::critical(this,"Scan","Bad response from Scan statement");
                 fclose(ScanFile);
                 fclose(Scans);
                }
            }

        }

    else if((ui->StartSpinBox->value())>=(ui->StopSpinBox->value()))
    {
        QMessageBox::information(this,"Scan","Start wavelength can't be greater than Stop wavelength");
    }
    else QMessageBox::information(this,"Connection ","Please first establish communication with the controller\n Press Connect Button");

}






void LEDMenu::on_SaveFileNameButton_clicked()
{
    if(ui->FileNameEdit->text().isEmpty())
    {
        QMessageBox::information(this,"Bad File Name","Please enter a correct value don't leave it empty");
    }
    else
    {
        strcpy(fileName,"Scans/");
        strcat(fileName,ui->FileNameEdit->text().toStdString().c_str());
        changedName=true;
        QMessageBox::information(this,"File Name","The scanning file name have been changed");
    }
}


void LEDMenu::on_SeeCurveButton_clicked()
{
    SeeRespCurve();
}

void LEDMenu::on_AnalyzeButton_clicked()
{
    FILE *pipe;
    char open=0;
    float actualRange = 0;
        setlocale(LC_ALL, "C");

    // Code for gnuplot pipe
        pipe = popen("gnuplot -persist", "w");
        fprintf(pipe, "\n");
        fprintf(pipe, "set xrange[%d:%d]\n",300,1100);
        //fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,4.5);
        fprintf(pipe, "set xlabel \"Wavelength (nm)\" \n");
        fprintf(pipe, "set ylabel \"Popt (mW)\" \n");
        fprintf(pipe, "set title \"OPTICAL POWER SCAN\" \n");
        fprintf(pipe, "unset key\n");
        fprintf(pipe, "set mxtics 5\n");
        fprintf(pipe, "set mytics 5\n");
        fprintf(pipe, "set grid mxtics mytics xtics ytics\n");
        //fprintf(pipe, "set key mxtics mytics xtics ytics\n");
        fprintf(pipe, "set multiplot\n");


    if((ui->Scan1Box->isChecked()&&(!(ui->Scan1Box->text().isEmpty()))))
    {
        strcpy(ScanBoxName,ui->Scan1Box->text().toStdString().c_str());
        Analysis * NewAnalysis = new Analysis();
        NewAnalysis->show();

        if(actualRange<PoptMAX)
        {
            actualRange=PoptMAX;
            fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,actualRange);
        }
        open++;

    }
    if((ui->Scan2Box->isChecked()&&(!(ui->Scan2Box->text().isEmpty()))))
    {
        strcpy(ScanBoxName,ui->Scan2Box->text().toStdString().c_str());
        Analysis * NewAnalysis = new Analysis();
        NewAnalysis->show();
        if(actualRange<PoptMAX)
        {
            actualRange=PoptMAX;
            fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,actualRange);

        }
            open++;

    }
    if((ui->Scan3Box->isChecked()&&(!(ui->Scan3Box->text().isEmpty()))))
    {
        strcpy(ScanBoxName,ui->Scan3Box->text().toStdString().c_str());
        Analysis * NewAnalysis = new Analysis();
        NewAnalysis->show();
        if(actualRange<PoptMAX)
        {
            actualRange=PoptMAX;
            fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,actualRange);

        }
            open++;
    }
    if((ui->Scan4Box->isChecked()&&(!(ui->Scan4Box->text().isEmpty()))))
    {
        strcpy(ScanBoxName,ui->Scan4Box->text().toStdString().c_str());
        Analysis * NewAnalysis = new Analysis();
        NewAnalysis->show();
        if(actualRange<PoptMAX)
        {
            actualRange=PoptMAX;
            fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,actualRange);

        }
            open++;
    }
    if((ui->Scan5Box->isChecked()&&(!(ui->Scan5Box->text().isEmpty()))))
    {
        strcpy(ScanBoxName,ui->Scan5Box->text().toStdString().c_str());
        Analysis * NewAnalysis = new Analysis();
        NewAnalysis->show();
        if(actualRange<PoptMAX)
        {
            actualRange=PoptMAX;
            fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,actualRange);

        }
            open++;
    }
    if((ui->Scan6Box->isChecked()&&(!(ui->Scan6Box->text().isEmpty()))))
    {
        strcpy(ScanBoxName,ui->Scan6Box->text().toStdString().c_str());
        Analysis * NewAnalysis = new Analysis();
        NewAnalysis->show();
        if(actualRange<PoptMAX)
        {
            actualRange=PoptMAX;
            fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,actualRange);

        }
            open++;
    }
    if((ui->Scan7Box->isChecked()&&(!(ui->Scan7Box->text().isEmpty()))))
    {
        strcpy(ScanBoxName,ui->Scan7Box->text().toStdString().c_str());
        Analysis * NewAnalysis = new Analysis();
        NewAnalysis->show();
        if(actualRange<PoptMAX)
        {
            actualRange=PoptMAX;
            fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,actualRange);

        }
            open++;
    }
    if((ui->Scan8Box->isChecked()&&(!(ui->Scan8Box->text().isEmpty()))))
    {
        strcpy(ScanBoxName,ui->Scan8Box->text().toStdString().c_str());
        Analysis * NewAnalysis = new Analysis();
        NewAnalysis->show();
        if(actualRange<PoptMAX)
        {
            actualRange=PoptMAX;
            fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,actualRange);

        }
            open++;
    }
    if((ui->Scan9Box->isChecked()&&(!(ui->Scan9Box->text().isEmpty()))))
    {
        strcpy(ScanBoxName,ui->Scan9Box->text().toStdString().c_str());
        Analysis * NewAnalysis = new Analysis();
        NewAnalysis->show();
        if(actualRange<PoptMAX)
        {
            actualRange=PoptMAX;
            fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,actualRange);

        }
            open++;
    }
    if((ui->Scan10Box->isChecked()&&(!(ui->Scan10Box->text().isEmpty()))))
    {
        strcpy(ScanBoxName,ui->Scan10Box->text().toStdString().c_str());
        Analysis * NewAnalysis = new Analysis();
        NewAnalysis->show();
        if(actualRange<PoptMAX)
        {
            actualRange=PoptMAX;
            fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,actualRange);
        }
            open++;
    }


    //////////////////////////////////////////////////
    ///     PLOTING NOW FOR A CORRECT Y RANGE
    /// ///////////////////////////////////////

    if((ui->Scan1Box->isChecked()&&(!(ui->Scan1Box->text().isEmpty()))))
    {

        fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 1 lt 0 lw 4\n",
                ui->Scan1Box->text().toStdString().c_str());
        /*fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 1 lt 0 lw 4 title \"%s\"\n",
                ui->Scan1Box->text().toStdString().c_str(),ui->Scan1Box->text().toStdString().c_str());*/

    }
    if((ui->Scan2Box->isChecked()&&(!(ui->Scan2Box->text().isEmpty()))))
    {

        fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 8 lt 1 \n",
                ui->Scan2Box->text().toStdString().c_str());
    }
    if((ui->Scan3Box->isChecked()&&(!(ui->Scan3Box->text().isEmpty()))))
    {
        fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 7 lt 1 \n",
                ui->Scan3Box->text().toStdString().c_str());
    }
    if((ui->Scan4Box->isChecked()&&(!(ui->Scan4Box->text().isEmpty()))))
    {
        fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 6 lt 1 \n",
                ui->Scan4Box->text().toStdString().c_str());
    }
    if((ui->Scan5Box->isChecked()&&(!(ui->Scan5Box->text().isEmpty()))))
    {
        fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 5 lt 1 \n",
                ui->Scan5Box->text().toStdString().c_str());
    }
    if((ui->Scan6Box->isChecked()&&(!(ui->Scan6Box->text().isEmpty()))))
    {
        fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 4 lt 1 \n",
                ui->Scan6Box->text().toStdString().c_str());
    }
    if((ui->Scan7Box->isChecked()&&(!(ui->Scan7Box->text().isEmpty()))))
    {
        fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 3 lt 1 \n",
                ui->Scan7Box->text().toStdString().c_str());
    }
    if((ui->Scan8Box->isChecked()&&(!(ui->Scan8Box->text().isEmpty()))))
    {
        fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 2 lt 1 \n",
                ui->Scan8Box->text().toStdString().c_str());
    }
    if((ui->Scan9Box->isChecked()&&(!(ui->Scan9Box->text().isEmpty()))))
    {
        fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 1 lt 1 \n",
                ui->Scan9Box->text().toStdString().c_str());
    }
    if((ui->Scan10Box->isChecked()&&(!(ui->Scan10Box->text().isEmpty()))))
    {
        fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 9 \n",
                ui->Scan10Box->text().toStdString().c_str());
    }

    fprintf(pipe, "unset multiplot\n");
    fprintf(pipe, "set terminal postscript enhanced color\n");
    fprintf(pipe, "set output \"Scans/LastScanAnalyzed.eps\"\n");

   if((ui->Scan1Box->isChecked()&&(!(ui->Scan1Box->text().isEmpty()))))
   {

       fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 1 lt 0 lw 4\n",
               ui->Scan1Box->text().toStdString().c_str());
       /*fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 1 lt 0 lw 4 title \"%s\"\n",
               ui->Scan1Box->text().toStdString().c_str(),ui->Scan1Box->text().toStdString().c_str());*/

   }
   if((ui->Scan2Box->isChecked()&&(!(ui->Scan2Box->text().isEmpty()))))
   {

       fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 8 lt 1 \n",
               ui->Scan2Box->text().toStdString().c_str());
   }
   if((ui->Scan3Box->isChecked()&&(!(ui->Scan3Box->text().isEmpty()))))
   {
       fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 7 lt 1 \n",
               ui->Scan3Box->text().toStdString().c_str());
   }
   if((ui->Scan4Box->isChecked()&&(!(ui->Scan4Box->text().isEmpty()))))
   {
       fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 6 lt 1 \n",
               ui->Scan4Box->text().toStdString().c_str());
   }
   if((ui->Scan5Box->isChecked()&&(!(ui->Scan5Box->text().isEmpty()))))
   {
       fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 5 lt 1 \n",
               ui->Scan5Box->text().toStdString().c_str());
   }
   if((ui->Scan6Box->isChecked()&&(!(ui->Scan6Box->text().isEmpty()))))
   {
       fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 4 lt 1 \n",
               ui->Scan6Box->text().toStdString().c_str());
   }
   if((ui->Scan7Box->isChecked()&&(!(ui->Scan7Box->text().isEmpty()))))
   {
       fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 3 lt 1 \n",
               ui->Scan7Box->text().toStdString().c_str());
   }
   if((ui->Scan8Box->isChecked()&&(!(ui->Scan8Box->text().isEmpty()))))
   {
       fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 2 lt 1 \n",
               ui->Scan8Box->text().toStdString().c_str());
   }
   if((ui->Scan9Box->isChecked()&&(!(ui->Scan9Box->text().isEmpty()))))
   {
       fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 1 lt 1 \n",
               ui->Scan9Box->text().toStdString().c_str());
   }
   if((ui->Scan10Box->isChecked()&&(!(ui->Scan10Box->text().isEmpty()))))
   {
       fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 9 \n",
               ui->Scan10Box->text().toStdString().c_str());
   }










    if(open==0)QMessageBox::critical(this,"Error","No scans selected");


    else
    {
        fprintf(pipe, "set yrange[%lf:%lf]\n",0.0,actualRange);
        fprintf(pipe, "replot \n");

        fclose(pipe);
    }


}

void LEDMenu::on_RecalculateValueButton_clicked()
{
int state;
QMessageBox::information(this,"Atention","Make sure the imput slit is closed");
state = RecalculateZeroValue(ui->GainComboBox_2->currentIndex());
if(state==0)QMessageBox::information(this,"Zero value","Zero value calculated correctly");

if(state!=0)QMessageBox::critical(this,"Zero Value","Bad response from Zero Value");

}

void LEDMenu::on_ScanWindow_tabBarClicked()
{
    FILE *Scans;

    Scans = fopen("Scans/LastScans","r");
    while(feof(Scans) == 0)
    {
        if(numberScans>=10)
        {
            fscanf(Scans,"%s\n", lastScans[10]);
            strcpy(lastScans[0],lastScans[1]);
            strcpy(lastScans[1],lastScans[2]);
            strcpy(lastScans[2],lastScans[3]);
            strcpy(lastScans[3],lastScans[4]);
            strcpy(lastScans[4],lastScans[5]);
            strcpy(lastScans[5],lastScans[6]);
            strcpy(lastScans[6],lastScans[7]);
            strcpy(lastScans[7],lastScans[8]);
            strcpy(lastScans[8],lastScans[9]);
            strcpy(lastScans[9],lastScans[10]);

        }
        else fscanf(Scans,"%s\n", lastScans[numberScans]);

        numberScans++;
    }
    ui->Scan1Box->setText(lastScans[0]);
    ui->Scan2Box->setText(lastScans[1]);
    ui->Scan3Box->setText(lastScans[2]);
    ui->Scan4Box->setText(lastScans[3]);
    ui->Scan5Box->setText(lastScans[4]);
    ui->Scan6Box->setText(lastScans[5]);
    ui->Scan7Box->setText(lastScans[6]);
    ui->Scan8Box->setText(lastScans[7]);
    ui->Scan9Box->setText(lastScans[8]);
    ui->Scan10Box->setText(lastScans[9]);

    fclose(Scans);
}



void LEDMenu::on_ImportScanButton_clicked()
{
    FILE *Scans;
    if(ui->ScanDirectory->count()==0)
    {
        QMessageBox::critical(this,"Error","No Scans Selected to import");
    }
    else
    {
    Scans = fopen("Scans/LastScans","a+");//Open file to add new scans
    fprintf(Scans,"%s\n",ui->ScanDirectory->item(0)->text().toStdString().c_str());
    fclose(Scans);
    }

    LEDMenu::on_ScanWindow_tabBarClicked();
}



void LEDMenu::on_StartSpinBox_valueChanged(double arg1)
{
    if(arg1>=ui->StopSpinBox->value())
    {
        ui->StopSpinBox->setValue(arg1+1);
    }

}

void LEDMenu::on_Browse1Button_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open File"),"Scans/");
    delete ui->ScanDirectory->item(0);
    ui->ScanDirectory->addItems(fileNames);
}



void LEDMenu::on_Browse2Button_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open File"),"Calibrations/");
    ui->listWidget_2->addItems(fileNames);

}




void LEDMenu::on_buttonBox_2_accepted()
{
    if((ui->UsernameTextBox_2->text().toStdString() == "monocrom")&&(ui->PasswordTextBox_2->text().toStdString()=="SP300i@L022"))
    {

        QMessageBox::information(this,"Password","Password acepted");
            ui->InstallGratButton_2->setVisible(true);
            ui->InstallTurrButton_2->setVisible(true);
            ui->respBox->setVisible(true);

            ui->InstallGratButton_2->setEnabled(true);
            ui->InstallTurrButton_2->setEnabled(true);
            ui->respBox->setEnabled(true);
            ui->GainComboBox_2->setEnabled(true);
            ui->RecalculateValueButton->setEnabled(true);
            ui->Browse2Button->setEnabled(true);
            ui->ImportNewCurveButton->setEnabled(true);


    }
    else QMessageBox::critical(this,"Error","Autentication error");

}



void LEDMenu::on_pushButton_clicked()
{
    PrintRGBspectre();
}

void LEDMenu::on_pushButton_2_clicked()
{
    SeeLumiCurve();
}
