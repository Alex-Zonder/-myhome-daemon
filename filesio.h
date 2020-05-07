//_____________________   V A R I A B L E S   ___________________________
char fileIn[128],fileOut[128];

//_____________________D E C L A R A T I O N S___________________________
void WriteToInFile (char *cmd);

//_____________________   F U N C T I O N S   ___________________________
void *ReadOutFile (void *arg){
    while (!done) {
        char command[255]="";
        char command1[255]="";
        
        FILE * pFile;
        pFile = fopen (fileOut , "r");
        if ( fgets (command , 255 , pFile) != NULL ) {
            if (strlen(command)>1)
            {
                //Erasing \n from command string
                int i;
                for (i=0; command[i] !='\n'; i++){
                    command1[i] = command[i];
                }
                
                sprintf(strToPrint,"OUT FILE:%s",command1);
                MyPrint ();
                
                //Erasing OUT.TXT
                freopen (fileOut, "w" , pFile);
                fprintf(pFile,"");

                //Sending...
                MakeUserAuto(command1,1);
            }
        }
        fclose (pFile);

        usleep(20000);
    }
    return 0;
}

void InitFilesIO (void) {
    sprintf(fileIn,"%s/%s",FilesFolder,IN_FILE);
    sprintf(fileOut,"%s/%s",FilesFolder,OUT_FILE);

    char sysCom[256];
    sprintf(sysCom,"touch %s",fileIn);
    system(sysCom);
    sprintf(sysCom,"echo -n "" > %s",fileIn);
    system(sysCom);
    
    sprintf(sysCom,"touch %s",fileOut);
    system(sysCom);
    if (strcmp(MainUser,"")) {
        sprintf(sysCom,"chown %s %s",MainUser,fileOut);
        system(sysCom);
    }
    sprintf(sysCom,"chmod 777 %s",fileOut);
    system(sysCom);

    // Creating pthead //
    pthread_t PTHFiles;
    if (pthread_create(&PTHFiles, NULL, ReadOutFile, NULL) != 0) {
        sprintf(strToPrint,"Error opening Usart PTHead");
        MyPrint ();
        done=1;
    }
}

void WriteToInFile (char *cmd){
    FILE *file = fopen (fileIn,"w+");
    if ( file != NULL ) {
        fprintf(file,"%s\n",cmd);
    }
    fclose (file);
}

void CloseFilesIO (void) {
    char sysCom[256];
    if (strlen(fileIn)>1){
        sprintf(sysCom,"rm %s",fileIn);
        system(sysCom);
    }
    
    if (strlen(fileOut)>1){
        sprintf(sysCom,"rm %s",fileOut);
        system(sysCom);
    }
}
