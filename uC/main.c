// ========================================================
// Programa LPC2103 para TFG caracterización diodos laser
// Autor: Guillermo Caminero Fernandez
// Fecha: 13-03-2016
// Última modificación: 09/02/2017
// Versión: 3.1
// ========================================================

#include <stdio.h>
#include <string.h>
#include "lpc2103.h"


/**********************************************************
                    Function prototypes
 **********************************************************/
 
void _prtnum(unsigned int);
//void _U0prtnum(unsigned int);
void _U0prthex(unsigned int);
void _U0prthex8(char);
void _U0puts(char *);
void _U0putch(char);
char _U0getch();
void _delay_loop(unsigned int);



void _puts(char *);
void _printf(char *,...);
void _sprintf(char *, char *,...);
extern void (*_vputch)(int);
extern int (*_vgetch)();
//#define _putch(d) ((*_vputch)(d))
//#define _getch() ((*_vgetch)())
void _putch(char);
char _getch();
int	_gets(unsigned char *,int);
//void IRQ_Timer0_Routine (void)    __attribute__ ((interrupt("IRQ")));
//void IRQ_Spurious_Routine (void)  __attribute__ ((interrupt("IRQ")));



//========================================================
// CLK DEFINITIONS
//========================================================

#define FOSC    14745600        // Crystal frequency
#define MSEL    4               // PLL multiplier
#define CCLK    (FOSC*MSEL)	    // CPU clock
#define PCKDIV  1               // APB divider
#define PCLK    (CCLK/PCKDIV)   // Peripheral clock
// delays
#define _delay_us(n) _delay_loop(CCLK/4000000*n-1)
#define _delay_ms(n) _delay_loop(CCLK/4000*n-1)



//========================================================
// DEFINITIONS
//========================================================

#define HTC_LDD_ONOFF 27
#define HTC_LDD_ON IOSET = (1<<HTC_LDD_ONOFF)
#define HTC_LDD_OFF IOCLR = (1<<HTC_LDD_ONOFF)

#define MODE 28
#define MODE_CONSTANT_CURRENT IOSET = (1<<MODE)
#define MODE_CONSTANT_POWER IOCLR = (1<<MODE)

#define _LDAC 19
#define _SHDN 20
#define _LDAC0 IOCLR = (1<<_LDAC)
#define _LDAC1 IOSET = (1<<_LDAC)
#define _SHDN0 IOCLR = (1<<_SHDN)
#define _SHDN1 IOSET = (1<<_SHDN)

#define LEDLAMP_ONOFF 21
#define LEDLAMP1 IOSET = (1<<LEDLAMP_ONOFF)
#define LEDLAMP0 IOCLR = (1<<LEDLAMP_ONOFF)

#define GAIN0 15 
#define GAIN1 16
#define GAIN0_0 IOCLR = (1<<GAIN0)
#define GAIN0_1 IOSET = (1<<GAIN0)
#define GAIN1_0 IOCLR = (1<<GAIN1)
#define GAIN1_1 IOSET = (1<<GAIN1)

#define LD 0
#define LED 1
#define LAMP 2


#define MAX_CHAR 30
#define SCPARAM 6 //Parameter without comma
#define GTPARAM 7 //Parameter with comma

/*ADC CONSTANTS*/
#define VREF 3.3
#define FULLSCALE 1023
	//Channels:
#define PD_MEASURE_CH 0
#define CURRENT_MONITOR_CH 1
#define POWER_MONITOR_CH 2
#define PDANODE_CH 3
#define LDCATHODE_CH 4
#define LDANODE_CH 5
#define ACTUAL_TEMP_MONITOR_CH 6


/*Channel control for DAC*/
#define TempChannel 0 //Value for setting temperature
#define LDDChannel 1 //Value for the power/current source LDD




