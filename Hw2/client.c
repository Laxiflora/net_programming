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
#define MAPSIZE 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


pthread_mutex_t lock_tern = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t lock_cond = PTHREAD_COND_INITIALIZER;
int invitation = 0;
int turn = 0;
int isFinish = 0;
char mySymbol;
char hisSymbol;
char map[MAPSIZE][MAPSIZE]={0};


typedef struct{
    int socket_fd;
    char* username;
}Data;

void map_init(){
    for(int i=0;i<MAPSIZE;i++){
        for(int j=0;j<MAPSIZE;j++){
            map[i][j]='N';
        }
    }
}

void drawmap(){
    for(int i=0;i<MAPSIZE;i++){
        for(int j=0;j<MAPSIZE;j++){
            if(map[i][j]=='N'){
                printf("N ");
            }
            else if(map[i][j]=='O'){
                printf("O ");
            }
            else if(map[i][j]=='X'){
                printf("X ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

int check_lose(){
	for(int i = 0;i < 3;i++)
		if(map[i][0] == map[i][1] && 
			map[i][0] == map[i][2] &&
			map[i][0] == hisSymbol){   //I win
            return 1;
            }



	for(int j = 0;j < 3;j++)
		if(map[0][j] == map[1][j] &&
			map[0][j]== map[2][j] && 
			map[0][j] == hisSymbol){
                return 1;
            }

    
	if(map[0][0] == map[1][1] &&
		map[0][0] == map[2][2] &&
		map[0][0] == hisSymbol){
            return 1;
        }


	if(map[0][2] == map[1][1] &&
		map[0][2] == map[2][0] &&
		map[0][2] == hisSymbol){
            return 1;
        }
	return 0;
}


int check_win(){
	for(int i = 0;i < 3;i++)
		if(map[i][0] == map[i][1] && 
			map[i][0] == map[i][2] &&
			map[i][0] == mySymbol){   //I win
            return 1;
            }



	for(int j = 0;j < 3;j++)
		if(map[0][j] == map[1][j] &&
			map[0][j]== map[2][j] && 
			map[0][j] == mySymbol){
                return 1;
            }

    
	if(map[0][0] == map[1][1] &&
		map[0][0] == map[2][2] &&
		map[0][0] == mySymbol){
            return 1;
        }


	if(map[0][2] == map[1][1] &&
		map[0][2] == map[2][0] &&
		map[0][2] == mySymbol){
            return 1;
        }
	return 0;
}


int check_tie(){
    for(int i=0;i<MAPSIZE;i++){
        for(int j=0;j<MAPSIZE;j++){
            if(map[i][j]=='N'){
                return 0;
            }
        }
    }
    return 1;
}




void chess_fight(int socketfd,int first){
    map_init();
    while(1){
        char input[10];
        char query[10];
        int x_axis = 0, y_axis = 0;
        if(first){
            first--;
            drawmap();
            fflush(stdin);
            fgets(input,10,stdin);
            fflush(stdin);
            sprintf(query,"m %s",input);
            y_axis = input[0]-'0';
            x_axis = input[2]-'0';
            map[x_axis-1][y_axis-1] = 'O';


            printf("query = %s\n",input);
            send(socketfd,query,sizeof(query),0);
        }
        else{
            drawmap();
            pthread_mutex_lock(&lock_tern);
            pthread_cond_wait(&lock_cond,&lock_tern); //your turn
            drawmap();
            if(check_lose() == 1){
                printf("You Lose.\n");
                invitation=0;
                pthread_cond_signal(&lock_cond);
                pthread_mutex_unlock(&lock_tern);
                break;
            }
            if(check_tie()==1){
                printf("TIE!\n");
                invitation =0;
                pthread_cond_signal(&lock_cond);
                pthread_mutex_unlock(&lock_tern);
                break;
            }



            fflush(stdin);
            fgets(input,10,stdin);
            fflush(stdin);
            sprintf(query,"m %s",input);
            y_axis = input[0]-'0';
            x_axis = input[2]-'0';
            map[x_axis-1][y_axis-1] = mySymbol;
            printf("input = %s\n",input);
            send(socketfd,query,sizeof(query),0);
            pthread_mutex_unlock(&lock_tern);
            drawmap();
            if(check_win()==1){   //I win
                printf("YOU WIN!\n");
                send(socketfd,"gw",sizeof("gw"),0);
                invitation = 0;
                break;
            }
            if(check_tie()==1){
                printf("TIE!\n");
                send(socketfd,"gt",sizeof("gt"),0);
                invitation =0;
                break;
            }
        }
    }
}



void list_players(int socket_fd){
    send(socket_fd, "l", sizeof("l"), 0);
}


void catch_data(void* userDat){
    Data* userData = (Data*)userDat;
    char responce[BUFFER];
    int length=0;
    while(1){
        length = recv(userData->socket_fd,responce,BUFFER,0);
        responce[length]='\0';
        if(strcmp(responce,"exit")==0){
            pthread_exit(NULL);
        }
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
                    invitation = 2;
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
        else if(responce[0] == 'm'){   //chessgame movement "m O 2 1" or "m X 2 1"
            pthread_mutex_lock(&lock_tern);
            int y_axis = (int)responce[4]-'0';
            int x_axis = (int)responce[6]-'0';
            char symbol = responce[2];
            if(symbol == 'O'){mySymbol = 'X'; hisSymbol = 'O';}
            if(symbol == 'X'){mySymbol = 'O'; hisSymbol = 'X';}
            map[x_axis-1][y_axis-1] = symbol;
            pthread_cond_signal(&lock_cond); //your turn
            pthread_mutex_unlock(&lock_tern);
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
    int exit =0;
    while(!exit){
        if(invitation >= 1){   //accept a chess_game invite
            int first =0;
            printf("entering the game...\n");
            if(invitation==2) first=1;
            chess_fight(userData.socket_fd,first);
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
            case 3 : //refresh
                ;
                break;
            case 4 : //exit
                ;
                send(socket_fd,"exit",sizeof("exit"),0);
                exit = 1;
                break;
            default:
                break;
        }

    }   
    printf("See you next time.\n");
//關閉
    pthread_join(recv_thread,NULL);
	close(socket_fd);
	return 0;
}