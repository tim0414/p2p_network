/* homework3 client*/

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


#define BUF_SIZE 1024

struct address{
	int port;
	char ip[1024];
	int sockfd;
};

struct p2p_info{
	char filename[128];
	char ip[128];
	int total;
	int no;
};




void p2p_upload(int sockfd){
	ssize_t n;
	char file_buffer[1024], sock_buffer[1024], file_name[1024];
	char over[] = "over";
	char filename[1024];
	memset(&filename, 0, sizeof(filename));


	read(sockfd, filename, 1024);

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

void p2p_download(int sockfd, char *filename, int total, int no){
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
	while( (nCount = recv(sockfd, recvline, BUF_SIZE, 0)) > 0){
		//printf("start recv\n");

		fwrite(recvline, sizeof(char), nCount, fp1); 

		if(nCount!=BUF_SIZE){
			printf("----------------\n");
			printf("%d recv success\n", i);
			printf("nCount is %d, i = %d\n", nCount, i);
			printf("----------------\n\n");
			//printf("%s\n", recvline);
		}
		send(sockfd, &nCount, sizeof(nCount), 0);
		memset(&recvline, 0, sizeof(recvline));
		
		if(nCount < BUF_SIZE) break;
		i ++;
	}	
		//printf("Download complete!\n");
	fclose(fp1);
	//chdir("..");
	


}

void *p2p_connect(void *arg){
	struct sockaddr_in addr_svr;
	int sockfd;
	uint16_t port = 9999;
	struct p2p_info *info = (struct p2p_info *)arg;
	char ip_num[1024];
	strcpy(ip_num, info->ip);
	//pthread_detach(pthread_self());
	//free(arg);

	printf("filename: %s, ip %s\n", info->filename, ip_num);
	
	memset(&addr_svr, 0, sizeof(addr_svr));
	addr_svr.sin_family = AF_INET;
	addr_svr.sin_port = htons(port);
	addr_svr.sin_addr.s_addr = inet_addr(ip_num);

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

	//write(sockfd, info->filename, strlen(info->filename));
	int total = info->total;
	int no = info->no;
	char filename[1024];
	sprintf(filename, "%s", info->filename);
	printf("p2p filename %s\n", filename);
	write(sockfd, filename, strlen(filename)+1);

	p2p_download(sockfd, filename, total, no);

	pthread_exit(NULL);

	//pthread_create(&tid,NULL, connect_server, (void *)&addr);

}

int download(int sockfd, char *filename)
{
	char sendline[1024], recvline[2048], buffer[1024];
	FILE *fp1;
	memset(&sendline, 0, sizeof(sendline));
	memset(&recvline, 0, sizeof(recvline));
	int nCount;
	int file_check;
	char client_ip[1024];
	int j=0;
	int file_num = 0;
	int server_has_file = 0;
	list<char*> ip_addr;
	struct address temp;
	pthread_t tid[32];
	char *ip_temp[32];
	void * thread_result;
	int thread_num = 0;

	memset(&file_num, 0, sizeof(file_num));
	recv(sockfd, &file_num, sizeof(file_num), 0);
	printf("total file number: %d\n", file_num);


	int no = 0;
	//for(int j=0;j<1;j++){
	while(1){
		memset(&client_ip, 0, sizeof(client_ip));
		recv(sockfd, client_ip, sizeof(client_ip), 0);
		send(sockfd, "start", sizeof("start"), 0);
		if(strcmp(client_ip, "over")==0){
			printf("over\n");
			break;		
		}
		no ++;
		if(strcmp(client_ip, "server ip")==0){
			server_has_file = 1;			
		} else{
			printf("%s push into\n", client_ip);
			struct p2p_info info;
			sprintf(info.filename, "%s", filename);
			sprintf(info.ip, "%s", client_ip);
			info.total = file_num;
			info.no = no;
			printf("iter client from %s\n", client_ip);
			pthread_create(&tid[thread_num], NULL, p2p_connect, (void *)&info);
			//pthread_join(tid[thread_num], NULL);
			
			thread_num++;

			sprintf(temp.ip, "%s", client_ip);
			ip_addr.push_back(temp.ip);
		}
		
		
		//file_num++;
		//printf("ip: %s\n", client_ip);
		//j++;
		//recv(sockfd, &file_num, sizeof(file_num), 0);
	}

	for(int k=0; k<thread_num; k++){
		pthread_join(tid[k], NULL);
	}
	

	//send(sockfd, "start", sizeof("start"), 0);
	//memset(&buffer, 0, sizeof(buffer));
	//int n1 = recv(sockfd, buffer, sizeof(buffer), 0);
	//printf("buffer is %d\n", n1);

	printf("file number is %d, no is %d\n", file_num, no);


	// connect to other client(p2p download)
/*
	int no = 0;
	list<char*>::iterator iter;
	for(iter=ip_addr.begin(); iter!=ip_addr.end(); iter++){
		struct p2p_info info;
		sprintf(info.filename, "%s", filename);
		sprintf(info.ip, "%s", *iter);
		info.total = file_num;
		info.no = no;
		printf("iter client from %s\n", *iter);
		pthread_create(&tid, NULL, p2p_connect, (void *)&info);
		//p2p_connect(*iter, filename, file_num, no);
		no++;
	}*/

	printf("%d has this file, does server has file? %d\n", file_num, server_has_file);

	printf("download: %s\n", filename);
	
	//chdir("file");
	if(server_has_file==1){
		
		fp1 = fopen("1", "wb");
		int i = 0;
		memset(&recvline, 0, sizeof(recvline));

		// start downloading file		
		while( (nCount = recv(sockfd, recvline, BUF_SIZE, 0)) > 0){
			//printf("start recv\n");

			fwrite(recvline, sizeof(char), nCount, fp1); 

			if(nCount!=BUF_SIZE){
				printf("----------------\n");
				printf("%d recv success\n", i);
				printf("nCount is %d, i = %d\n", nCount, i);
				printf("----------------\n\n");
				//printf("%s\n", recvline);
			}
			send(sockfd, &nCount, sizeof(nCount), 0);
			memset(&recvline, 0, sizeof(recvline));
		
			if(nCount < BUF_SIZE) break;
			i ++;
		}	
		//printf("Download complete!\n");
		fclose(fp1);
	}



	printf("thread num %d\n", thread_num);
	//for(int k=0; k<thread_num; k++){
	//	pthread_join(tid[k], NULL);
	//}

	return file_num;
	

	

	
	
}

int upload(int sockfd, char* filename)
{
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
	
	while( (nCount = fread(file_buffer, sizeof(char), BUF_SIZE,  fp)) > 0){
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

void file_exist(int sockfd)
{
	FILE *fp1;
	char noFile[] = "there is no file or directory\n";
	char buffer[1024];
	int nCount;
	int hasFile = 0;
	int size;
	char ack[12] = "yet";
	char *pch;
	int reply;
	chdir("file");


	memset(buffer, '\0', sizeof(buffer));
	fp1 = popen("ls", "r");
	//while( (nCount = fread(buffer, sizeof(char), BUF_SIZE,  fp1)) >0){
		fread(buffer, sizeof(char), BUF_SIZE,  fp1);
		hasFile = 1;
		struct stat st;
		printf("%s", buffer);
		pch = strtok(buffer, "\n");

		while(pch!=NULL){
			//send(sockfd, ack, strlen(ack), 0);
			//memset(&size, 0, sizeof(size));
			
			stat(pch, &st);
			size = st.st_size;
			send(sockfd, pch, strlen(pch), 0);
			recv(sockfd, &reply, sizeof(reply), 0);	
			send(sockfd, &size, sizeof(size), 0);

			printf("%s, size: %d\n", pch, size);
			//memset(pch, 0, sizeof(pch));
			pch = strtok(NULL, "\n");
		}
		char over[12] = "endfile";
		send(sockfd, over, strlen(over), 0);
		
	//}

	if(hasFile == 0){
		send(sockfd, noFile, sizeof(noFile), 0);
	}
	fclose(fp1);
	chdir("..");

}

void show(int sockfd){
	// show all clients and information
	char info[1024], buffer[1024];
	memset(&info, 0, sizeof(info));
	read(sockfd, info, 1024);
	printf("%s\n", info);

}

void recompose(int file_num, char* filename){
	int n;
	char file_buffer2[1024], file[1024];
	char file_no[2] ;
	sprintf(file, "%s", filename);
	//chdir("file");

	FILE *fp2;
	FILE *fp3;
	fp2 = fopen(file, "wb");

	printf("recompose file %s\n", file);
	for(int k=1; k<=file_num; k++){
		
		sprintf(file_no, "%d", k);
		printf("file name %s\n", file_no);
		fp3 = fopen(file_no, "rb");
		while( (n = fread(file_buffer2, sizeof(char), BUF_SIZE,  fp3)) > 0){
			//printf("%s", file_buffer2);
			fwrite(file_buffer2, sizeof(char), n, fp2);
			//memset(&file_buffer2, 0, sizeof(file_buffer2));
		
		}
		fclose(fp3);
		char rm[128];
		sprintf(rm, "rm ");
		strcat(rm, file_no);
		system(rm);
	}
	fclose(fp2);
	//chdir("..");
}

void cmd(int sockfd);



void *connect_server(void *arg){

	printf("tread start\n");
	int sockfd;
	struct address *addr = (struct address *)arg;
	struct sockaddr_in addr_svr;

	memset(&addr_svr, 0, sizeof(addr_svr));
	addr_svr.sin_family = AF_INET;
	addr_svr.sin_port = htons(addr->port);
	addr_svr.sin_addr.s_addr = inet_addr(addr->ip);



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
	char id[1024];
	//fgets(buffer, sizeof(buffer), stdin);
	printf("please enter id: ");
	fgets(id, 1024, stdin);
	if(id[strlen(id)-1]=='\n')
		id[strlen(id)-1]='\0';
	write(sockfd, id, strlen(id));

	file_exist(sockfd);
	cmd(sockfd);

	pthread_exit(NULL);

}

static void *wait_for_connect(void *arg){
	struct address *addr = (struct address *)arg;

	int connfd;
	connfd = addr->sockfd;
	char id[1024];



	//pthread_detach(pthread_self());

	p2p_upload(connfd);
	close(connfd);

	pthread_exit(NULL);
	return 0;
}



int main(int argc, char **argv)
{
	struct sockaddr_in addr_svr;
	int sockfd, *iptr;
	pthread_t tid, tid2;
	uint16_t port = atoi(argv[2]);
	struct address addr;
	void * thread_result;

	//connection test;
	//test.initialize();

	addr.port = port;
	sprintf(addr.ip, "%s", argv[1]);
	
	//memset(&addr_svr, 0, sizeof(addr_svr));
	//addr_svr.sin_family = AF_INET;
	//addr_svr.sin_port = htons(port);
	//addr_svr.sin_addr.s_addr = inet_addr(argv[1]);

	pthread_create(&tid,NULL, connect_server, (void *)&addr);


	// TCP server
	int chat_port = 9999;
	int tcpfd;

	tcpfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&addr_svr, 0, sizeof(addr_svr));
	
	addr_svr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_svr.sin_family = AF_INET;
	addr_svr.sin_port = htons(chat_port);

	int on = 1;
	setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	bind(tcpfd, (struct sockaddr *)&addr_svr, sizeof(addr_svr));

	listen(tcpfd, 10);

	struct sockaddr_in addr_cln;
	socklen_t sLen = sizeof(addr_cln);

	while(1){
		printf("waiting for connect\n");
		struct address addr;
		int connfd;


		connfd = accept(tcpfd, (struct sockaddr *)&addr_cln, &sLen);
		printf("connection from %s, port %d\n", inet_ntoa(addr_cln.sin_addr), ntohs(addr_cln.sin_port));
		
		// store user info
		addr.port = ntohs(addr_cln.sin_port);
		sprintf(addr.ip, "%s", inet_ntoa(addr_cln.sin_addr)); 
		addr.sockfd = connfd;

		if(connfd == -1){
			//cerr << "ERROR: accept()" << endl;
			exit(1);
		}
		
		pthread_create(&tid2, NULL, &wait_for_connect, (void*)&addr);
		
	}




	pthread_join(tid, &thread_result);
	pthread_join(tid2, &thread_result);

	printf("here\n");
	

	return 0;
}

void cmd(int sockfd){

	char cmd[1024], rcv[1024], dir[1024];
	while(1){
		printf("----start operation----\n");
		memset(&cmd, 0, sizeof(cmd));
		memset(&rcv, 0, sizeof(rcv));
		memset(&dir, 0, sizeof(dir));
		
		fgets(cmd, 1024, stdin);
		printf("cmd is %s\n", cmd);
		write(sockfd, cmd, strlen(cmd));
		
		
		// download cmd
		if(strcmp(cmd, "download\n") == 0){
			printf("start downloading\n");
			printf("please enter the file name: ");	
			chdir("file");
			int file_num;
			// get file name
			char sendline[1024];
			fgets(sendline, 1024, stdin);
			if(sendline[strlen(sendline)-1] == '\n')
			sendline[strlen(sendline)-1] ='\0';

			write(sockfd, sendline, strlen(sendline));
			
			file_num = download(sockfd, sendline);

			recompose(file_num, sendline);
			int size;
			
			struct stat st;
			stat(sendline, &st);
			size = st.st_size;
			send(sockfd, &size, sizeof(size), 0);

			printf("download complete\n");
			chdir("..");


			continue;

		// upload cmd
		} else if(strcmp(cmd, "upload\n") == 0){
			printf("start uploading\n");

			// get filename
			char filename[1024];
			printf("please enter the file name: ");
			fgets(filename, 1024, stdin);
			if(filename[strlen(filename)-1] == '\n')
			filename[strlen(filename)-1] ='\0';

			write(sockfd, filename, strlen(filename)+1);
			
			upload(sockfd, filename);
			printf("upload complete\n");
			continue;

		// ls cmd
		} else if(strcmp(cmd, "ls\n") == 0){
			read(sockfd, rcv, 1024);
			printf("%s", rcv);
			continue;
		
		}else if(strcmp(cmd, "show\n") == 0){
			show(sockfd);

		}else if(strcmp(cmd, "rm\n")==0){
			chdir("file");
			char filename[1024];
			fgets(filename, 1024, stdin);
			if(filename[strlen(filename)-1] == '\n')
			filename[strlen(filename)-1] ='\0';
			write(sockfd, filename, strlen(filename)+1);

			char rm[128];
			sprintf(rm, "rm ");
			strcat(rm, filename);
			
			system(rm);

			chdir("..");
			continue;
		} else if(strcmp(cmd, "over\n") ==0 ){
			printf("over\n");
			break;
	
		// wrong cmd
		} else
			printf("wrong cmd\n");
	}	
}
	