//========================================================
// TIMER0
//========================================================
////////////////////////////
void TIMER0_Init (/*int uStime*/){  
	//PCLK    (CCLK/PCKDIV)   // Peripheral clock
	T0TC = 0; 		  	//Timer counter 
	T0PC = 0; 	       	//Prescaler counter 
	T0PR = ((59-1)); 			//Prescaler 1 us -> 58.982, take real 59 time real = 1.00029us 
	T0TCR = 0b00000011; //Timer control register  Timer1 enabled and reset
	T0MR0 = (310-1);				//Matching register = time vector
	T0MCR = 0b0000000000000011; //Bits [2-0]: MR0 control:
								//Bit 0: Generate interrupt on matching if 1
								//Bit 1: Reset TxTC on matching if 1
								//Bit 2: Stop TxTC on matching if 1
	T0TCR=1;            // Reset released, start counting
	 
 
   // Interrupt setup
   // VICIntEnable=(1<<4);                        		// Timer 0 interrupt enable
   // VICDefVectAddr=(unsigned int)IRQ_Spurious_Routine;  // unvectored IRQs
   // VICVectAddr0=(unsigned int)IRQ_Timer0_Routine;      // Vector for slot 0
   // VICVectCntl0=(1<<5)|4;                              // Slot 0 for Timer0 and enabled
	 
	 // Enable both IRQ interrupts
    //asm volatile ("mrs r0,cpsr\n bic r0,r0,#0x80\n msr cpsr,r0");
}	



//========================================================
// UART0
//========================================================
////////////////////////////
//	UART0 INITIALIZATION
////////////////////////////
void UART0_Init(){
	U0LCR = 0x83;  // 10000011 Line Control Register
					//       .. Character length 11 = 8 bits
					//      .   Stop bits         0 = 1 bit
					//     .    Parity enable     0 = No parity
					//    .     Even parity       non sense
					//   .      Stick parity      non sense
					//  .       Break control     0 = No break
					// .        DLAB              1 = Access DLM,DLL
	U0DLL = 32;    
	U0DLM =  0;    // Baud = 4*14745600/(16* 32) = 115200
	U0LCR = 0x03;  // Same a above, but DLAB = 0 = Normal access 
	U0FCR = 0x07;  // 00000111 FIFO Control Register
					//        . FIFO enable       1 = TX&RX FIFO on 
					//       .  RX FIFO Reset     1 = Reset RX FIFO
					//      .   TX FIFO Reset     1 = Reset TX FIFO
					// ..       RX int.t trigger level 00 = 1 byte
};	
// -------------------------------------------------------
////////////////////////////
//	Send char 8bit
////////////////////////////
void UART0_putch(unsigned char c){
	while(!(U0LSR&0x20)); // while U0LSR.THRE == 0 
	U0THR = c;
}
// -------------------------------------------------------
////////////////////////////
//	Receive char 8bit
////////////////////////////
char UART0_getch(){
	while(!(U0LSR&0x01)); // while U0LSR.DR == 0
	return U0RBR;
}
// -------------------------------------------------------
////////////////////////////
//	Send string
////////////////////////////
void SendPC(char* s){
	while(*s){   
        UART0_putch(*s++);
    }
UART0_putch('\n');//End of line character to fgets func
}

//========================================================
// UART1
//========================================================
////////////////////////////
//	UART1 INITIALIZATION
////////////////////////////
void UART1_Init(){
	U1LCR = 0x83;  // 10000011 Line Control Register
					//       .. Character length 11 = 8 bits
					//      .   Stop bits         0 = 1 bit
					//     .    Parity enable     0 = Parity off
					//    .     Even parity       0 = non sense
					//   .      Stick parity      0 = non sense
					//  .       Break control     0 = No break
					// .        DLAB              1 = Access DLM,DLL
	U1DLL = 128;
	U1DLM =  1;    // Baud = 4*14745600/(16* 384) = 9600

	U1LCR = 0x03;  // Same a above, but DLAB = 0 = Normal access 
	U1FCR = 0x07;  // 00000111 FIFO Control Register
					//        . FIFO enable       1 = TX&RX FIFO on 
					//       .  RX FIFO Reset     1 = Reset RX FIFO
					//      .   TX FIFO Reset     1 = Reset TX FIFO
					// ..       RX int. trigger level 00 = 1 byte
}
// -------------------------------------------------------------
////////////////////////////
//	Send character 8bit
////////////////////////////
void UART1_putch(unsigned char c){ 
	while(!(U1LSR&0x20));	// while U0LSR.THRE == 0 
	U1THR = c;
}

