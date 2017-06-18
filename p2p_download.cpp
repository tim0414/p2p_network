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
#include "p2p_download.h"
using std::list;

#define BUFFER_SIZE 1024

struct p2p_info{
	char filename[128];
	char ip[128];
	int total;
	int no;
};

struct address{
	int port;
	char ip[1024];
	int sockfd;
};


void p2p_download::download(int sockfd, char *filename, int total, int no){
	FILE *fp1;
	char temp_file[2];
	char sendline[1024], recvline[2048], buffer[1024];
	int nCount;
	sprintf(temp_file, "%d", no);

	printf("start p2p download, filename %s\n", filename);
	write(sockfd, &total, sizeof(total));
	write(sockfd, &no, sizeof(no));
	

	//chdir("file");
	//strcat(filename, c);
	fp1 = fopen(temp_file, "wb");

	printf("p2p filename: %s\n", filename);

	int i = 0;
	memset(&recvline, 0, sizeof(recvline));

	// start downloading file		
	while( (nCount = recv(sockfd, recvline, BUFFER_SIZE, 0)) > 0){
		//printf("start recv\n");

		fwrite(recvline, sizeof(char), nCount, fp1); 

		if(nCount!=BUFFER_SIZE){
			printf("----------------\n");
			printf("%d recv success\n", i);
			printf("nCount is %d, i = %d\n", nCount, i);
			printf("----------------\n\n");
			//printf("%s\n", recvline);
		}
		send(sockfd, &nCount, sizeof(nCount), 0);
		memset(&recvline, 0, sizeof(recvline));
		
		if(nCount < BUFFER_SIZE) break;
		i ++;
	}	
		//printf("Download complete!\n");
	fclose(fp1);
	//chdir("..");
	

}
