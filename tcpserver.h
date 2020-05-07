#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
//#include <pthread.h>
//#include <sys/stat.h>

//_____________________   V A R I A B L E S   ___________________________
#define BACKLOG 10     // how many pending connections queue will hold
char TCP_PORT[10];  // the port users will be connecting to

int sockfd, new_fd[255], fd_count=0;  // listen on sock_fd, new connection on new_fd
struct addrinfo hints, *servinfo, *p;
struct sockaddr_storage their_addr; // connector's address information
socklen_t sin_size;
struct sigaction sa;
int yes=1;
char s[INET6_ADDRSTRLEN];
int rv;
// WEB SERVER
char WWW_FOLDER[128]="";
char CGI_FOLDER[128]="";
char UserOrign[255][50];

//_____________________D E C L A R A T I O N S___________________________
void *Tcp_listen_func (void *arg);
void Tcp_Send (char cmd[255]);
void *ListenClient (void *arg);
// WEB SERVER
void MakeGet (int thisFd);
void MakeOpt (int thisFd);
void MakePost (int thisFd);
void SendPost (int thisFd, char *cmd);

//_____________________   F U N C T I O N S   ___________________________
void sigchld_handler(int s) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int StartTcpServer(){
    memset(&hints, 0, sizeof hints);
    //hints.ai_family = AF_UNSPEC;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(NULL, TCP_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    pthread_t Tcp_listen;
    if (pthread_create(&Tcp_listen, NULL, Tcp_listen_func, NULL) != 0) {
        return EXIT_FAILURE;
    }

    sprintf(strToPrint,"TCP STARTED ON:%s Waiting for connections",TCP_PORT);
    MyPrint();
    
    return 0;
}

void *Tcp_listen_func (void *arg) {
 // Настройка атрибутов потока
// pthread_attr_t threadAttr;
// pthread_attr_init(&threadAttr);
// pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
 pthread_attr_t ListClientAttr;
 pthread_attr_init(&ListClientAttr);
 pthread_attr_setdetachstate(&ListClientAttr, PTHREAD_CREATE_DETACHED);
    while(!done) {
        long x=0;
//        while (new_fd[x] || new_fd[x]==8) {x++;}
        while (new_fd[x]) x++;
        
        sin_size = sizeof their_addr;
        new_fd[x] = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd[x] == -1) {
            if (!done){
                sprintf(strToPrint,"TCP-server: Port Opend, exit program...");
                MyPrint();
                done=1;
            }
        }
        else {
            inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
            sprintf(strToPrint,"TCP CONNECTION:%i - IP:%s",(int)x,s);
            MyPrint();
        
            pthread_t ListClient;
            // Creating pthead //
            if (pthread_create(&ListClient, &ListClientAttr, ListenClient, (void *) x) != 0) {
                sprintf(strToPrint,"Error opening TCP PTHead");
                MyPrint();

//                char script[128];
//                sprintf(script, "/etc/init.d/myhome.d start");
//                system(script);
            }
            if (x>fd_count) fd_count++;
        }
    }
    return 0;
}

char NewFDvalue[255];
char buf[2048];
int numbytes;
void *ListenClient (void *arg) {
    int thisFd=(int) arg;
    NewFDvalue[thisFd]=1;
    
    while (!done) {
        memset (buf,'\0',2048);
        numbytes = recv(new_fd[thisFd], buf, 2048, 0);

        //   A L L   S T R I N G   //
        //sprintf(strToPrint,"TCP:%i:%s",thisFd,buf);
        //MyPrint();

        //_______ E X I T __________
        if ((buf[0]=='b' && buf[1]=='y' && buf[2]=='e') || numbytes==0){
            break;
        }

        //_______ W E B   V A L U E__________
        else if (buf[0]=='G' && buf[1]=='E' && buf[2]=='T'){
            //MakeGet(thisFd);
        }
        else if (buf[0]=='O' && buf[1]=='P' && buf[2]=='T'){
            MakeOpt(thisFd);
        }
        else if ((buf[0]=='P' && buf[1]=='O' && buf[2]=='S') || NewFDvalue[thisFd]==2){
            MakePost(thisFd);
        }
        
        //_______ S E N D   T O   U S A R T __________
        else {
            sprintf(strToPrint,"TCP:%i:%s",thisFd,buf);
            MyPrint();
            
            if(buf[numbytes-1]!=';'){
                buf[numbytes-1] = '\0';numbytes--;
                if(buf[numbytes-1]!=';') {buf[numbytes-1] = '\0';numbytes--;}
            }
            
            if (strlen(buf)>1) MakeUserAuto(buf,1);
            //UsartSend(buf);
        }
        
    }
    close(new_fd[thisFd]);
    NewFDvalue[thisFd]=0;
    new_fd[thisFd]=0;
    sprintf(strToPrint,"TCP CLOSE:%i",thisFd);
    MyPrint();
pthread_exit(NULL);
    return 0;
}

