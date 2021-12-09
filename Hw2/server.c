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
#define REQUEST_BUFF 30
#define BUFFER 8000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


typedef struct{
    pthread_t* pthread;
    int connfd;
    char username[USERNAME_BUFF];
    int using;  //is it using?
    struct sockaddr* cli_addr;
    socklen_t length;
    int pair;
    char symbol;
}PlayerData;

PlayerData playerList[MAXUSER];


int findEnemy(int index){
    int i;
    for(i=0; playerList[i].using!=0;i++){
        if(playerList[i].pair == playerList[index].connfd){
            break;
        }
    }
    return playerList[i].connfd;
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

void init_playerList(int index){
    playerList[index].pair = -1;
    playerList[index].symbol = '\0';
}


void lobby(int index){
    //接收使用者名稱
    int length;
	length = recv(playerList[index].connfd, playerList[index].username, USERNAME_BUFF, 0);
	if(length>0) {
		playerList[index].username[length]= '\0';
	}
    printf("Player %s enter the lobby.\n",playerList[index].username);
    //將號碼牌交給使用者(就是server產生的connfd)
        char temp[10];
        sprintf(temp,"%d",playerList[index].connfd);
        send(playerList[index].connfd,temp,strlen(temp)+1,0);
    
    while(1){  //持續接收使用者傳輸的資料
        char request[REQUEST_BUFF];
        length = recv(playerList[index].connfd, request, REQUEST_BUFF, 0);
        request[length] = '\0';
//        printf("收到%d的要求：%s\n",playerList[index].connfd,request);

        if(strcmp(request,"exit")==0){
            send(playerList[index].connfd,"exit",sizeof("exit"),0);
            printf("User %s exit.\n",playerList[index].username);
            init_playerList(index);
//            playerList[index].using = 0;
            close(playerList[index].connfd);
            playerList[index].connfd = -1;
            pthread_exit(NULL);
        }
    
        switch(request[0]){
            case 'l' :   //list player request
                ;
                list_player(playerList[index].connfd);
                break;
            case 'i' :   //invite player to play
                ;
                //entract enemy player fd
                char str[80];
                memcpy(str,request+2,strlen(request)-2);
                int enemy = strtol(str,NULL,10);

                //get enemy player name
                for(int i=0;playerList[i].using!=0;i++){
                    if(playerList[i].connfd == enemy){
                        char invitation[80];
                        sprintf(invitation,"i %s",playerList[index].username);
                        send(enemy,invitation,sizeof(invitation),0);
                        playerList[index].pair=enemy;
                        playerList[index].symbol = 'X';
                    }
                }
                break;

            case 'Y' : //Accept duel
                    ;
                    enemy = findEnemy(index);
                    playerList[index].pair = enemy;
                    playerList[index].symbol = 'O';
                    send(enemy,"A1",sizeof("A1"),0);                  

            case 'N':  //Decline duel
                    ;
                    enemy = findEnemy(index);
                    playerList[index].pair = enemy;
//                    printf("send %d A1\n",enemy);
                    send(enemy,"A0",sizeof("A0"),0);                  

            case '0':
                ;
                break;

            case 'm':   //game movement   m 2 1
                ;
                
                char resp[10];
                enemy = findEnemy(index);
                sprintf(resp,"m %c %c %c",playerList[index].symbol,request[2],request[4]);
//                printf("RESP = %s" , resp);
                send(enemy,resp,sizeof(resp),0);


                break;
            case 'g':
                ;
                enemy = findEnemy(index);
                char opt = request[1];
                if(opt == 'w'){
                    init_playerList(index);
                    init_playerList(enemy);           
                }
                else if(opt == 't'){
                    init_playerList(index);
                    init_playerList(enemy);
                }
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
        playerList[i].pair = -1;
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
        printf("%d got connected in %d\n",playerList[i].connfd,i);
        playerList[i].using=1;
        playerList[i].pthread = malloc(sizeof(pthread_t));
        pthread_create(playerList[i].pthread, NULL, (void*)(&lobby), (void*)i);
        pthread_detach(*playerList[i].pthread);
    }


    return 0;
}