// ---------------------------------------------------
////////////////////////////
//	Receive character 8bit
////////////////////////////
char UART1_getch(){	
	while (!(U1LSR & 0x01)); // while U1LSR.DR == 0
	return U1RBR;
}
// ---------------------------------------------------------------

void SendMONO(char* s){
	while(*s) UART1_putch(*s++);
	//UART0_putch('\n');//End of line character 
}

//========================================================
// SPI
//========================================================
#define SS_Enable	IOPIN &=~(1<<7)
#define SS_Disable	IOPIN |= (1<<7)
#define SPIF (S0SPSR&(1<<7))
///////////////////////////////////////////////////////////////////////
//	SPI INITIALIZATION
///////////////////////////////////////////////////////////////////////
void SPI_Init(){
S0SPCR = 0x0024;		//000000100100 Control Reg.
						//          .. Reserved 
						//         .   BitEnable         1 = >8 bit
						//        .    CPHA = 0   First clock edged  
						//       .     CPOL = 0   sck high   
						//      .      MSTR = 1 Master
						//     .       LSBF = 0 MSB first   
						//    .        SPIE = 0 INT disabled             
						//....		   BITS		Transfer length (0000=16)
	
S0SPCCR = 59;  			// 8-bit clock frequency divider
						//Fsck=PCLK/S0SPCCR=1MHz --> S0SPCCR=4*14745600/1e6=58.98=59
						//Fsck = 14745600*4/59=999701 aprox 1MHz
}
///////////////////////////////////////////////////////////////////////
// SPI Transition
///////////////////////////////////////////////////////////////////////
unsigned char SPI_transition(unsigned int c){
	S0SPDR = (c&0xFFFF);
	while (!SPIF);
	return S0SPDR; //always dummy bits returned
}

///////////////////////////////////////////////////////////////////////
// SPI WRITE
///////////////////////////////////////////////////////////////////////
void DAC_Convert(char channel , int level){
	static int data;
	// Channel 0 = A ; Channel 1 = B
	//Vout = Vref * D / 4096 = 5V * D / 4096
    //Channel can be 0 or 1 --> 1 bit
    data = 0x3000;		//c011xxxxxxxxxxxx Control Reg.
						//    xxxxxxxxxxxx DATA 
						//   .             _SHDN 1 Active mode operation
						//  .              GA --> 1x (Vref=5 0->5V)
						// .               Buffer -->0 NO buffer 
						//.                channel

    data |= ((~channel&0x01)<<15);	//Setting channel

    data |= (level&0x0FFF);			//Setting level data 12bits FFF

    SS_Enable;						//Slave select
    SPI_transition(data);			//Send data information
	SS_Disable;						//Slave unselected
	
	_LDAC0;							//Habilite output latch
	_delay_us(5);		
	_LDAC1;							//Down line
}	