void Tcp_Send (char *cmd) {
    int l=0;
    while (l<=fd_count){
        // TCP value
        if (NewFDvalue[l]==1){
            //sprintf(strToPrint,"TCP SEND:%i\n",l);
            //MyPrint();
            
            if (new_fd[l] && send(new_fd[l], cmd, strlen(cmd), 0) == -1) {
                sprintf(strToPrint,"server: Error send client:%i - %i\n",l,new_fd[l]);
                MyPrint();
            }
        }
        
        // WEB value POST
        else if (NewFDvalue[l]==2){
            SendPost (l,cmd);
        }
        
        l++;
    }
}
void Tcp_close (void) {
    sprintf(strToPrint,"TCP-server: Stop.");
    MyPrint();
    int l=1;
    while (l<fd_count){
        close(new_fd[l]);
        l++;
    }
    close(sockfd);
}


// ________________   W E B   S E R V E R   ________________ //
void MakeGet (int thisFd){
    int htmlByteCount=0,startByte=5;
    char htmlFile[256]="";

    // Get file str
    if (buf[htmlByteCount+startByte]=='/')startByte=6;
    while (buf[htmlByteCount+startByte]!=' '){
        htmlFile[htmlByteCount]=buf[htmlByteCount+5];
        htmlByteCount++;
    }
    if (strlen(htmlFile)<2) sprintf(htmlFile,"index.html");
    sprintf(strToPrint,"TCP:%i GET FILE:%s---",thisFd,htmlFile);
    MyPrint();
    

    /*   C G I   -   B I N   */
    if (htmlFile[0]=='c' && htmlFile[1]=='g' && htmlFile[2]=='i'){
        char scriptReq[128];
        strcpy (scriptReq,"");
        char scriptReqPost[128];
        strcpy (scriptReqPost,"");
        htmlByteCount=htmlByteCount-8;
        memmove (htmlFile,htmlFile+8,htmlByteCount);
        //strncpy(scriptReq,htmlFile,htmlByteCount);
        int i=0;
        while (i<htmlByteCount && htmlFile[i]!='?') {scriptReq[i]=htmlFile[i];i++;}scriptReq[i]='\0';i++;
        int i2=0;
        while (i<htmlByteCount) {scriptReqPost[i2]=htmlFile[i];i++;i2++;}scriptReqPost[i2]='\0';

        
        sprintf(strToPrint,"TCP:%i GET CGI:%s - %s ---",thisFd,scriptReq,scriptReqPost);
        MyPrint();
        
        
        int lastSlash=0;
        int count=0;
        while (count<strlen(scriptReq)){
            if (scriptReq[count]=='/') lastSlash=count;
            count++;
        }
        char scriptPath[128];
        strcpy (scriptPath,"");
        i=0; if (lastSlash) {while (i<lastSlash) {scriptPath[i]=scriptReq[i];i++;}scriptPath[i]='\0';}

        
        /* Change directory. */
        char workingDir[128];
        if (lastSlash) sprintf(workingDir,"%s/%s",CGI_FOLDER,scriptPath);
        else sprintf(workingDir,"%s",CGI_FOLDER);
        chdir(workingDir);
        
        
        
        
        
        /* Open the command for reading. */
        FILE *fp;
        char scriptFile[128];
        sprintf(scriptFile,"%s/%s %s",CGI_FOLDER,scriptReq,scriptReqPost);
        fp = popen(scriptFile, "r");
        if (fp == NULL) {
            sprintf(strToPrint,"Failed to run command");
            MyPrint();
        }
        /* Read the output a line at a time - output it. */
        char html[20480];
        sprintf(html,"");
        char oneLine[20480];
        fgets(oneLine, sizeof(html)-1, fp);
        if (strcmp(oneLine,"Content-type: text/html\n")==0){
            fgets(oneLine, sizeof(html)-1, fp);
            /*while (fgets(oneLine, sizeof(html)-1, fp) != NULL) {
                sprintf(html,"%s%s",html,oneLine);
            }*/
            char ch;
            while((ch=fgetc(fp)) != EOF) {
                sprintf(html,"%s%c",html,ch);
                //send(new_fd[thisFd], line, strlen(line), 0);
                //printf("%c", ch);
            }
        }
        pclose(fp);
        

        // Generate and send HEAD HTML
        char htmlAnswer[2048];
        sprintf(htmlAnswer,"HTTP/1.1 200 OK\nContent-Length: %i\n\n",(int)strlen(html));
        send(new_fd[thisFd], htmlAnswer, strlen(htmlAnswer), 0);
        // Send html
        send(new_fd[thisFd], html, strlen(html), 0);
        
        sprintf(strToPrint,"OK:%s ---",htmlFile);
        MyPrint();
    }


    /*   H T M L   */
    else {
        // Open file
        char htmlFolderAndFile[256]="";
        sprintf(htmlFolderAndFile,"%s/%s",WWW_FOLDER,htmlFile);
        char html[2048]="";
        FILE *fp;
        char line[2048];
        fp = fopen(htmlFolderAndFile,"r");
        if (fp != NULL){
            // Get size
            int prev=ftell(fp);
            fseek(fp, 0L, SEEK_END);
            int htmlLen=ftell(fp);
            fseek(fp,prev,SEEK_SET);
            
            sprintf(strToPrint,"SIZE:%i",htmlLen);
            MyPrint();
            
            // Generate and send HEAD HTML
            char htmlAnswer[2048];
            // Detetct type of file
            if (htmlFile[strlen(htmlFile)-1]=='g') {
                sprintf(htmlAnswer,"HTTP/1.1 200 OK\nContent-Length: %i\nAccept-Ranges: bytes\nContent-Type: image/jpeg\r\n\r\n",htmlLen);
                sprintf(strToPrint,"IMAGE");
                MyPrint();
                
            }
            else sprintf(htmlAnswer,"HTTP/1.1 200 OK\nContent-Length: %i\nContent-Type: text/html\r\n\r\n",htmlLen);
            send(new_fd[thisFd], htmlAnswer, strlen(htmlAnswer), 0);
            
            // Send file
            //while ( fgets ( line, sizeof line, fp ) != NULL ) {
                //send(new_fd[thisFd], line, strlen(line), 0);
            //}
            
            char ch;
            while((ch=fgetc(fp)) != EOF) {
                sprintf(line,"%c",ch);
                send(new_fd[thisFd], line, strlen(line), 0);
                //printf("%c", ch);
            }
            //sprintf(line,"%c",ch);
            //send(new_fd[thisFd], line, strlen(line), 0);
            //sprintf(line,"\r\n\r\n");
            //send(new_fd[thisFd], line, strlen(line), 0);
            
            sprintf(strToPrint,"OK:%s ---",htmlFile);
            MyPrint();
        }
        else {
            sprintf(strToPrint,"NO FILE:%s ---",htmlFile);
            MyPrint();
            
            char htmlHotFound[]="<html><head><title>404 Not Found</title></head><body bgcolor=white><center><h1>404 Not Found</h1></center><hr><center>myhome</center></body></html>";
            sprintf(html,"HTTP/1.0 404 Not Found\nContent-Length: %li\n\n%s",strlen(htmlHotFound),htmlHotFound);
            send(new_fd[thisFd], html, strlen(html), 0);
        }
        fclose(fp);
    }
}


