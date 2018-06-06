#ifndef GLOBALS_H
#define GLOBALS_H

// ALL THE GLOBAL DECLARATIONS
#include <stdio.h>
#include <QString>

extern bool connected;
extern FILE *fp;
extern int fd, n;
extern struct tm *tlocal;
extern char output[128];
extern char fileName[256];
extern char responsivityCurveFileName[256];
extern char responsivityDetector[256];
extern char thermistorName[256];
extern bool changedName;
extern char lastScans[10][256];//Last 5 scans saved
extern int numberScans;
extern char ScanBoxName[256];
extern int errors;



extern float lambdaMAX; //Max value lambda for analysis
extern float PoptMAX; //Max pot for max lambda

int ugets(int fd,  char *buf);
void uputch(int fd,unsigned char a);
int uuenc(int fd,unsigned char *buf,int n);
int ConnectNow();
int RecalculateZeroValue(int index);
void PrintRGBspectre();
void SeeRespCurve();
void SeeLumiCurve();
float zeroChecker(float lambda, int index);
float responsivityChecker(float lambda);






#endif // GLOBALS_H
