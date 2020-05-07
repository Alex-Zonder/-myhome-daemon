//_____________________D E C L A R A T I O N S___________________________
//void MakeUserAuto (char *comm, int send);
void Make (char *comm, int send);
void MakeByOne (char *comm, char *data, int cont);


void MakeUserAuto (char *comm, int send){
    char lastByte = strlen(comm);
    while (comm[lastByte]=='\n') {comm[lastByte]='\0';lastByte--;}

    if (send != 1){
        int firstByte = 0;
        char * pch;
        pch = strchr(comm, ';');

        while (pch != NULL) {
            char data[1024] = "";
            int dataLen = (int)(pch-comm+1);

            memmove(data, comm+firstByte, dataLen-firstByte);
            data[dataLen-firstByte+1] = '\0';

            Make (data,send);

            //sprintf(strToPrint,"AUTO:%i - %s\n",dataLen,data);
            //MyPrint();

            firstByte = dataLen;
            pch = strchr(pch+1, ';');
        }
    }

    else {
        Make (comm,send);
    }
}



void Make (char *comm, int send){
    char cont=0;
    while (cont < CommCount){
        char * pch;
        pch = strchr(fileComm[cont], '|');
        if (pch == NULL) {
            if (strcmp(comm, fileComm[cont]) == 0){
                MakeByOne(comm, fileComm[cont], cont);
                send = 0;
            }
        }
        else {
            char firstByte=0;
            while (pch!=NULL) {
                char data[50] = "";
                char dataLen = (int)(pch-fileComm[cont]);

                memmove(data,fileComm[cont]+firstByte,dataLen-firstByte);
                data[dataLen-firstByte+1]='\0';

                MakeByOne(comm, data, cont);

                firstByte=dataLen+1;
                pch=strchr(pch+1,'|');
            }

            char data[50] = "";
            char dataLen = strlen(fileComm[cont]);

            memmove(data,fileComm[cont]+firstByte,dataLen-firstByte);
            data[dataLen-firstByte+1]='\0';

            MakeByOne(comm,data,cont);
        }
        cont++;
    }
    if (send==1) UsartSend(comm);
}


// - - - - - - - - - - - - M A K E   S O R T E D   C O M A N D S - - - - - - - - - - //
void MakeByOne (char *comm, char *data, int cont){
    if (strcmp (comm, data) == 0) {
        if (strcmp (fileAction[cont],"") == 0) {
            sprintf(strToPrint,"AUTO BY:%s TO:%s ACT:USART\n",fileComm[cont],fileSend[cont]);
            MyPrint();

            usleep(100000);
            char * pch;
            pch=strchr(fileSend[cont],'|');
            if (pch == NULL) {
                UsartSend(fileSend[cont]);
            }
            else {
                char firstByte=0;
                while (pch != NULL) {
                    char data[50] = "";
                    char dataLen = (int)(pch-fileSend[cont]);

                    memmove(data,fileSend[cont]+firstByte,dataLen-firstByte);
                    data[dataLen-firstByte+1]='\0';

                    //sprintf(strToPrint,"AUTO BY:%s:%s TO:%s ACT:SCRIPT\n",data,comm,fileSend[cont]);
                    //MyPrint();

                    //MakeByOne(comm,data,cont);
                    UsartSend(data);
                    usleep(100000);

                    firstByte=dataLen+1;
                    pch=strchr(pch+1,'|');
                }
                char data[50] = "";
                char dataLen = strlen(fileSend[cont]);

                memmove(data,fileSend[cont]+firstByte,dataLen-firstByte);
                data[dataLen-firstByte+1]='\0';
                UsartSend(data);
            }
        }
        else {
            sprintf(strToPrint,"AUTO BY:%s TO:%s ACT:SCRIPT\n",fileComm[cont],fileSend[cont]);
            MyPrint();

            char script[128];
            sprintf(script, "bash %s \'%s\' > /dev/null 2>&1 &",fileSend[cont],comm);
            system(script);
        }
    }
}
