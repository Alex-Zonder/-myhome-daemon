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

int USART_SLEEP;
int USART_SLEEP_BYTE;
int USART_RW_GPIO_ENABLE;
char USART_RW_GPIO_EXPORT[50];
char USART_RW_GPIO_UNEXPORT[50];
char USART_RW_GPIO_DIRECTION[50];
char USART_RW_GPIO_VALUE[50];

int uart0=-1;

//_____________________D E C L A R A T I O N S___________________________
void InitUsart (void);
void *ReadUsart(void *arg);
void UsartSend (char command[255]);
void CloseUsart(void);


//_____________________   F U N C T I O N S   ___________________________
pthread_t PTHUsart = 0;
void InitUsart (void) {
    sprintf(strToPrint,"INIT USART AT:%s SPEED:%i",USART_PORT,USART_BAUD);
    MyPrint();

    // G P I O   -   R S - 4 8 5 //
    if (USART_RW_GPIO_ENABLE){
        sprintf(strToPrint,"Init GPIO-%i as RS485_R/W",USART_RW_GPIO_ENABLE);
        MyPrint();
        
        //_____Init GPIO as RW for Max485_________
        system(USART_RW_GPIO_EXPORT);
        system(USART_RW_GPIO_DIRECTION);
    }
    
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

char UsartCommandBad[1024];
char UsartCommand[1024];
void *ReadUsart(void *arg){	// Read Usart
    while (!done) {
        if (uart0 != -1) {
            char rx_buffer[1024] = "";
            int rx_length = read(uart0, (void*)rx_buffer, 1024);     //read
            if (rx_length < 0) { /* No Data */ }
            /* An error occured (will occur if there are no bytes) */
            else if (rx_length == 0) {
                sprintf(strToPrint,"USART - Dev-error!");
                MyPrint();
                ReastartUsart ();
            }
            /* Bytes received */
            else {
                sprintf(UsartCommand,"%s",rx_buffer);
                
                /*   ALL COMAND   */
                if (UsartCommand[strlen(UsartCommand)-1]=='\n' || UsartCommand[strlen(UsartCommand)-1]==EndOfString){
                    if (strlen(rx_buffer)+strlen(UsartCommandBad)<1024){
                        sprintf(UsartCommand,"%s%s",UsartCommandBad,rx_buffer);
                    }
                    sprintf(UsartCommandBad,"%s","");
                    
                    sprintf(strToPrint,"USART - %s",UsartCommand);
                    MyPrint();
                    
//                    WriteToInFile(UsartCommand);
                    Tcp_Send(UsartCommand);
                    MakeUserAuto (UsartCommand, 0);
                }
                /*   NOT ALL COMAND   */
                else {
                    if (LogNotEndedString==1){
                        sprintf(strToPrint,"USART BAD - %s",UsartCommand);
                        MyPrint();
                    }
                    if (strlen(rx_buffer)+strlen(UsartCommandBad)<1024)
                        sprintf(UsartCommandBad,"%s%s",UsartCommandBad,rx_buffer);
                    else
                        sprintf(UsartCommandBad,"%s",rx_buffer);
                }
            }
            usleep(10000);
        }
        else {
//            sprintf(strToPrint,"USART - Dev-error!");
//            MyPrint();
            ReastartUsart ();
            if (uart0 == -1) sleep(5);
        }
    }
    return 0;
}

void UsartSend (char command[255]) {		// SEND TO USART
    if (uart0 != -1) {
        // G P I O   -   R S - 4 8 5 //
        if (USART_RW_GPIO_ENABLE){
            FILE *fp;
            fp=fopen(USART_RW_GPIO_VALUE,"w");              //Max485 transmit mode
            putc('1',fp);
            fclose(fp);
        }
        
        //   S E N D   //
        int commandLen;
        commandLen = strlen(command);
        if (command[commandLen-1]=='\n') commandLen=commandLen-2;
        int count = write(uart0, command, commandLen);  //Sending.....
        /* Error sending */
        if (count < 0) {
            sprintf(strToPrint,"UART TX error");
            MyPrint ();
            ReastartUsart();
            if (uart0 != -1) UsartSend (command);
        }
        
        // G P I O   -   R S - 4 8 5 //
        if (USART_RW_GPIO_ENABLE){
            usleep(USART_SLEEP_BYTE*(commandLen-1)+USART_SLEEP);    //Sleep.....
            FILE *fp;
            fp=fopen(USART_RW_GPIO_VALUE,"w");              //Max485 resave mode
            putc('0',fp);
            fclose(fp);
        }
    }
    else {
        ReastartUsart();
        if (uart0 != -1) UsartSend (command);
    }
}

void CloseUsart(void){
    sprintf(strToPrint,"USART: Stop.");
    MyPrint();
    close(uart0);
}
