#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>

// COM9 => ttyS8
#define DEV "/dev/ttyS9" 
#define CRYSTAL 12000

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
    int fork_PID;
	unsigned char a,*dn=DEV,*imn,fn[256];
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
	int i,j,k,fd,fdin,dir,baud=115200,result,hex=0,crlf=0,dbase=0x40000000,isp=0;
	struct termios term;
	unsigned char *p,buf[1024],bn[4];
	static const unsigned char anim[4]={'|','\\','-','/'};

        // Parse args
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
                case 'd':   dn=argv[++i]; break;
                case 's':   baud=atoi(argv[++i]); break;
		case 'x':   hex=1; break;
		case 'c':   crlf=1; break;
		case 'i':   isp=1; break;
		case 'l':   imn=argv[++i]; break;
		case 'b':   dbase=strtoul(argv[++i],NULL,16); break;
                }
            }
        }

        printf("Puerto serie: %s\nVelocidad (boot): 115200 baudios\n",dn);
	

	if ((fd=open(dn,O_RDWR))==-1) {perror("open"); exit(0);}

	/* ponemos las cosas del puerto en su sitio */
	tcgetattr(fd,&term);

	term.c_cflag&=~CBAUD;
	term.c_cflag|=B115200;
	//term.c_cflag&=~CSIZE;
	//term.c_cflag|=CS8;
	//term.c_cflag|=CSTOPB;
	//term.c_cflag&=~PARENB;

	term.c_lflag&=~ICANON;
	term.c_lflag&=~ECHO;
	term.c_lflag&=~ISIG;
	term.c_iflag&=~ICRNL;
	term.c_iflag&=~IXOFF;
	term.c_iflag&=~IXON;
