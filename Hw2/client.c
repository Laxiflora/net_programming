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

typedef struct{
    int socket_fd;
    char* username;
}Data;


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



    }




}


void invite(char* targetname,int socket_fd){
    
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
        printf("welcome %s ! please enter your action...\n\
        1: list online players\n\
        2:invite someone to play\n\
        3:exit\n",username);
        int choice =0;
        scanf("%d" ,&choice);
        switch(choice){
            case 1 :
                list_players(socket_fd);
                break;

            case 2 :
                ;
                char targetname[USERNAME_BUFF];
                printf("enter the player you want to play with:\n");
                fgets(targetname, USERNAME_BUFF, stdin);
                invite(targetname,socket_fd);
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