//========================================================
// ADC
//========================================================
//unsigned int ADC_Convert(unsigned char channel = 1, unsigned int repeats = 1); 
//default values, function prototipe
int ADC_Convert(unsigned char channel, unsigned int repeats){
	static unsigned int val;
    int measure=0;
    unsigned char i;
	unsigned char ADCR_SEL = (1<<channel);
	unsigned char ADCR_CLKDIV = 0x0D; //PCLK = 13+1 for ADC clock = 4*14745600/(13+1)=4.21exp6
	unsigned char PDN_CLKS_BURST = 0x20;// 00100000 
							//        . Burst = 0 Conversions are software controlled and require 11 clocks.
							//     ...  Number of clocks used 000 = 11clocks/10bits
							//    .    	Reserved = 0
							//   .     	PDN      1 The A/D converter is operational. 0 The A/D converter is in power-down mode.
							// ..       Reserved = 0

	//10 bit conversion time = 11 clock cycles ~= 0.18649631uS
	
	char EDGE_START = 0x00;	// 00000000 
							//      ... Start conversion Now = 000 
							//     .  	Edge not significant 0
							// ....    	Reserved
	ADCR = (EDGE_START<<24)|(PDN_CLKS_BURST<<16)|(ADCR_CLKDIV<<8)|(ADCR_SEL<<0); 	
    for(i=0;i<repeats;i++){
        ADCR  |= 0x01000000;                   /* Start A/D Conversion               */
        //while ((ADGDR & 0x80000000) == 0); 	   /*Wait for the conversion to complete */
        //val = ((ADGDR >> 6) & 0x03FF);	       /*Extract the A/D result 	         */
        //return val;
        do{
            val = ADGDR; // Read A/D Data Register
        } while ((val & 0x80000000) == 0); // Wait for end of A/D Conversion
        measure += (val >> 6) & 0x03FF; // bit 6:15 is 10 bit AD value
    	_delay_us(310);
    }
return (measure/repeats);
}

////////////////////////////
//	Helpfull funtions
////////////////////////////
// reverses a string 'str' of length 'len'
void reverse(char *str, int len){
    int i=0, j=len-1, temp;
    while (i<j){
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}
 
 // Converts a given integer x to string str[].  d is the number
 // of digits required in output. If d is more than the number
 // of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d){
    int i = 0;
    while (x){
        str[i++] = (x%10) + '0';
        x = x/10;
    }
 
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d) str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}
int aToInt(char *str){
    int i=0;
	while(*str){   
        i=i*10;
        i= i+((*str++)-48);
    }
return i;
}
// Converts a floating point number to string.
/*void ftoa(float n, char *res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
 
    // Extract floating part
    float fpart = n - (float)ipart;
 
    // convert integer part to string
    int i = intToStr(ipart, res, 0);
 
    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot
 
        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * pow(10,afterpoint);
 
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}
*/

////////////////////////////
////////////////////////////

void ReadPC(char *T){
	unsigned char i=0, c;
	do{
		c=UART0_getch();
		//UART0_putch(c);		//Echo
		//_U0prthex8(c);
		T[i] = c;
		i++; 
	#ifndef REAL  	
	}while((c!='\r') && (i<MAX_CHAR));		//Condition for simulations
	#else   	
	}while((c!='\n') && (i<MAX_CHAR));		//Condition for implementation
	#endif	
	T[i-1]=0;	//Sobreescribe retorno de carro (\n) con NULL caracter
}


void ReadMONO(char *T){
	unsigned char i=0, c;
	do{
		c = UART1_getch();
		T[i] = c;
		i++;
	#ifndef REAL  	
	}while((c!='\r') && (i<MAX_CHAR));		//Condition for simulations
	#else    	
	}while((T[i-1]!='\n')||(T[i-2]!='\r'));		//Condition for implementation read /n & /r to
												//eliminate it from buffer...
	T[i-2]='\0';
	T[i-1]='\0';
	#endif	
}

