#include <unistd.h>             //Sleep
#include <fcntl.h>              //UART
#include <termios.h>            //UART
#include <signal.h>             //SIGINT
//#include <pthread.h>

//_____________________   V A R I A B L E S   ___________________________
char USART_PORT[128];
int USART_BAUD;
char EndOfString;
int LogNotEndedString;


int uart0 = -1;


/* Vendors */
char vendorUniel = 0;


//_____________________D E C L A R A T I O N S___________________________
void InitUsart (void);
void *ReadUsart(void *arg);
void UsartSend (char command[255]);
void CloseUsart(void);




// ------------------------------- Init Usart ------------------------------ //
pthread_t PTHUsart = 0;
void InitUsart (void) {
    sprintf(strToPrint,"INIT USART AT:%s SPEED:%i",USART_PORT,USART_BAUD);
    MyPrint();

    uart0 = open(USART_PORT, O_RDWR | O_NOCTTY | O_NDELAY);         //Open in non blocking read/write mode
    if (uart0 == -1){
        sprintf(strToPrint,"Error - Unable to open UART.  Ensure it is not in use by another application");
        MyPrint ();
//        done=1;
    }
    else {
        if(USART_BAUD==110)USART_BAUD=B110;//3
        else if(USART_BAUD==150)USART_BAUD=B150;//5
        else if(USART_BAUD==300)USART_BAUD=B300;//7
        else if(USART_BAUD==600)USART_BAUD=B600;//8
        else if(USART_BAUD==1200)USART_BAUD=B1200;//9
        else if(USART_BAUD==2400)USART_BAUD=B2400;//11
        else if(USART_BAUD==4800)USART_BAUD=B4800;//12
        else if(USART_BAUD==9600)USART_BAUD=B9600;//13
        else if(USART_BAUD==19200)USART_BAUD=B19200;//14
        else if(USART_BAUD==38400)USART_BAUD=B38400;//15
        else if(USART_BAUD==57600)USART_BAUD=B57600;//4097
        else if(USART_BAUD==115200)USART_BAUD=B115200;//4098

        struct termios options;
        tcgetattr(uart0, &options);
        options.c_cflag = USART_BAUD | CS8 | CLOCAL | CREAD;            //Set baud rate
        options.c_iflag = IGNPAR | ICRNL;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcflush(uart0, TCIFLUSH);
        tcsetattr(uart0, TCSANOW, &options);

        // Read old data //
        usleep(10000);
        if (uart0 != -1) {
            char rx_buffer[1024] = "";
            int rx_length = read(uart0, (void*)rx_buffer, 1024);
        }
    }

    // Creating pthead //
    //    pthread_t PTHUsart = 0;
    if (PTHUsart == 0) {
        if (pthread_create(&PTHUsart, NULL, ReadUsart, NULL) != 0) {
            sprintf(strToPrint,"Error opening Usart PTHead");
            MyPrint ();
            done=1;
        }
    }

}

void ReastartUsart (){
  if (uart0 != -1) close(uart0);
  InitUsart();
}



// ------------------------------- Read Usart ------------------------------ //
char usart_rx_start[1024];
void *ReadUsart(void *arg) {	// Read Usart
    while (!done) {
        // Usart opened //
        if (uart0 != -1) {
            // Read usart //
            char usart_rx[1024] = "";
            int usart_rx_length = read(uart0, (void*)usart_rx, 1024);//read

            /* An error occured (will occur if there are no bytes) */
            if (usart_rx_length == 0) {
                sprintf(strToPrint, "USART - Dev-error!");
                MyPrint();
                ReastartUsart ();
            }
            /* No Data */
            else if (usart_rx_length < 0) {  }

            /* Bytes received */
            else {
                // Cyber-Light //
                if (!vendorUniel) {
                    if ((usart_rx_length + strlen(usart_rx_start)) < 1024) {
                        char usart_rx_tmp[1024] = "";
                        sprintf(usart_rx_tmp, "%s%s", usart_rx_start, usart_rx);
                        sprintf(usart_rx, "%s", usart_rx_tmp);
                    }
                    sprintf(usart_rx_start, "%s", "");

                    /*   ALL COMAND   */
                    if (usart_rx[strlen(usart_rx) - 1] == '\n' || usart_rx[strlen(usart_rx) - 1] == EndOfString){
                        sprintf(strToPrint, "USART - %s", usart_rx);
                        MyPrint();

                        Tcp_Send(usart_rx);
                        MakeUserAuto (usart_rx, 0);
                    }
                    /*   NOT ALL COMAND   */
                    else {
                        if (LogNotEndedString == 1){
                            sprintf(strToPrint, "USART BAD - %s", usart_rx);
                            MyPrint();
                        }
                        if ((strlen(usart_rx) + strlen(usart_rx_start)) < 1024)
                            sprintf(usart_rx_start, "%s%s", usart_rx_start, usart_rx);
                        else
                            sprintf(usart_rx_start, "%s", usart_rx);
                    }
                }


                // Uniel //
                if (vendorUniel) {
                    // With out controller //
                    if (usart_rx_length > 10) {
                        const char rxShort[10];
                        memcpy((unsigned char*)rxShort, usart_rx + 8, 8);
                        //unielCheck(rxShort, 8);
                        sprintf(usart_rx, "%s", unielCheck((unsigned char*)rxShort, 8));
                    }
                    // Only client data //
                    else {
                        sprintf(usart_rx, "%s", unielCheck(usart_rx, 8));
                    }

                    sprintf(strToPrint, "USART - %s", usart_rx);
                    MyPrint();

                    Tcp_Send(usart_rx);
                    MakeUserAuto (usart_rx, 0);
                }
            }
            usleep(10000);
        }
        // Error opned usart //
        else {
//            sprintf(strToPrint,"USART - Dev-error!");
//            MyPrint();
            ReastartUsart ();
            if (uart0 == -1) sleep(5);
        }
    }
    return 0;
}



// ----------------------------- Send to usart ----------------------------- //
void UsartSend (char command[255]) {		// SEND TO USART
    if (uart0 != -1) {
        int commandLen;

        //   Cyber-Light   //
        if (!vendorUniel) {
            commandLen = strlen(command);
            if (command[commandLen-1]=='\n') commandLen=commandLen-2;
            else sprintf(strToPrint,"Send: %s", command);
        }
        //   Uniel   //
        else if (vendorUniel) {
            commandLen = 8;
            memcpy(command, cyber2uniel(command), 8);
            sprintf(strToPrint,"Send: %s", hex2string(command, 8));
        }
        MyPrint ();

        //   S E N D   //
        int count = write(uart0, command, commandLen);
        /* Error sending */
        if (count < 0) {
            sprintf(strToPrint,"UART TX error");
            MyPrint ();
            ReastartUsart();
            if (uart0 != -1) UsartSend (command);
        }
    }
    else {
        ReastartUsart();
        if (uart0 != -1) UsartSend (command);
    }
}



// ------------------------------ Close Usart ------------------------------ //
void CloseUsart(void) {
    sprintf(strToPrint,"USART: Stop.");
    MyPrint();
    close(uart0);
}
