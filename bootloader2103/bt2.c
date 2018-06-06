#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#ifdef __linux__
  #include <asm/termios.h>
#else
  #include <termios.h>
#endif

#include <fcntl.h>
#include <stdio.h>

#define CRYSTAL 14746

#ifndef __linux__
#include "windows.h"
#define true 1
#define false 0

//#define DEV "/dev/ttyS2"
//#define COM "\\\\.\\COM3"

// Variables necesarias
HANDLE idComDev;
DCB dcb = {0};
//------------------------------------------------------------------------------
int AbrirCOM(char * COM_PORT)
{
 idComDev = CreateFile(COM_PORT, GENERIC_READ | GENERIC_WRITE,
                    0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
 if(idComDev ==INVALID_HANDLE_VALUE)
  { printf("Error 1, puerto serie: %d\n", GetLastError()); exit(1); }
 dcb.DCBlength=sizeof(dcb);

 if(!GetCommState(idComDev, &dcb))
  { printf("Error 2, puerto serie\n"); exit(2); }
 dcb.BaudRate = (DWORD) CBR_115200;
 dcb.ByteSize = (BYTE) 8;
 dcb.Parity   = (BYTE) NOPARITY;
 dcb.StopBits = (BYTE) ONESTOPBIT;

 if(!SetCommState(idComDev, &dcb))
  { printf("Error 3, SetCommStatus\n"); exit(3); }

 COMMTIMEOUTS timeouts={0};
 timeouts.ReadIntervalTimeout = 50;
 timeouts.ReadTotalTimeoutConstant = 50;
 timeouts.ReadTotalTimeoutMultiplier = 10;
 timeouts.WriteTotalTimeoutConstant = 50;
 timeouts.WriteTotalTimeoutMultiplier = 10;

 if(!SetCommTimeouts(idComDev,&timeouts))
  { printf("Error 4, timeouts\n"); exit(4); }

 return true;
}
//------------------------------------------------------------------------------
void CerrarCOM()
{
 CloseHandle(idComDev);
}
#else
#define DEV "/dev/ttyUSB0"
#endif

unsigned char boot2[]=
#include "boot2.h"

int ugets(int fd, unsigned char *buf)
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


main(int argc,char **argv)
{
        unsigned char a,dn[128],*imn,fn[256];
        const int baudtable[][2]={
            50,B50,
            75,B75,
            110,B110,
            150,B150,
            200,B200,
            300,B300,
            600,B600,
            1200,B1200,
            2400,B2400,
            4800,B2400,
            9600,B9600,
            19200,B19200,
            38400,B38400,
            57600,B57600,
            115200,B115200,
            921600,B921600,
            1500000,B1500000,
            0,0
        };
        int i,j,k,fd,fdin,dir,baud=115200,result,hex=0,crlf=0,dbase=0x40000000,isp=0,sl;
        struct termios term,tterm;
        unsigned char *p,buf[1024],bn[4];
        static const unsigned char anim[4]={'|','\\','-','/'};

        // Parse args
        strcpy(dn,DEV);
        imn=NULL;
        for (i=1;i<argc;i++) {
            if (*argv[i]=='-') {
                switch(argv[i][1]) {
                case 'h':   printf("Uso: %s opciones\n",argv[0]);
                            printf("Opciones:\n");
                            printf("-h : Help\n");
                            printf("-d <dispositivo_serie> : puerto serie a usar\n");
                            printf("-s <velocidad> : Velocidad en baudios del puerto\n");
                            printf("-l <fichero.bin> : carga fichero imagen\n");
                            printf("-b dir_base(hex) : dirección de carga\n");
                            printf("-i : terminal para ISP\n");
                            printf("-x : volcado hexadecimal de datos recibidos\n");
                            printf("-c : emite \\r\\n en lugar de sólo \\n\n");
                            exit(0);
                case 'd':   strcpy(dn,argv[++i]); break;
                case 's':   baud=atoi(argv[++i]); break;
                case 'x':   hex=1; break;
                case 'c':   crlf=1; break;
                case 'i':   isp=1; break;
                case 'l':   imn=argv[++i]; break;
                case 'b':   dbase=strtoul(argv[++i],NULL,16); break;
                }
            }
        }

        /* ponemos las cosas del puerto en su sitio */

#ifndef __linux__
                for (i=20;i>1;i--) {
                        sprintf(dn,"/dev/ttyS%d",i);
                        //printf(">%s<\n",dn);
                        if ((fd=open(dn,O_RDWR|O_NOCTTY|O_NONBLOCK))>0) goto L1;
                }
                printf("*** Puerto serie no encontrado\n");
                exit(0);
L1:             close(fd);
                sprintf(buf,"\\\\.\\COM%d",i+1);
        AbrirCOM(buf);
        CerrarCOM();
#endif
    printf("Puerto serie: %s\nVelocidad (boot): 115200 baudios\n",dn);

        if ((fd=open(dn,O_RDWR|O_NOCTTY|O_NONBLOCK))==-1) {perror("open"); exit(0);}

        tcflush(fd, TCIOFLUSH);
        bzero(&term,sizeof(term));
        bzero(&tterm,sizeof(tterm));
        term.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
        term.c_iflag = IGNPAR | IGNBRK | IXON | IXOFF;
        term.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
        term.c_lflag = 0;

        cfmakeraw(&term);       // flags for RAW serial mode

        //tcflush(fd,TCIFLUSH);
        if (tcsetattr(fd,TCSANOW,&term)) { fprintf(stderr,"Fallo en tcsetattr\n"); exit(0); }

        // Flush input and output buffers (importante no se lee si no hacemos esto)
        tcsetattr(fd, TCSAFLUSH, &tterm);
        /* reset the tty to its original settings */
        tcsetattr(fd, TCSADRAIN, &term);

        // reset y bootloader H
        if(ioctl(fd, TIOCMGET, &i)) {fprintf(stderr,"Fallo en ioctl TIOCMGET\n"); exit(0);}
        i&=~(TIOCM_DTR|TIOCM_RTS); // Reset y Bootloader H
        if(ioctl(fd, TIOCMSET, &i)) {fprintf(stderr,"Fallo en ioctl TIOCMSET\n"); exit(0);}

        if (imn || isp) {

          // LPC ISP (carga de bootloader 2 en RAM)

          // Intentamos Reset automático
          printf("\n--------------------- LPC24xx ISP --------------------\n");
          if(ioctl(fd, TIOCMGET, &i)) {fprintf(stderr,"Fallo en ioctl TIOCMGET\n"); exit(0);}
          i|=TIOCM_DTR|TIOCM_RTS; // Reset y Bootloader L
          if(ioctl(fd, TIOCMSET, &i)) {fprintf(stderr,"Fallo en ioctl TIOCMSET\n"); exit(0);}
          usleep(200000);
          i&=~TIOCM_DTR;        // Reset H
          if(ioctl(fd, TIOCMSET, &i)) {fprintf(stderr,"Fallo en ioctl TIOCMSET\n"); exit(0);}
          usleep(500000);
          i&=~TIOCM_RTS;        // bootloader H
          if(ioctl(fd, TIOCMSET, &i)) {fprintf(stderr,"Fallo en ioctl TIOCMSET\n"); exit(0);}

          sprintf(buf,"Synchronized\r\n");

          for(k=0;;k++) {
//          do {i=read(fd,&a,1);} while (i==1); // Flush UART buffer
            tcflush(fd, TCIOFLUSH);
            a='?'; write(fd,&a,1);              // Character for autobaud
            //putchar('?'); fflush(stdout);
            usleep(200000);
            p=buf;
            for (;;) {
                j=read(fd,&a,1);
                if (j!=1)  break;
                //putchar(a); fflush(stdout);
                if (a!=*p) break;
                if (*(++p)==0) goto syn2;
            }
            putchar('\r'); putchar(anim[k&3]); fflush(stdout);
          }
syn2:     i=fcntl(fd,F_GETFL); fcntl(fd,F_SETFL,i&(~O_NONBLOCK));
          write(fd,buf,strlen(buf));
          i=ugets(fd,buf);      // borra eco
          printf("\r>%s<\n",buf);
          i=ugets(fd,buf);
          printf(">%s<\n",buf);
          sprintf(buf,"%d\r\n",CRYSTAL);
          write(fd,buf,strlen(buf));
          i=ugets(fd,buf);      // borra eco
          printf(">%s<\n",buf);
          i=ugets(fd,buf);
          printf(">%s<\n",buf);

          sprintf(buf,"A 0\r\n");       // Quitar eco
          write(fd,buf,strlen(buf));
          i=ugets(fd,buf);      // borra eco
          printf(">%s<\n",buf);
          i=ugets(fd,buf);
          printf(">%s<\n",buf);

          sprintf(buf,"U 23130\r\n");   // Unlock programming
          write(fd,buf,strlen(buf));
          printf("%s",buf);
          i=ugets(fd,buf);
          printf(">%s<\n",buf);

          if (isp) {
                sprintf(buf,"A 1\r\n"); // pone eco
                write(fd,buf,strlen(buf));
                printf("%s",buf);
                goto ispterm;
          }

          j=sizeof(boot2);
          j=((j+11)/12)*12; // pad a mÃºltiplo de 12 bytes
          dir=0x40000000+512 ;
          i=0x40000000+8192;            // Final de la RAM en este micro
          boot2[32]=i&0xff; boot2[33]=(i>>8)&0xff;
          boot2[34]=(i>>16)&0xff; boot2[35]=(i>>24)&0xff;

          sprintf(buf,"W %u %u\r\n",dir,j);     // Write to RAM
          write(fd,buf,strlen(buf));
          printf("%s--uuencoded data--\n",buf);

          k=uuenc(fd,boot2,j);

          sprintf(buf,"%u\r\n",k);
          write(fd,buf,strlen(buf));
          printf("%s",buf);
          ugets(fd,buf);
          printf(">%s<\n",buf);
          ugets(fd,buf);
          printf(">%s<\n",buf);

          sprintf(buf,"G %u A\r\n",dir+36);     // Goto
          write(fd,buf,strlen(buf));
          printf("%s",buf);
          ugets(fd,buf);
          printf(">%s<\n",buf);

          // BOOTLOADER 2

          printf("-------------------- Bootloader 2 --------------------\n");
          // Sync

          i=fcntl(fd,F_GETFL); fcntl(fd,F_SETFL,i|O_NONBLOCK);  // flush UART
          do {i=read(fd,&a,1);} while (i==1);
          fcntl(fd,F_SETFL,O_RDWR);
          j=0;
          do {
            buf[0]=0x80; buf[1]=0xaa; buf[2]=0x33; buf[3]=0x55;
            write(fd,buf,4);
            fcntl(fd,F_SETFL,O_RDWR|O_NONBLOCK);
            for (i=a=0;i<10;i++) {
                if (read(fd,&a,1)==1) break;
                usleep(100000);
            }
            i=fcntl(fd,F_GETFL); fcntl(fd,F_SETFL,i&=~O_NONBLOCK);
            if (a!=0x80) {putchar('\r'); putchar(anim[j&3]); fflush(stdout); j++;}
            else puts("\rSYNC OK\n");
          } while (a!=0x80);


          // Write file
          if((fdin=open(imn,O_RDONLY))<=0) {perror("open code.bin"); exit(1);}

          for(dir=dbase,j=0;;) {
            i=read(fdin,buf,1024);
            printf("\r%08X",dir); fflush(stdout);
            if (i<=0) break;
            a=0x81; write(fd,&a,1);
            bn[0]=dir&0xff; bn[1]=(dir>>8)&0xff; bn[2]=(dir>>16)&0xff; bn[3]=(dir>>24)&0xff;
            write(fd,bn,4);
            bn[0]=i&0xff; bn[1]=(i>>8)&0xff; bn[2]=(i>>16)&0xff; bn[3]=(i>>24)&0xff;
            write(fd,bn,4);
            write(fd,buf,i);
            read(fd,&a,1);
            if (a!=0x80) { printf("Bad ACK\n"); exit(2);}
            dir+=i;
            j+=i;
          }
          printf("\nLoad %s OK (%d bytes)\n",imn,j);
          close(fdin);

          // Exec
          dir=dbase;
          a=0x82; write(fd,&a,1);
          bn[0]=dir&0xff; bn[1]=(dir>>8)&0xff; bn[2]=(dir>>16)&0xff; bn[3]=(dir>>24)&0xff;
          write(fd,bn,4);
          read(fd,&a,1);
          if (a!=0x80) { printf("Bad ACK\n"); exit(2);}
          printf("EXEC new code...\n");

        }

        //////////////////////////////////////////////////
        // Terminal
        //////////////////////////////////////////////////

        printf("------------------------- Terminal ------------------------\n");
        for (i=0;baudtable[i][0];i++) {
            if (baud==baudtable[i][0]) break;
        }
        if (!baudtable[i][0]) {printf("Velocidad no soportada\n"); exit(0);}
        printf("Terminal: Velocidad: %d baudios\n",baud);
        term.c_cflag&=~CBAUD;
        term.c_cflag|=baudtable[i][1];
        if (tcsetattr(fd,TCSANOW,&term)) {
                fprintf(stderr,"Fallo en tcsetattr\n");
                exit(0);
        }
ispterm:
        printf("-----------------------------------------------------------\n");
        // non blocking uart
        i=fcntl(fd,F_GETFL); fcntl(fd,F_SETFL,i|O_NONBLOCK);

        /* ponemos las cosas de STDIN en su sitio */
        i=fcntl(0,F_GETFL); fcntl(0,F_SETFL,i|O_NONBLOCK);
        tcgetattr(0,&term);

        term.c_lflag&=~ICANON;
        term.c_lflag&=~ECHO;

        if (tcsetattr(0,TCSANOW,&term)) {
                fprintf(stderr,"Fallo en tcsetattr\n");
                exit(0);
        }

        fdin=0;
        for (;;) {
                sl=2;
                if (read(fd,&a,1)==1) {
                        if (hex) {printf("%02X ",a); fflush(stdout);}
                        else write(1,&a,1);
                } else sl--;
                j=read(fdin,&a,1);
                if (j==0 && fdin) {
                        close(fdin);
                        fdin=0; // Si se acabo el fichero pasamos a stdin
                }
                if (j==1) {
                        switch (a) {
                        case    27:
                                /* ponemos las cosas de STDIN como estaban */
                                tcgetattr(0,&term);
                                term.c_lflag|=ICANON;
                                term.c_lflag|=ECHO;
                                if (tcsetattr(0,TCSANOW,&term))
                                        fprintf(stderr,"Fallo en tcsetattr\n");
                                exit(0);
                        case    'X'-64: a=3;
                                break;
                        case    '\n':   if (crlf) a='\r';
                                break;
                        case    'F'-64: printf("\nInclude File: ");
                                fflush(stdout);
                                tcgetattr(0,&term);
                                term.c_lflag|=ECHO;
                                tcsetattr(0,TCSANOW,&term);
                                i=fcntl(0,F_GETFL); fcntl(0,F_SETFL,i&(~O_NONBLOCK));
                                gets(fn);
                                fcntl(0,F_SETFL,i|O_NONBLOCK);
                                term.c_lflag&=~ECHO;
                                tcsetattr(0,TCSANOW,&term);
                                if ((fdin=open(fn,O_RDONLY))==-1) fdin=0;
                                continue;
                        }
                        write(fd,&a,1); usleep(7000);
                        if (a=='\r') {a='\n'; write(fd,&a,1); usleep(20000);}
                } else sl--;
                if (!sl) usleep(40000);
        }
}

