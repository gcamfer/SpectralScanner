#include "globals.h"
#include <QString>
#include <stdio.h>
#include <QMessageBox>

#include <unistd.h>     // UNIX standard function definitions
#include <sys/ioctl.h>
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>
#include "ledmenu.h"

// ALL THE GLOBAL DEFINITIONS
time_t tiempo = time(0);
bool connected=false;
FILE *fp;
struct tm *tlocal = localtime(&tiempo);
char output[128];
int fd, n;
char fileName[256]="There is no name seted";
char responsivityCurveFileName[256]="Calibrations/SpectralResponse.txt";
char responsivityDetector[256]="";
char thermistorName[256]="";
bool changedName=false;
char lastScans[10][256];//Last 5 scans saved
int numberScans=0;
char ScanBoxName[256]="";
float lambdaMAX; //Max value lambda for analysis
float PoptMAX; //Max pot for max lambda
int errors=0;



int ugets(int fd,  char *buf)
{
    int i=0;
    unsigned char a;

    for(;;) {
        read(fd,&a,1);
        if (a=='\n') break;
        if (a=='\r') continue;
        buf[i++]=a;
    }
    buf[i++]=0;
    return --i;
}

void uputch(int fd,unsigned char a)
{
    write(fd,&a,1);
}

int uuenc(int fd,unsigned char *buf,int n)
{
    int i,j,ck;
    unsigned char a,b,c,*p;

    p=buf;
    ck=0;
    do {
        j=45;
        if (n<j) j=n;
        uputch(fd,32+j);
        for (i=j/3;i;i--) {
            a=*p++; b=*p++; c=*p++;
            uputch(fd,32+(a>>2));
            uputch(fd,32+((a&0x3)<<4)+(b>>4));
            uputch(fd,32+((b&0xf)<<2)+(c>>6));
            uputch(fd,32+(c&0x3f));
            ck+=a+b+c;
        }
        uputch(fd,'\n');
        n-=j;
    } while (n);
    return ck;
}




///////////////////////////////////////////////////////
/// \brief ConnectNow
///////////////////////////////////////////////////////
///
///         CONECTION WITH uC
///////////////////////////////////////////////////////


int ConnectNow()
{
    struct termios term;
    if(connected==false)
    {

        fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
        if (fd == -1) {
                perror("open_port: Unable to open /dev/ttyUSB0 - ");
                return -1;

            }
            if ((fp = fdopen(fd, "rw")) == NULL)
            {
                perror("fdopen");
                return -1;

            }
            // Flags.
            tcflush(fd, TCIOFLUSH);
            bzero(&term, sizeof(term));
            term.c_cflag = B115200; //Bauds


            term.c_cflag |= CS8 | CLOCAL | CREAD;
            term.c_iflag = IGNPAR | IGNBRK | IXON | IXOFF;
            term.c_oflag = 0;


            /* set input mode (non-canonical, no echo,...) */
            term.c_lflag = 0;

            cfmakeraw(&term); // flags for RAW serial mode


            if (tcsetattr(fd, TCSANOW, &term)) {
                fprintf(stderr, "Fail tcsetattr\n");
                return -1;
            }

            // reset  bootloader H
            if (ioctl(fd, TIOCMGET, &n)) {
                fprintf(stderr, "Faail ioctl TIOCMGET\n");
            }
            n &= ~(TIOCM_DTR | TIOCM_RTS); // Reset  Bootloader H


            if (ioctl(fd, TIOCMSET, &n)) {
                fprintf(stderr, "Fail ioctl TIOCMSET\n");
                return -1;
            }
        connected=true;
        return 0;
  }

    else return 0;
}




