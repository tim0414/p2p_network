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
#include "upload.h"
#include "p2p_download.h"

#define BUFFER_SIZE 1024

void cmd(int sockfd);




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

char server_ip[128];
int server_port;

static void *p2p_connect(void *arg){
	pthread_detach(pthread_self());
    struct sockaddr_in addr_svr;
	int sockfd;
	uint16_t port = 9999;
	struct p2p_info *info = (struct p2p_info *)arg;
	char ip_num[1024];
	strcpy(ip_num, info->ip);
	
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

    p2p_download p2p_dl;
	p2p_dl.download(sockfd, filename, total, no);

	pthread_exit(NULL);

	//pthread_create(&tid,NULL, connect_server, (void *)&addr);

}

int download(int sockfd, char *filename){
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
			pthread_join(tid[thread_num], NULL);
			
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
	

	printf("file number is %d, no is %d\n", file_num, no);



	printf("%d has this file, does server has file? %d\n", file_num, server_has_file);

	printf("download: %s\n", filename);
	
	//chdir("file");
	if(server_has_file==1){
		
		fp1 = fopen("1", "wb");
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
	}



	printf("thread num %d\n", thread_num);
	for(int k=0; k<thread_num; k++){
		pthread_join(tid[k], NULL);
	}

	return file_num;

}