char GoToLED(char *parameters){ //Function goto determinate lambda
	static char T[MAX_CHAR];
	static char lambda[7];
	char *ret;
	char dog;
	
	strncpy(lambda,parameters,7);
	dog=0;
	do{
		if (dog>=20){
			SendMONO("MONO-STOP\r");
			memset(T, '\0', sizeof(T));
			ReadMONO(T);
			return 1;
		}
		//SEND LAMBDA TO GO
		//SendPC(lambda);									//Debugg	
		memset(T, '\0', sizeof(T));			
		SendMONO(lambda);
		SendMONO(" >goto\r");
		ReadMONO(T);
		ret = strstr(T, "ok");
		//SendPC("Response to goto");SendPC(T);			//Debugg
		_delay_ms(100);
		dog++;
	}while(ret==NULL);

	//PROCESS CONTROL FLOW --> MAKE SURE THE MONOCHROMER IS IN THE CORRECT LAMBDA
	dog=0;								//Control the number of tries 20s to fail
	do{
		if(dog>=20){
			SendMONO("MONO-STOP\r");
			memset(T, '\0', sizeof(T));
			ReadMONO(T);
			return 1;
		}	
		_delay_ms(1000);					//1s delay
		//ASK LENGTH FOR PC SOFTWARE
		SendMONO("?nm\r");
		memset(T, '\0', sizeof(T));
		ReadMONO(T);
		ret = strstr(T, "ok");
		if(ret==NULL){			
			SendMONO("MONO-STOP\r");
			memset(T, '\0', sizeof(T));
			ReadMONO(T);
			return 4;
		}
		//SendPC("Response to ?nm");SendPC(T);			//Debugg
		SendMONO("mono-?done\r");
		memset(T, '\0', sizeof(T));
		ReadMONO(T);
		ret = strstr(T, "ok");
		if(ret==NULL){
			SendMONO("MONO-STOP\r");
			memset(T, '\0', sizeof(T));
			ReadMONO(T);
			return 3;					//Need a ok response else error
		}
		ret = strstr(T, "mono-?done 1  ok");			// 1 ok --> finish ; 0 ok --> in process
		//SendPC("Response to mono-?done");SendPC(T);		//Debugg
		dog++;							// +1 second next try
	}while(ret==NULL);						//While command end is not reached
	
		//SEND COMMAND TO STOP THE MONOCHROMER
	dog=0;								//Control the number of tries 20 to fail
	SendMONO("MONO-STOP\r");
	memset(T, '\0', sizeof(T));
	ReadMONO(T);

return 0; //All right "OK"
}

char Scann(char *StartLambda, char *StopLambda, char *resollution, char *g){ //Function scann from to lambda
												//LEDSC03005003007000001010
	static char lambda[MAX_CHAR];
	static char T[MAX_CHAR];
	//static char parts[5];
	char *ret;
	char state;
	static int nano=0, dec=0, read=0;
	static int gain=0, i;
	//Values are in 0.01nm
	//Conversion /100 to nm
    
    
	//Selecting the correct Amplification for the PGA
	//VALID COMMANDS TO AMPLIFICATION:
	//GAIN1 GAIN0			AMPLIFICATION
	//0		0 		-->		*47 	
	//0		1		-->		*94
	//1		0		-->		*194
	//1		1		-->		*414
	gain=aToInt(g);
	switch (gain)
	{
	case(0):
		GAIN1_0; 
		GAIN0_0;
		break;
	case(1):
		GAIN1_0; 
		GAIN0_1;
		break;
	case(10):
		GAIN1_1; 
		GAIN0_0; 
		break;
	case(11):
		GAIN1_1; 
		GAIN0_1; 
		break;
	default: //Condition for using with the preamplifier HMS564
             //Make sure the switch is in the correct position
		GAIN1_0; 
		GAIN0_0; 
		break;
	}
	
	_delay_ms(50); //Time to wait the multiplexor
	//_delay_ms(1000);

	//SendPC("SCANNNING\n");
			//PROCESS:
			//1: GoTo first position
			//2: Take measure
			//3: Send to PC lambda and measure
			//4: Increment to nex lambda
			//5: If reached last lambda exit
			//6: Send AOK to PC to indicate end operation
			
	for(i=aToInt(StartLambda) ; i<=aToInt(StopLambda) ; i = i + aToInt(resollution)){	
			/*1:Going to the posotion, formating lambda*/
		nano=i/100;//Nanometers
		dec=i%100;//Rest
		
        _sprintf(lambda,"%d.%d",nano,dec);
		//intToStr(nano,lambda,4);
		//strcat(lambda,".");
		//intToStr(dec,lambda+5,2);
		state=GoToLED(lambda);
		if(state!=0)return 2;

		//Now read the photodiode value:
		_delay_ms(50);
		read = ADC_Convert(PD_MEASURE_CH,64); //Converts in channel 0 --> To decode *VREF/FULLSCALE		
     
        _sprintf(lambda,"%04d.%02d\t%04d",nano,dec,read);
		//intToStr(read,lambda+7,4);
            SendPC(lambda);

            ReadPC(T);
			ret = strstr(T, "ACK");
			if(ret==NULL)return 3;							
        }//End scanning for reached
return 0; //All right "OK"
}	
	
	
char TestConnection(){
	char read[MAX_CHAR]="";
	//const char haystack[20];
	//const char needle[10] = "ok";
	char *ret;
	memset(read, '\0', sizeof(read));

	SendMONO("2\r");
	ReadMONO(read);
	ret = strstr(read, "ok");
	if(ret==NULL)return 0;

	memset(read, '\0', sizeof(read));
	
	SendMONO("s\r");
	ReadMONO(read);
	ret = strstr(read, "ok");
	if(ret==NULL)return -1;
	
	SendMONO("MODEL\r");
	ReadMONO(read);
	ret = strstr(read, "ok");
	if(ret==NULL)return -1;
		
	SendMONO("?EEOPTIONS\r");
	ReadMONO(read);
	ret = strstr(read, "ok");
	if(ret==NULL)return -1;
		
	SendMONO("mono-eestatus\r");
	ReadMONO(read);
	ret = strstr(read, "ok");
	if(ret==NULL)return -1;
		
	SendMONO("?gratings\r");
	ReadMONO(read);
	ret = strstr(read, "ok");
	if(ret==NULL)return -1;
	
	return 0;//Established connection
}