int RecalculateZeroValue(int index)
{

    float Startlambda, Stoplambda,resollution,lambda;
    int SelectGains[4] = {00,01,10,11};//47,94,194,414
    static double gainVector[15]={47,94,194,414,10000,100000,1000000,10000000,
                               100000000,1000000,10000000,100000000,1000000000,10000000000};
    float measure,gain;
    int selection;
    int n;
    char str[26]="\0";
    char *ret;
    static char buf[256],ZeroName[128];

    FILE *ScanFile;



    if((connected==true))
        {
        Startlambda=35000;//For a complete integer value
        Stoplambda=108000;
        resollution=200;
        gain=gainVector[index];
        if(gain==47)selection=0;
        else if(gain==94)selection=1;
        else if(gain==194)selection=2;
        else if(gain==414)selection=3;
        else selection=0;

        if(index<4)
        {
            sprintf(ZeroName, "Calibrations/ZeroValueX%.0f", gain);
        }
        else if((index>3) &&(index<9))
        {
            sprintf(ZeroName, "Calibrations/ZeroValueX%.0fLN", gain);
        }
        else
        {
            sprintf(ZeroName, "Calibrations/ZeroValueX%.0fHS", gain);
        }
        ScanFile = fopen(ZeroName, "w");

        fprintf(ScanFile,"%s %s","#length(nm)\t", "Measure(mV)\n"); // Writing the header
        setlocale(LC_ALL, "C");
        sprintf(str,"LEDSC%.6d%.6d%.6d%.2d\n",int(Startlambda),int(Stoplambda),int(resollution),SelectGains[selection]);
        n = write(fd, str, strlen(str));                        //SEND FUNCTION & PARAMETERS
        if (n < 0)fputs("write() LEDSC failed!\n", stderr);

        n=ugets(fd, buf);                              //READ "Scanning LED mode"

        do{

            memset(buf, '\0', sizeof(buf));
            n=ugets(fd, buf);                              //READ VALUE
            ret = strstr(buf, "OK");
            if((ret==NULL)&&(n!=0))
            {

                strncpy(str,buf,7);//This is the lambda part next \t and measure
                lambda=atof(str);
                memset(str, '\0', sizeof(str));//Clear string
                strncpy(str,buf+8,4);//Part of value obtained
                measure=atof(str);
                fprintf(ScanFile, "%.2f\t%f\n", lambda, measure); // Writing the file
            }
            n=write(fd,"ACK\n",strlen("ACK\n"));  //Acknowledge to value and solicite next

        }while(ret==NULL);

            if(strcmp(buf,"AOK")==0)
            {
            fclose(ScanFile);
            return 0;
            }
            else
            {
            return -1;
            fclose(ScanFile);
            }

        }
    return -2;
}


void SeeRespCurve()
{
    FILE *pipe;
    FILE *responsivityCurve;

    responsivityCurve = fopen(responsivityCurveFileName, "r");
    fscanf(responsivityCurve,"%s\n",responsivityDetector);
    fclose(responsivityCurve);

    /*FILE *curve;
    float start,stop=1100,res;
    curve=fopen(responsivityCurveFileName, "r");
    fscanf(curve,"%f %f",&start, &res);
    while((!feof(curve)))//adjust xrange
    {
        fscanf(curve,"%f %f",&stop, &res);
    }
    fclose(curve);*/

   // fscanf("%s\n",responsivityDetector);

    // Code for gnuplot pipe
        pipe = popen("gnuplot -persist", "w");
        fprintf(pipe, "\n");
        fprintf(pipe, "set xrange[%d:%d]\n",300,1100);
        fprintf(pipe, "set xlabel \"Wavelength (nm)\" \n");
        fprintf(pipe, "set ylabel \"Responsivity (A/W)\" \n");
        fprintf(pipe, "set title \"TYPICAL SPECTRAL RESPONSE FOR %s\" \n",responsivityDetector);
        fprintf(pipe, "unset key\n");
        fprintf(pipe, "set mxtics 5\n");
        fprintf(pipe, "set mytics 5\n");
        fprintf(pipe, "set grid mxtics mytics xtics ytics\n");
        fprintf(pipe, "plot \"%s\" with lines\n", responsivityCurveFileName);
        fprintf(pipe, "set terminal postscript enhanced color\n");
        fprintf(pipe, "set output \"Calibrations/SpectralResponse.eps\"\n");
        fprintf(pipe, "replot\n");
        fclose(pipe);
}

void SeeLumiCurve()
{
    FILE *pipe;


        pipe = popen("gnuplot -persist", "w");
        fprintf(pipe, "\n");
        fprintf(pipe, "set xrange[%d:%d]\n",300,1100);
        fprintf(pipe, "set xlabel \"Wavelength (nm)\" \n");
        fprintf(pipe, "set ylabel \"Relative Efficiency\" \n");
        fprintf(pipe, "set title \"Luminous efficiency @ http://www.cvrl.org/\" \n");
        fprintf(pipe, "unset key\n");
        fprintf(pipe, "set mxtics 5\n");
        fprintf(pipe, "set mytics 5\n");
        fprintf(pipe, "set grid mxtics mytics xtics ytics\n");
        fprintf(pipe, "plot \"%s\" using ($1):($2) w l lc 1 lt 1 lw 2 \n","Calibrations/LuminousEfficiency.txt");
        fprintf(pipe, "set terminal postscript enhanced color\n");
        fprintf(pipe, "set output \"Calibrations/LuminousEfficiency.eps\"\n");
        fprintf(pipe, "replot\n");
        fclose(pipe);
}

