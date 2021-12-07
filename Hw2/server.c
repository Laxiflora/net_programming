#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#define MAXUSER 10
#define USERNAME_BUFF 10
#define REQUEST_BUFF 10
#define BUFFER 8000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    pthread_t* pthread;
    int connfd;
    char username[USERNAME_BUFF];
    int using;  //is it using?
    struct sockaddr* cli_addr;
    socklen_t length;
}PlayerData;

PlayerData playerList[MAXUSER];


void chessgame(int enemyfd,int indexfd){
    printf("in chess game");
}



void list_player(int caller){
    char responce[BUFFER];
    int j=0;
    for(int i=0;playerList[i].using!=0;i++){
        responce[j++] = '[';
        char temp[10];
        sprintf(temp,"%d",playerList[i].connfd);
        strcat(responce,temp);
        j+=strlen(temp);
        responce[j++] = ']';
        responce[j++] = ' ';
        strcat(responce,playerList[i].username);
        j+=strlen(playerList[i].username);
        responce[j++]='\n';
    }
    send(caller,responce,j,0);
}


void lobby(int index){
    //接收使用者名稱
    int length;
    printf("flagA\n");
	length = recv(playerList[index].connfd, playerList[index].username, USERNAME_BUFF, 0);
    printf("flagB\n");
	if(length>0) {
		playerList[index].username[length]= '\0';
	}
    //將號碼牌交給使用者(就是server產生的connfd)
    printf("flagC\n");
        char temp[10];
        sprintf(temp,"%d",playerList[index].connfd);
        printf("%s\n",temp);
        send(playerList[index].connfd,temp,strlen(temp)+1,0);
    printf("flagD\n");
    
    while(1){  //持續接收使用者傳輸的資料
        printf("flagE\n");
        char request[REQUEST_BUFF];
        length = recv(playerList[index].connfd, request, REQUEST_BUFF, 0);
        request[length] = '\0';
        switch(request[0]){
            case 'l' :   //list player request
                ;
                printf("flagF\n");
/*          
                char* str;
                memcpy(str,request+2,sizeof(request)-sizeof(char)*2);
                printf("caller = %s",str);
                int caller = strtol(str,NULL,10);
                */
                list_player(playerList[index].connfd);
                printf("flagG\n");
                break;
            case 'i' :   //invite player to play
                ;
                //entract enemy player fd
                char str[80];
                memcpy(str,request+2,strlen(request)-2);
                printf("enemy = %s\n",str);
                int enemy = strtol(str,NULL,10);
                printf("num_enemy = %d", enemy);

                //get enemy player name
                for(int i=0;playerList[i].using!=0;i++){
                    if(playerList[i].connfd == enemy){
                        char invitation[80];
                        sprintf(invitation,"i %s",playerList[index].username);
                        printf("invitation %s" , invitation);
                        send(enemy,invitation,sizeof(invitation),0);
                        recv(enemy,invitation,sizeof(invitation),0);
                        if(invitation[2]=='Y'){
                            send(index,"i Y",sizeof("i Y"),0);
                            chessgame(enemy,index);
                        }
                    }
                }
                break;
            case 'm':   //game movement
                ;
                break;

        }

    }




}


int main(int argc,char* argv[]){
    int sockfd = 0;
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    if(sockfd == -1){  //connection error
        perror("Socket create failed.\n");
        return 3;
    }

    //init
    for(int i=0;i<MAXUSER;i++){
        playerList[i].using=0;
        playerList[i].connfd=-1;
    }


    //
    //-----set socket address info-----
    struct sockaddr_in serv_info,cli_info;
    socklen_t address_len = sizeof(cli_info);
    bzero(&serv_info , sizeof(serv_info));
    serv_info.sin_family = PF_INET;
    serv_info.sin_port = htons(8080);
    serv_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(sockfd , (struct sockaddr*) &serv_info , sizeof(serv_info) );
    listen(sockfd,5);
//    signal(SIGCHLD, sigchld);

    //----------------------------------
    while(1){
        int i=0;
        for(i=0;i<MAXUSER;i++){
            if(playerList[i].using==0){
                break;
            }
        }
        playerList[i].connfd = accept(sockfd, playerList[i].cli_addr , &playerList[i].length);
        playerList[i].using=1;
        playerList[i].pthread = malloc(sizeof(pthread_t));
        pthread_create(playerList[i].pthread, NULL, (void*)(&lobby), (void*)i);
    }


    return 0;
}

/*
    pthread_mutex_unlock(&mutex);
    printf("LOCK>\n");
    pthread_mutex_lock(&mutex);
    */