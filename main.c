#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <sys/types.h>
//#include <pwd.h>

#define PROGRAMM_NAME "Myhome"
#define PROGRAMM_VERSION "2.6.beta1"


//____________________   V E N D O R S   ___________________________
#include "hex-converter.h"
#include "vendor/uniel.h"



//_____________________   V A R I A B L E S   ___________________________
/* S E T T I N G S */
char FILE_CONF[128]="/etc/myhome.conf";

char UserSettsFile[128]="";
char MainUser[50]="";

int LogDays=0;
char LogFolder[128]="";


/* V A L U E S */
int done;
char viewToScreen;


//_____________________D E C L A R A T I O N S___________________________
void UsartSend (char command[255]);
void Tcp_Send (char *cmd);
void WriteToInFile (char *cmd);
void MakeUserAuto (char *comm, int send);


//_____________________ M Y   I N C L U D E S ___________________________
#include "myprint.h"
#include "usart.h"
#include "tcpserver.h"
#include "readsetts.h"
#include "auto.h"


//_____________________   F U N C T I O N S   ___________________________

//______________Exiting by KILLING & CTRL+C_________________//
/* Signal Handler for KILL */
void term(int signum) {
    sprintf(strToPrint,"---------- EXIT %s!!! ----------",PROGRAMM_NAME);
    MyPrint();
    done = 1;
}
/* Signal Handler for SIGINT */
void sigintHandler(int sig_num) {
    /* Reset handler to catch SIGINT next time. */
    sprintf(strToPrint,"---------- EXIT %s!!! ----------",PROGRAMM_NAME);
    MyPrint();
    signal(SIGINT, sigintHandler);
    done = 1;
}
/* Initing SIGTERM & SIGINT */
void InitSelfKilling (void) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);

    /* Set the SIGINT (Ctrl-C) signal handler to sigintHandler */
    signal(SIGINT, sigintHandler);
}


//_________________________ M A I N ____________________________//
int main (int argc, char *argv[]) {
    InitSelfKilling();

    int i;
    for (i=1; i<argc; i++){
        if ((argv[i][0]=='-') || (argv[i][0]=='/')){
            if (argv[i][1]=='s'){
                if (argv[i][2]=='l'){
                    int sleep=0;
                    if (argv[i+1]) sleep = atoi(argv[i+1]);
                    //printf ("Sleep:%i\n",sleep);
                    //fflush(stdout);
                    while (sleep) {
                        usleep(920);
                        sleep--;
                    }
                    return 0;
                }
                else viewToScreen='1';
            }
            else if (argv[i][1]=='v'){
                printf ("%s:%s\n",PROGRAMM_NAME,PROGRAMM_VERSION);
                fflush(stdout);
                return 0;
            }
            else if (argv[i][1]=='f'){
                strcpy (FILE_CONF,argv[i+1]);
            }
        }
    }


    ReadMainSettsFile();
    ReadAutoFile();

    sprintf(strToPrint,"-------- S T A R T I N G   %s:%s --------",PROGRAMM_NAME,PROGRAMM_VERSION);
    MyPrint();

    done=0;
    InitUsart();
    StartTcpServer();

    if (!done) {
        sprintf(strToPrint,"------------- WORKING ---------------");
        MyPrint();
    }
    else {
        sprintf(strToPrint,"------------- ERROR ---------------");
        MyPrint();
    }

    while (!done) {
        usleep(100000);
    }

    //___ E X I T I N G ___//
    CloseUsart();
    Tcp_close();
    return 0;
}
