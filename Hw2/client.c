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
#define USERNAME_BUFF 10
#define BUFFER 8000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int invitation = 0;


typedef struct{
    int socket_fd;
    char* username;
}Data;


void chess_fight(){

}



void list_players(int socket_fd){
    /*
    char* to_send;
    printf("Before");
    sprintf(to_send,"%d",socket_fd);
    printf("to send = %s" , to_send);
    */
    send(socket_fd, "l", sizeof("l"), 0);
    //wait for responce
}


void catch_data(void* userDat){
    Data* userData = (Data*)userDat;
    char responce[BUFFER];
    int length=0;
    while(1){
        length = recv(userData->socket_fd,responce,BUFFER,0);
        responce[length]='\0';
        if(responce[0] == '['){ //list_players responce
            printf("%s ",responce);
        }
        else if(responce[0]=='i'){   //chess invitation
            char quest[80];
            memcpy(quest,responce+2,strlen(responce)-2);
            if(quest[0]=='Y'){
                pthread_exit("YES");
            }
            else if(quest[0]=='N'){
                pthread_exit("NO");
            }
            else{
                printf("user ID %s has just invite you to have a game! Accept? [Yes/No]\n",quest);
                scanf("%s",responce);
                if(strcmp(responce,"Yes")==0){
                    send(userData->socket_fd,"Y",sizeof("Y"),0);
                    invitation = 1;
                    printf("You accepted a duel! press 3 to enter the contest.\n");
                }
                else{
                    send(userData->socket_fd,"N",sizeof("N"),0);
                    invitation = -1;
                }
//to fix
            }
        }
        else if (responce[0]=='A'){
            if(responce[1]=='1'){
                printf("Invite Accepted.\n");
                invitation=1;
                pthread_mutex_lock(&mutex);
                pthread_cond_signal(&cond);
                pthread_mutex_unlock(&mutex);
            }
            else{
                printf("Invite rejected.\n");
                pthread_mutex_lock(&mutex);
                pthread_cond_signal(&cond);
                pthread_mutex_unlock(&mutex);
            }
        }
    }
}


void invite(char* targetname,int socket_fd){
    char bridge[50];
    sprintf(bridge,"i %s",targetname);
    send(socket_fd,bridge,sizeof(bridge),0);
}



void connect_to_server(int socket_fd, struct sockaddr_in *address)
{
	int response = connect(socket_fd, (struct sockaddr *) address, sizeof *address);
	if (response < 0) {
		perror("connection failed :");
		exit(1);
	}else {
		printf("Connected\n");
	}
}



int main(int argc, char**argv)
{
	long port = strtol(argv[2], NULL, 10);
	struct sockaddr_in address, cl_addr;
	char * server_address;
	int socket_fd, response;
	char prompt[USERNAME_BUFF+4];
	char username[USERNAME_BUFF];
    pthread_t recv_thread;

//檢查arguments
	if(argc < 3) {
		printf("Usage: ./client [IP] [PORT]\n");
		exit(1);
	}

//進入大廳
	printf("Enter your ID: ");
	fgets(username, USERNAME_BUFF, stdin);
	username[strlen(username) - 1] = 0;
//連線設定
	server_address = argv[1];
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(server_address);
	address.sin_port = htons(port);
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	connect_to_server(socket_fd, &address);

//建立thread來接對戰邀請
    Data userData;
    userData.socket_fd = socket_fd;
    userData.username = username;
    pthread_create(&recv_thread,NULL,(void*)catch_data,&userData);

//傳送自己的名字給server
    send(socket_fd, userData.username, strlen(username), 0);

    while(1){
        if(invitation == 1){   //accept a chess_game invite
            printf("entering the game...\n");
            chess_fight();
        }

        printf("welcome %s ! please enter your action...\n\
        1: list online players\n\
        2:invite someone to play\n\
        3:update status\n\
        4:exit\n",username);
        int choice =0;
        scanf("%d" ,&choice);
        switch(choice){
            case 1 :
                list_players(socket_fd);
                break;

            case 2 :
                ;
                char to_duel[5];
                char targetname[USERNAME_BUFF];
                printf("enter the player ID you want to play with:");
                scanf("%s",targetname);
                invite(targetname,socket_fd);
                printf("Waiting for opponent...\n");
                pthread_mutex_lock(&mutex);
                pthread_cond_wait(&cond,&mutex);
                pthread_mutex_unlock(&mutex);
                break;
            case 3 :
                ;
                break;
            default:
                break;
        }

    }   




//關閉
	close(socket_fd);
	pthread_exit(NULL);
	return 0;
}