char resetMONO(){
	char read[MAX_CHAR]="";
	char *ret;
	char dog=0;

	memset(read, '\0', sizeof(read));

	SendMONO("hi\r");
	do
		{
		ReadMONO(read);
		ret = strstr(read, "ok");
		dog++;
	}while((ret==NULL)&&(dog<=100));
	if(dog==100) return 1;
	else return 0;
}

char ledON(){
	LEDLAMP1;	//Wilson current source ON
	return 0;
}
char ledOFF(){
	LEDLAMP0;	//Wilson current source OFF
	return 0;
}
char setTemp(char setPoint){
	//VSET = RTh * 100 μA
	// Because of the complex ecuations of steinhart the
	// value is sent in mV -> between 0.358V to 3.196V
	// Temperature control is in VOUTA --> Channel 0
	// Convert the temperature in mv to the correct level
	int Dn=0;
	// Vref = 5V
	// G = 1
	// n = 12
	// Vout = Vref*G*(Dn/2^n)
	// Dn = 2^n*Vout/(Vref*G)
	Dn = (int) setPoint*4096/5000; //5000 Because setpoint in mv
	DAC_Convert(TempChannel,Dn);
	return 0;
}

char setPower(char setPoint){ //Settling constant power value
	MODE_CONSTANT_POWER;
	
	return 0;

}

char setCurrent(char setPoint){ //Settling constant current value
	MODE_CONSTANT_CURRENT;

	return 0;
}

char LDSPowerScann(){
	//Skeleton for POWER laser scanning
	//HTC_LDD_ON;
	//LEDLAMP0;
return 0;
}
	