/*	term.c_cflag|=TIOCM_RI; */
	term.c_oflag&=~OPOST;


	if (tcsetattr(fd,TCSANOW,&term)) {
		fprintf(stderr,"Fallo en tcsetattr\n");
		exit(0);
	}

	if (imn || isp) {

	  // LPC ISP (carga de bootloader 2 en RAM)

	  printf("\n--------------------- LPC24xx ISP --------------------\n");
	  sprintf(buf,"Synchronized\r\n");
	  i=fcntl(fd,F_GETFL); fcntl(fd,F_SETFL,i|O_NONBLOCK);
	  for(k=0;;k++) {
	    do {i=read(fd,&a,1);} while (i==1); // Flush UART buffer 
	    a='?'; write(fd,&a,1);		// Character for autobaud
	    usleep(100000);
	    p=buf;
	    for (;;) {
		j=read(fd,&a,1);
		if (j!=1) break;
		if (a!=*p) break;
		if (*(++p)==0) goto syn2;
	    }
	    putchar('\r'); putchar(anim[k&3]);
	    fflush(stdout);
	  }
syn2:	  i=fcntl(fd,F_GETFL); fcntl(fd,F_SETFL,i&(~O_NONBLOCK));
	  write(fd,buf,strlen(buf));
	  i=ugets(fd,buf);	// borra eco
	  printf("\r>%s<\n",buf);
	  i=ugets(fd,buf);
	  printf(">%s<\n",buf);
	  sprintf(buf,"%d\r\n",CRYSTAL);
	  write(fd,buf,strlen(buf));
	  i=ugets(fd,buf);	// borra eco
	  printf(">%s<\n",buf);
	  i=ugets(fd,buf);	
	  printf(">%s<\n",buf);

	  sprintf(buf,"A 0\r\n");	// Quitar eco
	  write(fd,buf,strlen(buf));
	  i=ugets(fd,buf);	// borra eco
	  printf(">%s<\n",buf);
	  i=ugets(fd,buf);	
	  printf(">%s<\n",buf);

	  sprintf(buf,"U 23130\r\n");	// Unlock programming
	  write(fd,buf,strlen(buf));
	  printf("%s",buf);
	  i=ugets(fd,buf);
	  printf(">%s<\n",buf);

	  if (isp) {
		sprintf(buf,"A 1\r\n");	// pone eco
	  	write(fd,buf,strlen(buf));
		printf("%s",buf);
		goto ispterm;
	  }

	  j=sizeof(boot2);
	  j=((j+11)/12)*12; // pad a múltiplo de 12 bytes
	  dir=0x40000000+512 ;
	  i=0x40010000;		// Final de la RAM en este micro
	  boot2[32]=i&0xff; boot2[33]=(i>>8)&0xff;
	  boot2[34]=(i>>16)&0xff; boot2[35]=(i>>24)&0xff;

	  sprintf(buf,"W %u %u\r\n",dir,j);	// Write to RAM
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

	  sprintf(buf,"G %u A\r\n",dir+36);	// Goto
	  write(fd,buf,strlen(buf));
	  printf("%s",buf);
	  ugets(fd,buf);
	  printf(">%s<\n",buf);

	  // BOOTLOADER 2

          printf("-------------------- Bootloader 2 --------------------\n");
	  // Sync

	  fcntl(fd,F_SETFL,O_RDWR|O_NONBLOCK);	// flush UART
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
	    fcntl(fd,F_SETFL,O_RDWR);
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
        baud=baudtable[i][1];
	term.c_cflag&=~CBAUD;
	term.c_cflag|=baud;
	if (tcsetattr(fd,TCSANOW,&term)) {
		fprintf(stderr,"Fallo en tcsetattr\n");
		exit(0);
	}
ispterm:
	printf("-----------------------------------------------------------\n");

	/* ponemos las cosas de STDIN en su sitio */
	tcgetattr(0,&term);

	term.c_lflag&=~ICANON;
	term.c_lflag&=~ECHO;

	if (tcsetattr(0,TCSANOW,&term)) {
		fprintf(stderr,"Fallo en tcsetattr\n");
		exit(0);
	}


	result=0;
	result|=TIOCM_RTS;
	ioctl (fd, TIOCMBIC, &result);



	if (!(fork_PID=fork())) {
		read(fd,&a,1);
		for (i=0;;){
			read(fd,&a,1);
			if (hex) {printf("%02X ",a); fflush(stdout);}
			else write(1,&a,1);
		}
	} else {
		for (;;) {
			read(0,&a,1);
			switch (a) {
		case 	'X'-64:	a=3;
				write(fd,&a,1);
				break;
		case	'\n':	if (crlf) { a='\r'; write(fd,&a,1);}
				a='\n';
				write(fd,&a,1);
				break;				
		case	'F'-64:	printf("\nInclude File: ");
				fflush(stdout);
				tcgetattr(0,&term);
				term.c_lflag|=ECHO;
				tcsetattr(0,TCSANOW,&term);
				fgets(fn,255,stdin);
				term.c_lflag&=~ECHO;
				tcsetattr(0,TCSANOW,&term);
				if ((fdin=open(fn,O_RDONLY))==-1) break;
				while ((i=read(fdin,&a,1))>0) {
				        if (a=='\n' && crlf) {
					    a='\r';
					    write(fd,&a,i);
					    usleep(10000);
					    a='\n';
					}
					write(fd,&a,i);
					usleep((a=='\n')?10000:5000);
					//if (a=='\n') usleep(20000);
				}
				//putchar('\n'); fflush(stdout);
				close(fdin);
				break;

        case 27: 
        	tcgetattr(0,&term);

            term.c_lflag|=ICANON;
            term.c_lflag|=ECHO;

            if (tcsetattr(0,TCSANOW,&term)) {
                fprintf(stderr,"Fallo en tcsetattr\n");            
            exit(0);
            }
            close(fd);
            
            kill( fork_PID, SIGKILL );
            return 0;
            break;
		default:	write(fd,&a,1);
			}
		}
	}
}