void PrintRGBspectre()
{
    FILE *pipe;


    pipe = popen("gnuplot -persist", "w");
    fprintf(pipe, "\n");
    fprintf(pipe, "set multiplot\n");
    //fprintf(pipe, "set xrange[%d:%d]\n",300,1100);
    fprintf(pipe, "set xlabel \"Wavelength (nm)\" \n");

    fprintf(pipe, "set title \"Cone Fundamentals @ http://www.cvrl.org/\" \n");
    fprintf(pipe, "unset key\n");
    fprintf(pipe, "set mxtics 5\n");
    fprintf(pipe, "set mytics 5\n");
    fprintf(pipe, "set grid mxtics mytics xtics ytics\n");

    fprintf(pipe, "plot \"%s\" using ($1):($2) w l lc 1 lt 1 lw 2 \n","Calibrations/RGBtab.txt");

    fprintf(pipe, "plot \"%s\" using ($1):($3) w l lc 2 lt 1 lw 2 \n","Calibrations/RGBtab.txt");

    fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 3 lt 1 lw 2 \n","Calibrations/RGBtab.txt");

    fprintf(pipe, "unset multiplot\n");
    fprintf(pipe, "set terminal postscript enhanced color\n");
    fprintf(pipe, "set output \"Calibrations/RGB.eps\"\n");
    fprintf(pipe, "set multiplot\n");

    fprintf(pipe, "plot \"%s\" using ($1):($2) w l lc 1 lt 1 lw 2 \n","Calibrations/RGBtab.txt");

    fprintf(pipe, "plot \"%s\" using ($1):($3) w l lc 2 lt 1 lw 2 \n","Calibrations/RGBtab.txt");

    fprintf(pipe, "plot \"%s\" using ($1):($4) w l lc 3 lt 1 lw 2 \n","Calibrations/RGBtab.txt");

    fclose(pipe);

}



float zeroChecker(float lambda,int index)
{
    //R=Iph/Popt
    bool found=false;
    float value,X[2],Y[2];
    FILE *zeroValues;
    char nameFile[128],header[128];

    static double gainVector[15]={47,94,194,414,10000,100000,1000000,10000000,
                               100000000,1000000,10000000,100000000,1000000000,10000000000};
    float gain;
    gain=gainVector[index];

    if(index<4)
    {
        sprintf(nameFile, "Calibrations/ZeroValueX%.0f", gain);
    }
    else if((index>3) &&(index<9))
    {
        sprintf(nameFile, "Calibrations/ZeroValueX%.0fLN", gain);
    }
    else
    {
        sprintf(nameFile, "Calibrations/ZeroValueX%.0fHS", gain);
    }

    zeroValues = fopen(nameFile, "r");


   X[0]=0;
   X[1]=0;
   Y[0]=0;
   Y[1]=0;

   fgets(header,sizeof(header),zeroValues);

    while((!feof(zeroValues))&&(!found))
    {
        if((lambda>X[0])&&(lambda>X[1]))//Not reached the lambda --> get new lambdas
            {
                X[0]=X[1];//Actualization the first lambda
                Y[0]=Y[1];
                setlocale(LC_ALL, "C");
                fscanf(zeroValues,"%f %f\n",&X[1], &Y[1]);
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
    fclose(zeroValues);

    if (!found)
    {
        fprintf(stderr, "Lambda not found in ZeroValue file\n");
        return 0.0;
    }


    return (value);
}



float responsivityChecker(float lambda)
{
    //R=Iph/Popt
    bool found=false;
    float value,X[2],Y[2];
    FILE *responsivityCurve;
    responsivityCurve = fopen(responsivityCurveFileName, "r");

   X[0]=0;
   X[1]=0;
   Y[0]=0;
   Y[1]=0;

   fgets(responsivityDetector,sizeof(responsivityDetector),responsivityCurve);

    while((!feof(responsivityCurve))&&(!found))
    {
        if((lambda>X[0])&&(lambda>X[1]))//Not reached the lambda --> get new lambdas
            {
                X[0]=X[1];//Actualization the first lambda
                Y[0]=Y[1];
                setlocale(LC_ALL, "C");
                fscanf(responsivityCurve,"%f %f\n",&X[1], &Y[1]);
                //printf("%f\t%f\n", X[1], Y[1]);
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
    fclose(responsivityCurve);

    if (!found)
    {
        fprintf(stderr, "Lambda not found in Responsivity file\n");
        return 0;
    }


    return (value);
}
