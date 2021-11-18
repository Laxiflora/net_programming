#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define BUFFER_SIZE 8096




void sendData(int,char*,char*);

void getData(int clifd , char* buffer , char* fstr , char* extension){
    int tokenLength=0;
    char temp[128];
    sprintf(temp,"Content-Type: %s",fstr);
    char* start = strstr(buffer , temp);  //image/jpeg
    char* end = strstr(start,"-------");
    start+=28;
    char rawData[BUFFER_SIZE];
    memcpy(rawData,start,(end-start));
    char dataName[128];
    sprintf(dataName,"upload%s",extension);
    if(!strcmp(extension,".jpg") || !strcmp(extension,".png")){
        for(int i=0;i<sizeof(buffer);i++){
            printf("%c",buffer[i]);
        }
        FILE* fd = fopen(dataName,"wb");
        fprintf(fd,"%s",rawData);
        fclose(fd);
    }
    else{
        FILE* fd = fopen(dataName,"w+");
        fprintf(fd,"%s",rawData);
        fclose(fd);
    }
    sendData(clifd , "./index.html","text/html");
}

void sendData(int clifd , char* requestedFile,char* contentType){
    long ret=0;
    static char buffer[BUFFER_SIZE+1];
    int datafd = open(requestedFile , O_RDONLY);
        if(datafd == -1){
            perror("open failed :");
        }
        sprintf(buffer , "HTTP/1.1 200 OK\r\nContent-Type: %s;charset=UTF-8\r\n\r\n",contentType);
        write(clifd , buffer , strlen(buffer));
        while((ret = read(datafd , buffer , BUFFER_SIZE)) >0){
            write(clifd,buffer,ret);
        }
}



void handle_request(int clifd){

    char* category[5] = {"image/jpeg","image/png","text/html","text/plain","image/zip"};
    char* category_[5] = {".jpg",".png",".html",".txt",".zip"};


    long ret = 0;
    static char buffer[BUFFER_SIZE+1];
    ret = read(clifd , buffer , BUFFER_SIZE);
    if(ret==0 || ret ==1){
        printf("Connection Error.");
        exit(3);
    }


    int buflen = strlen(buffer);
	char fstr[128],extension[128];
/*-----------------------判別種類--------------*/
    if(strstr(buffer,"image/jpeg")!=NULL){
        sprintf(fstr,"%s","image/jpeg");
        sprintf(extension,"%s",".jpg");
    }
    if(strstr(buffer,"image/png")!=NULL){
        sprintf(fstr,"%s","image/png");
        sprintf(extension,"%s",".png");
    }
    if(strstr(buffer,"text/html")!=NULL){
        sprintf(fstr,"%s","text/html");
        sprintf(extension,"%s",".html");
    }
    if(strstr(buffer,"text/plain")!=NULL){
        sprintf(fstr,"%s","text/plain");
        sprintf(extension,"%s",".txt");
    }
    if(strstr(buffer,"image/zip")!=NULL){
        sprintf(fstr,"%s","image/zip");
        sprintf(extension,"%s",".zip");
    }


/*
	for(int i=0;i<5;i++) {
        char* temp = buffer;

		if(strstr(temp,category[i]) != NULL) {
            printf("i等於:%d\n",i);
			sprintf(fstr,"%s",category[i]);
            sprintf(extension,"%s",category_[i]);
            printf("fstr等於:%s",fstr);
			break;
		}
    }
*/
/*--------------------------------------*/
    if(!strncmp(buffer, "GET / " , 6) || !strncmp(buffer , "get / ",6)){  //GET -> requesting html
        sendData(clifd , "./index.html","text/html");
    }
    else if(!strncmp(buffer, "GET /" , 5) || !strncmp(buffer , "get /",5)){//GET other else -> requesting img
        sendData(clifd , "pic.jpg" , "image/jpeg");
    }
    else if(!strncmp(buffer, "POST /" , 5) || !strncmp(buffer , "post /",5)){// POST -> ready to get data
        getData(clifd , buffer, fstr , extension);
    }
    
    exit(1);
}

void sigchld(int signo)
{
	printf("child in sigchild\n");
    pid_t pid;
	
	while((pid=waitpid(-1, NULL, WNOHANG))>0);
}




int main(int argc,char* argv[]){
    //create socket
    int sockfd = 0;
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    if(sockfd == -1){  //connection error
        perror("Socket create failed.\n");
        return 3;
    }

    //-----set socket address info-----
    struct sockaddr_in serv_info,cli_info;
    socklen_t address_len = sizeof(cli_info);
    bzero(&serv_info , sizeof(serv_info));
    serv_info.sin_family = PF_INET;
    serv_info.sin_port = htons(8080);
    serv_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(sockfd , (struct sockaddr*) &serv_info , sizeof(serv_info) );
    listen(sockfd,5);
    signal(SIGCHLD, sigchld);

    //----------------------------------
    while(1){
        int clifd = accept(sockfd , (struct sockaddr*) &cli_info , &address_len);
        int pid = 0;
        if( clifd<0 ){
            perror("Socket accept failed :");
            exit(3);
        }
        //------fork---------
        if((pid = fork())<0){
            perror("fork failed : ");
            exit(3);
        }
        else if(pid == 0){    //child process: close listenfd
            close(sockfd);
            handle_request(clifd);
            exit(0);
        }
        else{                //parent process: close clientfd
            close(clifd);
        }
        //-------------------
    }



    

    return 0;
}

/**
 * GET / HTTP/1.1
Host: 127.0.0.1:8080
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:94.0) Gecko/20100101 Firefox/94.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,* / *;q=0.8
Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.5,en;q=0.3
Accept-Encoding: gzip, deflate
Connection: keep-alive
Upgrade-Insecure-Requests: 1
Sec-Fetch-Dest: document
Sec-Fetch-Mode: navigate
Sec-Fetch-Site: none
Sec-Fetch-User: ?1

**/

/*
 * POST / HTTP/1.1
Host: 127.0.0.1:8080
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:94.0) Gecko/20100101 Firefox/94.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,* / *;q=0.8
Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.5,en;q=0.3
Accept-Encoding: gzip, deflate
Content-Type: application/x-www-form-urlencoded
Content-Length: 38
Origin: http://127.0.0.1:8080
Connection: keep-alive
Referer: http://127.0.0.1:8080/
Upgrade-Insecure-Requests: 1
Sec-Fetch-Dest: document
Sec-Fetch-Mode: navigate
Sec-Fetch-Site: same-origin
Sec-Fetch-User: ?1
 * **/