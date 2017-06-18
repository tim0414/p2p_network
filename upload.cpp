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
#include "upload.h"

#define BUFFER_SIZE 1024



void upload::upload_file(int sockfd, char*filename){
	ssize_t n;
	char file_buffer[2048], recvline[2048], file_fail[2048];
	
	
	memset(&file_buffer, 0, sizeof(file_buffer));
	FILE *fp;
/*	
	printf("please enter the file name: ");
	
	fgets(filename, 1024, stdin);
	if(filename[strlen(filename)-1] == '\n')
		filename[strlen(filename)-1] ='\0';

	write(sockfd, filename, strlen(filename)+1);
*/

	printf("upload: %s\n", filename);

	chdir("file");
	fp = fopen(filename, "rb");
	int nCount;	
	
	int i = 0;
	int recv_num, num;

	printf("start uploading\n");
	
	while( (nCount = fread(file_buffer, sizeof(char), BUFFER_SIZE,  fp)) > 0){
		num = send(sockfd, file_buffer, nCount, 0);
		
		printf("----------------\n");
		printf("send: %d\n", num);
		printf("%d recv success\n", i);
		printf("----------------\n\n");
		
		recv(sockfd, &recv_num, sizeof(recv_num), 0);
		memset(&file_buffer, 0, sizeof(file_buffer));
		i ++;
	}
	fclose(fp);
	chdir("..");
}


void upload::p2p_upload(int sockfd){
    ssize_t n;
	char file_buffer[BUFFER_SIZE], sock_buffer[BUFFER_SIZE], file_name[BUFFER_SIZE];
	char over[] = "over";
	char filename[64];
	memset(&filename, 0, sizeof(filename));


	read(sockfd, filename, 64);

	printf("start p2p upload, filename: %s\n", filename);

	int total, no;
	memset(&total, 0, sizeof(total));
	memset(&no, 0, sizeof(no));
	read(sockfd, &total, sizeof(total));
	read(sockfd, &no, sizeof(no));
	printf("total file number: %d, no: %d\n", total, no);
	
	
	memset(&file_buffer, 0, sizeof(file_buffer));
	memset(&sock_buffer, 0, sizeof(sock_buffer));
	FILE *fp;

	chdir("file");

    //get file size
	struct stat st;
	int size;
	stat(filename, &st);
	size = st.st_size;
	int part = size/total;
	int bias = part*(no-1);


	
	fp = fopen(filename, "rb");

	
	printf("download: %s, file size: %d, part size: %d, bias: %d\n", filename, size, part, bias);
	int nCount;	
	int i = 0;
	int num = 0;
	int recv_num;
	int send_byte = 0;

	fseek(fp, bias, SEEK_SET);
	if(total==no) part = part+(size%total);
	printf("send size: %d\n", part);
	//memset(&sock_buffer, 0, sizeof(sock_buffer));
	int buf_size = 1024;
	if(buf_size>part) buf_size = part;

	send_byte = send_byte + buf_size;
	while( (nCount = fread(file_buffer, sizeof(char), buf_size,  fp)) >0){
		
	
		//printf("----------------\n");	
		num = send(sockfd, file_buffer, nCount, 0);
		//printf("nCount is %d, i = %d, success: %d\n", nCount, i, num);

		//printf("%d recv success\n", i);
		//printf("----------------\n");
		//printf("\n");
		memset(&file_buffer, 0, sizeof(file_buffer));
		recv(sockfd, &recv_num, sizeof(recv_num), 0);
		i ++;
		send_byte = send_byte + buf_size;
		if(send_byte>part)buf_size = part - (send_byte-buf_size);
		
	}
	printf("writing complete\n");
	fclose(fp);
	chdir("..");
	
	return;

}

