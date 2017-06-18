#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <list>
using std::list;
#include "connection.h"



void connection::client_init(){
    printf("testing\n");
    
    memset(&addr_svr, 0, sizeof(addr_svr));
	addr_svr.sin_family = AF_INET;
	addr_svr.sin_port = htons(port);
	addr_svr.sin_addr.s_addr = inet_addr(ip);
    
}

int connection::server_init(){
    memset(&addr_svr, 0, sizeof(addr_svr));
	
	addr_svr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
	addr_svr.sin_family = AF_INET;
	addr_svr.sin_port = htons(port);
	printf("%d", port);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	int sock_opt = 1;
	if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt))==-1 ){
		printf("setsockopt REUSEADDR false\n");
	}

	if(bind(sockfd, (struct sockaddr *)&addr_svr, sizeof(addr_svr)) == -1){
		printf("ERROR: bind()\n");
	}

		
	if(listen(sockfd, 10) == -1){
		printf("ERROR: listen()\n");
	}

    return sockfd;
}


int connection::connect_to_server(){

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		printf("ERROR: socket()\n");
		exit(1);
	}

	if(connect(sockfd, (struct sockaddr *)&addr_svr, 
		sizeof(addr_svr)) == -1){
		printf("ERROR: connect()\n");
		exit(1);
	}
	
	//file_exist(sockfd);
	//cmd(sockfd);
    return sockfd;
    
}


void connection::inputID(){
    char id[64];
	//fgets(buffer, sizeof(buffer), stdin);
	printf("please enter id: ");
	fgets(id, 64, stdin);
	if(id[strlen(id)-1]=='\n')
		id[strlen(id)-1]='\0';
	write(sockfd, id, strlen(id));

}

