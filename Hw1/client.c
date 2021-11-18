#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



int main(int argc,char* argv[]){
    //create socket
    int sockfd = 0;
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    if(sockfd == -1){  //connection error
        printf("Socket create failed.\n");
        return 3;
    }

    //-----set socket address info-----
    struct sockaddr_in serv_info;

    bzero(&serv_info , sizeof(serv_info));
    serv_info.sin_family = PF_INET;
    serv_info.sin_port = htons(8081);
    serv_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    //----------------------------------

    if(connect(sockfd , (struct sockaddr* ) &serv_info , sizeof(serv_info)) == -1 ){
        perror("Connection error ");
    }

    return 0;
}