void MakeOpt (int thisFd){
    sprintf(strToPrint,"TCP:%i OPT:",thisFd);
    MyPrint();
    
    // GETTING UserOrign
    int r, r1, body;
    r=0;r1=0;body=0;
    memset (UserOrign[thisFd],'\0',50);
    while (r<=strlen(buf)){
        if (buf[r]=='\n' && buf[r+1]=='O' && buf[r+2]=='r' && buf[r+3]=='i'){
            body=1;
            r=r+9;
            r1=0;
        }
        if (body==1) {
            if (buf[r]=='\n') break;
            UserOrign[thisFd][r1]=buf[r];
            r1++;
        }
        r++;
    }
    
    if(viewToScreen)printf("OPT - Client:%i - WEB ip:%s - Ori:%s\n",thisFd,s,UserOrign[thisFd]);
    
    char ok[1024];
    strcpy (ok,"HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: ");
    strcat (ok,UserOrign[thisFd]);
    strcat (ok,"\nAccess-Control-Request-Headers: origin, content-type\nAccess-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\nDate: Wed, 17 Jul 2015 13:03:49 GMT\nServer: Myhome\nContent-Length: 0\nKeep-Alive: timeout=6, max=6\nConnection: Keep-Alive\nContent-Type: text/plain\n\n");
    
    send(new_fd[thisFd], ok, strlen(ok), 0);
}