/////////////////////// The actual MAIN //////////////////
int main (void) {   

	static char T[MAX_CHAR];
	char state = 1;			//0:Normal, 1:Error
	static char function[5]="";
	//Por ahora dejo lo siguiente pero no voy a necesitar auxiliar para los parametros
	static char parameters[GTPARAM]="";
	static char StartLambda[(SCPARAM+1)], StopLambda[(SCPARAM+1)], resollution[(SCPARAM+1)],g[3],temp[4],power[5];

	char Conn = 0;  		//0:No conexion , 1: Connected

	///////////////////////////// Configuration for communication files.
	
	/////////////////////////////
	/////////////////////
	/*PIN CONFIGURATION*/
	/////////////////////
	
	//PINSEL0
	//			33222222222211111111110000000000
	//			10987654321098765432109876543210 
	//P0.0		..............................01	TXD0 (UART0)
	//P0.1		............................01..	RXD0 (UART0)
	//P0.2		..........................00....	GPIO/NO(I2C)
	//P0.3		........................00......	GPIO/NO(I2C)
	//P0.4		......................01........	SCK0 (SPI)
	//P0.5		....................00..........	GPIO
	//P0.6		..................01............	MOSI0 (SPI)
	//P0.7		................01..............	SSEL0 (SPI)
	//P0.8		..............01................	TXD1 (UART1)
	//P0.9		............01..................	RXD1 (UART1)
	//P0.10		..........11....................	AD0.3 PDANODE_CH
	//P0.11		........11......................	AD0.4 LDCATHODE_CH
	//P0.12		......11........................	AD0.5 LDANODE_CH
	//P0.13		....00..........................	GPIO 
	//P0.14		..00............................	GPIO
	//P0.15		00..............................	GPIO GAIN0 (PGA)
	//PINSEL0=0b00000011111101010101000100000101;

	PINSEL0 = 0x03F55105;				
	
	//PINSEL1
	//			33222222222211111111110000000000
	//			10987654321098765432109876543210 
	//P0.16		..............................00	GPIO GAIN1 (PGA)
	//P0.17		............................00..	GPIO
	//P0.18		..........................00....	GPIO
	//P0.19		........................00......	GPIO  _LDAC
	//P0.20		......................00........	GPIO  _SHDN
	//P0.21		....................00..........	GPIO  LEDLAMP_ONOFF
	//P0.22		..................11............	AD0.0 PD_MEASURE_CH
	//P0.23		................11..............	AD0.1 CURRENT_MONITOR
	//P0.24		..............11................	AD0.2 POWER_MONITOR_CH
	//P0.25		............11..................	AD0.6 ACTUAL_TEMP_MONITOR_CH
	//P0.26		..........00....................	AD0.7 GPIO
	//P0.27		........00......................	GPIO  HTC_LDD_ONOFF
	//P0.28		......00........................	GPIO  MODE
	//P0.29		....00..........................	GPIO
	//P0.30		..00............................	GPIO
	//P0.31		00..............................	GPIO
	//PINSEL1=0b00000000000011111111000000000000; 
	
	PINSEL1 = 0x000FF000; 
	
	
	//////////////////////
	/*IN - OUT selection*/
	//////////////////////
	
	//IN: 1,9,10,11,12,14,22,23,24,25,26
	//OUT:0,2,3,4,5,6,7,8,13,15,16,17,18,19,20,21,27,28,29,30,31
			//    33222222222211111111110000000000
			//    10987654321098765432109876543210  
	//IODIR=   (0b11111000001111111010000111111101);
	
	IODIR = 0xF83FA1FD;
	
	
	
	/////////////////////////////
	/*PERIFERALS INITIALIZATION*/
	/////////////////////////////
	UART0_Init(); 
	UART1_Init(); 
	SPI_Init();
	_LDAC1;	//Disables latch converter
	//_SHDN1; //Disables hardware power converter
	//TIMER0_Init();	

 	///////////
	/*PROGRAM*/
	///////////
//	Conn=TestConnection();//Initial connection to monocrhomer
	while(1) { //normal procedure
	
		//while(1){ReadPC(T);SendPC(T);SendPC("OK");}
		memset(T, '\0', sizeof(T));
        ReadPC(T);			//Wait for a PC valid command
        sscanf(T,"%s\t%s",function,parameters);
		//memset(function, '\0', sizeof(function));
		//memset(parameters, '\0', sizeof(parameters));
		//strncpy(function,T,5);
		//strcpy(T,T+5);



			if (strcmp(function, "LEDGT") == 0){ //GOTO statement
				//strncpy(parameters,T,GTPARAM);
				SendPC("GOTO statement");
				state = GoToLED(parameters);
				if(state==0)SendPC("AOK");
				else SendPC("NOK");
			
			} 			
			else if(strcmp(function, "LEDTR") == 0){ //Terminal mode for LED's operation
				SendPC("Terminal mode");
				_delay_ms(100);
				do{
					memset(T, '\0', sizeof(T));
					ReadPC(T);
					if(strcmp(T, "LEDTE") != 0){
						SendPC(T);	//Echo debug
						SendMONO(T);SendMONO("\r");
						memset(T, '\0', sizeof(T));
						ReadMONO(T);
						SendPC(T);
						}
           		}
				while(strcmp(T, "LEDTE") != 0);	//LED mode Terminal END
			}
			
			else if (strcmp(function, "LEDSC") == 0){ //Led scan statement	
				strncpy(StartLambda,T,SCPARAM);
                strcat(StartLambda,"\0");//Closed
				strcpy(T,T+SCPARAM);
				strncpy(StopLambda,T,SCPARAM);
                strcat(StopLambda,"\0");//Closed
				strcpy(T,T+SCPARAM);
        		strncpy(resollution,T,SCPARAM);
                strcat(resollution,"\0");//Closed
				strcpy(T,T+SCPARAM);
        		strncpy(g,T,2);
                strcat(g,"\0");//Closed

                /*//Descomentar cuando se modifique el programa del ordenador
				sscanf(T,"%s\t%s\t%s\t%s",StartLambda,StopLambda,resollution,g); */


				SendPC("Scanning LED mode");
                LEDLAMP1;	//Wilson current source ON
                HTC_LDD_OFF; //Laser diode driver & HTC OFF
				state = Scann(StartLambda,StopLambda,resollution,g);
				if(state==0)SendPC("AOK");//Send to PC end of scan ok to make the correct file
				else SendPC("NOK");
				LEDLAMP0;	//Wilson current source ON
			}
			
			else if(strcmp(function, "CONNX") == 0){ //Monochromer connexion status
              	Conn=TestConnection();
               	_delay_ms(100);
               	if(!Conn)SendPC("Monochromer is not connected");
               	else SendPC("Monochromer is correctly connected");
			}
			else if(strcmp(function, "RESET") == 0){ //Monochromer reset request
              	state=resetMONO();
				if(state==0)SendPC("AOK");
				else SendPC("NOK");			
			}

			else if(strcmp(function, "LEDON") == 0){ //LED ON command
				if(state==0)SendPC("AOK");
				else SendPC("NOK");	
			}
			else if(strcmp(function, "LEDOF") == 0){ //LED OFF command
				if(state==0)SendPC("AOK");
				else SendPC("NOK");	
			}
			else if(strcmp(function, "LDSSC") == 0){ //Laser Diode Spectral Scan
				sscanf(T,"%s\t%s\t%s\t%s",StartLambda,StopLambda,resollution,g,temp,power);

				state = setTemp(temp); //Temp in mV
				state = setPower(power);

				state = Scann(StartLambda,StopLambda,resollution,g);
				if(state==0)SendPC("AOK");
				else SendPC("NOK");
			}
			else if(strcmp(function, "LDTSC") == 0){ //Laser diode Temperature Scan
				state = LDSTemperatureScann();
				if(state==0)SendPC("AOK");
				else SendPC("NOK");
			}
			else if(strcmp(function, "LDPSC") == 0){ //Laser diode Power scan
				state = LDSPowerScann();
				if(state==0)SendPC("AOK");
				else SendPC("NOK");
			}
			

			
		}//end while(1)


	if(state==-1){
		Conn=TestConnection();
	}
		
	
return 0;
}  

// --------------------------------------------------------------
