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

				// Serial settings //
                if (strcmp(name,"UsartPort")==0) strcpy(USART_PORT,data);
                else if (strcmp(name,"UsartBaud")==0) USART_BAUD=atoi(data);
                else if (strcmp(name,"EndOfString")==0) EndOfString=data[0];
                else if (strcmp(name,"LogNotEndedString")==0) LogNotEndedString=atoi(data);
				// Tcp settings //
                else if (strcmp(name,"TcpPort")==0) strcpy(TCP_PORT,data);
				// Statistic settings //
                else if (strcmp(name,"LogDays")==0) LogDays=atoi(data);
                else if (strcmp(name,"LogFolder")==0) strcpy(LogFolder,data);
				// Auto settings //
                else if (strcmp(name,"UserSettsFile")==0) strcpy(UserSettsFile,data);
                //else if (strcmp(name,"MainUser")==0) strcpy(MainUser,data);
				// Vendors //
				else if (strcmp(name,"vendor-uniel")==0) vendorUniel=atoi(data);
            }
        }
        fclose (file);
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