void file_exist(int sockfd){
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
		fread(buffer, sizeof(char), BUFFER_SIZE,  fp1);
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

static void *connect_server(void *arg){
    pthread_detach(pthread_self());
    
    int sockfd;
    struct address *addr = (struct address *)arg;
    
    connection client_connect;
	client_connect.port = addr->port;
	client_connect.ip = addr->ip;

    printf("ip: %s\n", client_connect.ip);
	
    client_connect.client_init();
    sockfd = client_connect.connect_to_server();
    client_connect.inputID();

    file_exist(sockfd);
    cmd(sockfd);

    pthread_exit(NULL);
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
		while( (n = fread(file_buffer2, sizeof(char), BUFFER_SIZE,  fp3)) > 0){
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

static void *wait_for_connect(void *arg){
    pthread_detach(pthread_self());

	struct address *addr = (struct address *)arg;
	int connfd;
	connfd = addr->sockfd;
	char id[1024];

	//p2p_upload(connfd);
	upload up2client;
    up2client.p2p_upload(connfd);
    
    close(connfd);

	pthread_exit(NULL);
}

static void *send_download_cmd(void *arg){
    pthread_detach(pthread_self());

	struct address *addr = (struct address *)arg;
	int connfd, file_num;
	connfd = addr->sockfd;
    char filename[64];

    recv(connfd, filename, strlen(filename), 0);
    printf("before call download fileanem: %s\n", filename);

    p2p_download dl;
    sprintf(filename, "%s", "cover.jpeg");
    dl.normal_download(connfd, filename);

/*
    connection download_file;
    download_file.ip = server_ip;
    download_file.port = server_port;
    download_file.client_init();
    int sockfd = download_file.connect_to_server();
    
    send(sockfd, "test", strlen("test")+1, 0);
    char over[12] = "endfile";
    int size = 0;
	send(sockfd, over, strlen(over), 0);
    send(sockfd, &size, sizeof(size), 0);
    //file_exist(sockfd);
    printf("start command\n");
    char cmd[32];
    sprintf(cmd, "%s", "download\n");

    chdir("file");
    send(sockfd, cmd, strlen(cmd)+1, 0);
    
    send(sockfd, filename, strlen(filename)+1, 0);

    file_num = download(sockfd, filename);
	recompose(file_num, filename);
    chdir("..");*/
}


static void *upload_to_client(void *arg){
    pthread_detach(pthread_self());

    int sockfd;
    pthread_t tid;

    connection wait_to_up;
    wait_to_up.port = 9999;
    sockfd = wait_to_up.server_init();

    struct sockaddr_in addr_cln;
	socklen_t sLen = sizeof(addr_cln);
    while(1){
		printf("waiting for connect\n");
		struct address addr;
		int connfd;

		connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
		printf("connection from %s, port %d\n", inet_ntoa(addr_cln.sin_addr), ntohs(addr_cln.sin_port));
		
		// store user info
		addr.port = ntohs(addr_cln.sin_port);
		sprintf(addr.ip, "%s", inet_ntoa(addr_cln.sin_addr)); 
		addr.sockfd = connfd;

		if(connfd == -1){
			//cerr << "ERROR: accept()" << endl;
			exit(1);
		}
		
		pthread_create(&tid, NULL, &wait_for_connect, (void*)&addr);
		
	}

    pthread_exit(NULL);
}

static void *download_from_client(void *arg){
    pthread_detach(pthread_self());

    int sockfd;
    pthread_t tid;
    connection wait_for_up;
    wait_for_up.port = 5200;
    sockfd = wait_for_up.server_init();

    struct sockaddr_in addr_cln;
	socklen_t sLen = sizeof(addr_cln);
    while(1){
		printf("waiting for connect\n");
		struct address addr;
		int connfd;

		connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
		printf("connection from %s, port %d\n", inet_ntoa(addr_cln.sin_addr), ntohs(addr_cln.sin_port));
		
		// store user info
		addr.port = ntohs(addr_cln.sin_port);
		sprintf(addr.ip, "%s", inet_ntoa(addr_cln.sin_addr)); 
		addr.sockfd = connfd;

		if(connfd == -1){
			//cerr << "ERROR: accept()" << endl;
			exit(1);
		}
		
		pthread_create(&tid, NULL, &send_download_cmd, (void*)&addr);
		
	}

    pthread_exit(NULL);
}


int main(int argc, char **argv){
    pthread_t tid1, tid2, tid3;
    struct address addr;
    
    // get ip address and port, connect to server //
    addr.port = atoi(argv[2]);
    server_port = atoi(argv[2]);
	sprintf(addr.ip, "%s", argv[1]);
    sprintf(server_ip, "%s", argv[1]);
    pthread_create(&tid1, NULL, connect_server, (void *)&addr);

    // a server wait for other client upload file to it //
    pthread_create(&tid2, NULL, download_from_client, NULL);

    // a server wait to upload some file to other clients //
    pthread_create(&tid3, NULL, upload_to_client, (void *)&addr);


	pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);
	

    return 0;
}

void show(int sockfd){
	// show all clients and information
	char info[1024], buffer[1024];
	memset(&info, 0, sizeof(info));
	read(sockfd, info, 1024);
	printf("%s\n", info);

}

void cmd(int sockfd){
    
    char cmd[64], rcv[256], dir[64];
	while(1){
		printf("----start operation----\n");
		memset(&cmd, 0, sizeof(cmd));
		memset(&rcv, 0, sizeof(rcv));
		memset(&dir, 0, sizeof(dir));
		
		fgets(cmd, 64, stdin);
		printf("cmd is %s\n", cmd);
		send(sockfd, cmd, strlen(cmd)+1, 0);
		
		
		// download cmd
		if(strcmp(cmd, "download\n") == 0){
			printf("start downloading\n");
			printf("please enter the file name: ");	
			chdir("file");
			int file_num;

			// get file name
			char filename[64];
			fgets(filename, 64, stdin);

			if(filename[strlen(filename)-1] == '\n')
			    filename[strlen(filename)-1] ='\0';

			send(sockfd, filename, strlen(filename)+1, 0);
            file_num = download(sockfd, filename);
			recompose(file_num, filename);
			
            int size;
			struct stat st;
			stat(filename, &st);
			size = st.st_size;
			send(sockfd, &size, sizeof(size), 0);

			printf("download complete\n");
			chdir("..");


			continue;

		// upload cmd
		} else if(strcmp(cmd, "upload\n") == 0){
			printf("start uploading\n");

			// get filename
			char filename[64];
			printf("please enter the file name: ");
			fgets(filename, 64, stdin);
			if(filename[strlen(filename)-1] == '\n')
			    filename[strlen(filename)-1] ='\0';

			send(sockfd, filename, strlen(filename)+1, 0);
			
			upload upcmd;
            upcmd.upload_file(sockfd, filename);
			printf("upload complete\n");
			continue;

        //upload to client
        }else if(strcmp(cmd, "upload to client\n")==0){
            char ip[64];
            char filename[64];
            int file_num;

            memset(&filename, 0, sizeof(filename));
            memset(&ip, 0, sizeof(ip));

            printf("please enter client ip: ");
            fgets(ip, 64, stdin);
			if(ip[strlen(ip)-1] == '\n')
			    ip[strlen(ip)-1] ='\0';

            printf("please enter the file name: ");
			fgets(filename, 64, stdin);
			if(filename[strlen(filename)-1] == '\n')
			    filename[strlen(filename)-1] ='\0';

			//send(sockfd, filename, strlen(filename)+1, 0);

            //recv(sockfd, &file_num, sizeof(file_num), 0);    

            connection up2client;
            up2client.ip = ip;
            up2client.port = 5200;    
            
            up2client.client_init();
            int upsock = up2client.connect_to_server();
            printf("filename :%s\n", filename);
            send(upsock, filename, strlen(filename)+1, 0);
            upload upcmd;
            upcmd.upload_file(upsock, filename);
			printf("upload complete\n");
            //send(upsock, filename, strlen(filename)+1, 0);

		// ls cmd
		} else if(strcmp(cmd, "ls\n") == 0){
			read(sockfd, rcv, 256);
			printf("%s", rcv);
			continue;
		
        //show online user information
		}else if(strcmp(cmd, "show\n") == 0){
			show(sockfd);

        //remove file
		}else if(strcmp(cmd, "rm\n")==0){
			chdir("file");
			char filename[64];
			fgets(filename, 64, stdin);
			if(filename[strlen(filename)-1] == '\n')
			    filename[strlen(filename)-1] ='\0';
			write(sockfd, filename, strlen(filename)+1);

			char rm[128];
			sprintf(rm, "rm ");
			strcat(rm, filename);
			
			system(rm);

			chdir("..");
			continue;

        //over connect    
		} else if(strcmp(cmd, "over\n") ==0 ){
			printf("over\n");
			break;
	
		// wrong cmd
		} else
			printf("wrong cmd\n");
	}	
}
