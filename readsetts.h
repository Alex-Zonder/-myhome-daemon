//_____________________________S E T T S__________________________________//
char name[128], data[128];
int LogDaysPam;
void ReadMainSettsFile () {
    char filename[128];
    strcpy (filename,FILE_CONF);
    
    sprintf(strToPrint,"----- OPEN FILE:%s -----",filename);
    MyPrint();
    
    FILE *file = fopen ( filename, "r" );
    if ( file != NULL ) {
        char line [ 128 ];
        while ( fgets ( line, sizeof line, file ) != NULL ) {
            if (strlen(line)>1){
                LogDaysPam=LogDays;LogDays=0;
                sprintf(strToPrint,"%s",line);
                MyPrint();
                LogDays=LogDaysPam;

                
                int y=0;
                while (y<128){if(name[y])name[y]='\0';if(data[y])data[y]='\0';y++;}
                
                int x=0;
                int x1=0;
                int name_data=0;
                int lenLine = strlen(line)-1;
                while (x<lenLine){
                    if (line[x]==' ' && name_data==0) {x1=0;x++;name_data=1;}
                    if (name_data==0)name[x1]=line[x];
                    else data[x1]=line[x];
                    x++;
                    x1++;
                }
                
                
                if (strcmp(name,"USART_RW_GPIO_ENABLE")==0){
                    USART_RW_GPIO_ENABLE=atoi(data);
                    
                    strcpy (USART_RW_GPIO_EXPORT,"echo ");
                    strcat (USART_RW_GPIO_EXPORT,data);
                    strcat (USART_RW_GPIO_EXPORT," > /sys/class/gpio/export");
                    
                    strcpy (USART_RW_GPIO_UNEXPORT,"echo ");
                    strcat (USART_RW_GPIO_UNEXPORT,data);
                    strcat (USART_RW_GPIO_UNEXPORT," > /sys/class/gpio/unexport");
                    
                    strcpy (USART_RW_GPIO_DIRECTION,"echo out > /sys/class/gpio/gpio");
                    strcat (USART_RW_GPIO_DIRECTION,data);
                    strcat (USART_RW_GPIO_DIRECTION,"/direction");
                    
                    strcpy (USART_RW_GPIO_VALUE,"/sys/class/gpio/gpio");
                    strcat (USART_RW_GPIO_VALUE,data);
                    strcat (USART_RW_GPIO_VALUE,"/value");
                }
                
                else if (strcmp(name,"UsartPort")==0) strcpy(USART_PORT,data);
                else if (strcmp(name,"UsartBaud")==0) USART_BAUD=atoi(data);
                else if (strcmp(name,"EndOfString")==0) EndOfString=data[0];
                else if (strcmp(name,"LogNotEndedString")==0) LogNotEndedString=atoi(data);
                else if (strcmp(name,"USART_SLEEP")==0) USART_SLEEP=atoi(data);
                else if (strcmp(name,"USART_SLEEP_BYTE")==0) USART_SLEEP_BYTE=atoi(data);
                
                else if (strcmp(name,"TcpPort")==0) strcpy(TCP_PORT,data);
                else if (strcmp(name,"WwwFolder")==0) strcpy(WWW_FOLDER,data);
                else if (strcmp(name,"CgiFolder")==0) strcpy(CGI_FOLDER,data);
                
                else if (strcmp(name,"LogDays")==0) LogDays=atoi(data);
                else if (strcmp(name,"LogFolder")==0) strcpy(LogFolder,data);
                
                else if (strcmp(name,"IOFolder")==0) strcpy(FilesFolder,data);
                else if (strcmp(name,"InFile")==0) strcpy(IN_FILE,data);
                else if (strcmp(name,"OutFile")==0) strcpy(OUT_FILE,data);
                
                else if (strcmp(name,"UserSettsFile")==0) strcpy(UserSettsFile,data);
                else if (strcmp(name,"MainUser")==0) strcpy(MainUser,data);

            }
        }
        fclose (file);
        
        //sprintf(strToPrint,"---------- %s ------------",FilesFolder);
        //MyPrint();
    }
    else{perror (filename); /* why didn't the file open? */}
}

//_____________________________A U T O__________________________________//
char fileComm[50][50];
char fileSend[50][50];
char fileAction[50][2];
char CommCount=0;

void ReadAutoFile (){
    sprintf(strToPrint,"----- OPEN FILE:%s -----",UserSettsFile);
    LogDaysPam=LogDays;LogDays=0;
    MyPrint();
    LogDays=LogDaysPam;
    
    //______Read File______
    FILE *file = fopen ( UserSettsFile, "r" );
    if ( file != NULL ){
        char line [ 128 ];
        while ( fgets ( line, sizeof line, file ) != NULL ) {
            char x;
            char y;
            char z=0;
            //______Read String______
            for (x=0; x<=strlen(line); x++){
                if (line[x]==' ' || line[x]=='\n') {z++;x++;y=0;}
                
                if (z==0){
                    fileComm[CommCount][x]=line[x];
                }
                else if (z==1){
                    fileSend[CommCount][y]=line[x];
                    y++;
                }
                else if (z==2){
                    fileAction[CommCount][y]=line[x];
                    y++;
                }
            }
            
            sprintf(strToPrint,"Com(%i):%s Otv:%s Act:%s\n",CommCount,fileComm[CommCount],fileSend[CommCount],fileAction[CommCount]);
            LogDaysPam=LogDays;LogDays=0;
            MyPrint();
            LogDays=LogDaysPam;
            
            CommCount++;
        }
    }
    else{perror (UserSettsFile); /* why didn't the file open? */}
}