void MakePost (int thisFd){
    //sprintf(strToPrint,"POST:%i - %s:%i\n",thisFd,buf,(int)strlen(buf));
    //MyPrint();

    if (NewFDvalue[thisFd]==2){
//        sprintf(strToPrint,"TCP POST:%i - %s\n",thisFd,buf);
//        MyPrint();
//        MakeUserAuto(buf,1);
        // GETTING Comand
        int r, r1, body;
        r=strlen(buf);
        while (r>0 && buf[r]!='\n') r--;
        if (r!=0){
            char buf1[50];memset(buf1,'\0',50);
            r=r+1;r1=0;
            while (r<strlen(buf)){
                if (buf[r]=='\n') break;
                buf1[r1]=buf[r];
                r++;r1++;
            }
if (strlen(buf1)>2 && buf1[1]!='w') {
            MakeUserAuto(buf1,1);
            sprintf(strToPrint,"POST-FD-2:%i - %s\n",thisFd,buf1);
            MyPrint();
}
        }
	else {
if (strlen(buf)>2 && buf[1]!='w') {
            MakeUserAuto(buf,1);
            sprintf(strToPrint,"POST-FD-2:%i - %s\n",thisFd,buf);
            MyPrint();
}
	}
    }
    
    else {
        // GETTING UserOrign
        int r, r1, body;
        if (NewFDvalue[thisFd]==1){
            r=0;r1=0;body=0;
            memset (UserOrign[thisFd],'\0',50);
            while (r<=strlen(buf)){
                if (buf[r]=='\n' && buf[r+1]=='O' && buf[r+2]=='r' && buf[r+3]=='i'){
                    body=1;
                    r=r+9;
                    r1=0;
                }
                if (body==1) {
                    if (buf[r]=='\n') break;
                    UserOrign[thisFd][r1]=buf[r];
                    r1++;
                }
                r++;
            }
        }
        //sprintf(strToPrint,"TCP orign1:%i - %s\n",thisFd,UserOrign[thisFd]);
        //MyPrint();

        NewFDvalue[thisFd]=2;
        
        // GETTING Comand
        r=strlen(buf);
        while (buf[r]!='\n') r--;
        if (r<strlen(buf)-1){
            char buf1[50];memset(buf1,'\0',50);
            r=r+1;r1=0;
            while (r<=strlen(buf)){
                if (buf[r]=='\n') break;
                buf1[r1]=buf[r];
                r++;r1++;
            }
if (buf1[1]!='w' && r1>2)            MakeUserAuto(buf1,1);
            sprintf(strToPrint,"POST-FD-1:%i - %s\n",thisFd,buf1);
            MyPrint();
        }
        
    }
}
void SendPost (int thisFd, char *cmd){
    char ok[2048];
    sprintf (ok,"HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: %s\nKeep-Alive: timeout=6, max=6\nConnection: Keep-Alive\nContent-Length: %i\n\n%s",UserOrign[thisFd],(int)strlen(cmd),cmd);
if (new_fd[thisFd] || new_fd[thisFd]==8)    send(new_fd[thisFd], ok, strlen(ok), 0);//NewFDvalue[thisFd]=1;
}



