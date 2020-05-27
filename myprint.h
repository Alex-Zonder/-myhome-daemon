#include <dirent.h>
#include <sys/time.h>
//#include <time.h>


char myTime[24];
char year[5],month[3],day[3];
void GetTime (void){
    /* Get the current time. */
    const time_t timer = time(NULL);
    struct tm* aTm = localtime(&timer);
    sprintf(year,"%04d",aTm->tm_year+1900);
    sprintf(month,"%02d",aTm->tm_mon+1);
    sprintf(day,"%02d",aTm->tm_mday);
    /* Get the current ms. */
    struct timeval now;
    struct tm *parsed;
    gettimeofday(&now, NULL);
    parsed = localtime((const time_t*)&now.tv_sec);
    /* Convert to string. */
    sprintf(myTime,"%02d:%02d:%02d:%03d",
            aTm->tm_hour,aTm->tm_min,aTm->tm_sec,
            (int)now.tv_usec/1000);

}



char strToPrint[4096];
void MyPrint (void) {
    if (strToPrint[strlen(strToPrint)-1]=='\n') {
        //strToPrint[strlen(strToPrint)]='\0';
        strToPrint[strlen(strToPrint)-1]='\0';
    }
    GetTime();


    if (viewToScreen){
        printf ("%s - %s\n",myTime,strToPrint);
        fflush(stdout);
    }

    if (LogDays){
        /* Generate neme of file */
        DIR* dir = opendir(LogFolder);
        if (!dir){
            printf ("%s - Make Logs directory\n",myTime);
            fflush(stdout);
            char mkLog[140];
            sprintf(mkLog,"mkdir %s",LogFolder);
            system(mkLog);
        }
        else {
            closedir(dir);
        }

        /* Generate neme of file */
        char fileLog[50];
        sprintf(fileLog,"%s-%s-%s.txt",year,month,day);
        /* Writing file */
        char filename[256];
        sprintf(filename, "%s/%s",LogFolder,fileLog);
        FILE *file = fopen ( filename, "a+" );
        if ( file != NULL ) {
            fprintf(file, "%s - %s\n",myTime,strToPrint);
        }
        fclose (file);

        /* Delete old files*/
        int filesCount=0;
        struct dirent **namelist;
        int n;
        n = scandir(LogFolder, &namelist, NULL, alphasort);
        if (n < 0)
            perror("scandir");
        else {
            while (n--) {
                if (namelist[n]->d_name[0]!='.') {
                    filesCount++;
                    //printf("%s\n", namelist[n]->d_name);
                    if (filesCount>LogDays) {
                        //printf("remove %s\n", namelist[n]->d_name);
                        char sysCom[512];
                        sprintf(sysCom,"exec rm -r %s/%s",LogFolder,namelist[n]->d_name);
                        system(sysCom);
                    }
                }
                free(namelist[n]);
            }
            free(namelist);
        }
    }
}
