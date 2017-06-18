/* homework3 server*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <list>
using std::list;

#define BUF_SIZE 1024

struct file_info{
	char filename[64];
	int size;
};

struct user{
	char id[1024];
	int port;
	char ip[1024];
	int no;
	list<file_info> file;
};

struct address{
	int port;
	char ip[1024];
	int sockfd;
	int no;
};

struct p2p_info{
	char filename[128];
	char ip[128];
	int total;
	int no;
};



list<struct user> userfile;

void renew_file_list(char*ip, char* filename, int size){
	struct file_info new_file;
	sprintf(new_file.filename, "%s", filename);
	new_file.size = size;

	list<struct user>::iterator iter;

	if(size==0){
		for(iter=userfile.begin(); iter!=userfile.end(); iter++){
			if(strcmp(iter->ip, ip)==0){
				list<struct file_info>::iterator iter2;
					for(iter2=iter->file.begin(); iter2!=iter->file.end(); iter2++){
						if(strcmp(iter2->filename, filename)==0){
							iter->file.erase(iter2++);
						}
					}
				
			}
		}
	} else{
		for(iter=userfile.begin(); iter!=userfile.end(); iter++){
			if(strcmp(iter->ip, ip)==0){
				iter->file.push_back(new_file);
			}
		}
	}
	
}


int cli_download(int sockfd, char* filename)
{
	ssize_t n;
	char file_buffer[1024], file_name[1024], buffer[1024];
	char over[] = "over";
	int file_num = 0;
	int zero = 0;
	int server_has_file = 0;
	int size;

	list<struct user>::iterator iter;
	for(iter=userfile.begin(); iter!=userfile.end(); iter++){
		//printf("%s, from %s, port %d\n", iter->id, iter->ip, iter->port);
		list<struct file_info>::iterator iter2;
		for(iter2=iter->file.begin();iter2!=iter->file.end(); iter2++){
			if(strcmp(filename, iter2->filename)==0){
				if(strcmp(iter->ip, "server ip")==0){
					server_has_file = 1;
					size = iter2->size;
				}
				file_num++;
				//send(sockfd, &zero, sizeof(zero), 0);
			}
			//printf("%s, size: %d bytes\n", iter2->filename, iter2->size);
		}
		printf("\n");
	}

	printf("file number: %d\n", file_num);
	send(sockfd, &file_num, sizeof(file_num), 0);





	printf("-------------\n");
	//list<struct user>::iterator iter;
	for(iter=userfile.begin(); iter!=userfile.end(); iter++){
		//printf("%s, from %s, port %d\n", iter->id, iter->ip, iter->port);
		list<struct file_info>::iterator iter2;
		for(iter2=iter->file.begin();iter2!=iter->file.end(); iter2++){
			if(strcmp(filename, iter2->filename)==0){
				printf("%s, from %s, port %d, has ", iter->id, iter->ip, iter->port);
				printf("%s, size: %d bytes\n", iter2->filename, iter2->size);
				send(sockfd, iter->ip, sizeof(iter->ip), 0);
				recv(sockfd, buffer, sizeof(buffer), 0);
				//file_num++;
				//send(sockfd, &zero, sizeof(zero), 0);
			}
			//printf("%s, size: %d bytes\n", iter2->filename, iter2->size);
		}
		printf("\n");
	}
	printf("-------------\n");
	//char over[1024] = "over";
	int n1 = send(sockfd, "over", sizeof("over"), 0);
	recv(sockfd, buffer, sizeof(buffer), 0);
	//printf("over: %d\n", n1);

	printf("does server has file? %d\n", server_has_file);
	
	
	memset(&file_buffer, 0, sizeof(file_buffer));
	FILE *fp;


	
	//chdir("file");

	if(server_has_file==1){
		fp = fopen(filename, "rb");



	
		printf("download: %s\n", filename);
		int nCount;	
		int i = 0;
		int num = 0;
		int recv_num;

		int buf_size = 1024;
		int part = size/file_num;
		int send_byte = 0;

		if(buf_size>part) buf_size = part;

		send_byte = send_byte + buf_size;

	
		while( (nCount = fread(file_buffer, sizeof(char), buf_size,  fp)) >0){
			//printf("start send\n");
			if(nCount!=BUF_SIZE){
				printf("----------------\n");	
				//num = send(sockfd, file_buffer, nCount, 0);
				printf("nCount is %d, i = %d, success: %d\n", nCount, i, num);

				printf("%d recv success\n", i);
				printf("----------------\n\n");
			}
			num = send(sockfd, file_buffer, nCount, 0);
			//printf("nCount is %d, i = %d, success: %d\n", nCount, i, num);
			memset(&file_buffer, 0, sizeof(file_buffer));
			recv(sockfd, &recv_num, sizeof(recv_num), 0);
			i ++;
			send_byte = send_byte + buf_size;
			if(send_byte>part)buf_size = part - (send_byte-buf_size);
		}	
		printf("writing complete\n");
		fclose(fp);
	}
	
	//chdir("..");
	
	return 0;

}

void p2p_download(int sockfd, char *filename, int total, int no){
	FILE *fp1;
	char temp_file[2];
	char sendline[1024], recvline[2048], buffer[1024];
	int nCount;
	sprintf(temp_file, "%d", no);

	printf("start p2p download, filename %s\n", filename);
	send(sockfd, &total, sizeof(total), 0);
	send(sockfd, &no, sizeof(no), 0);
	

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
	pthread_detach(pthread_self());
	struct sockaddr_in addr_svr;
	int sockfd;
	uint16_t port = 9999;
	struct p2p_info *info = (struct p2p_info *)arg;
	char ip_num[1024];
	strcpy(ip_num, info->ip);

	printf("filename: %s, p2p ip %s\n", info->filename, ip_num);
	
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

	write(sockfd, info->filename, strlen(info->filename)+1);
	int total = info->total;
	int no = info->no;
	char filename[1024];
	sprintf(filename, "%s", info->filename);
	printf("p2p filename %s\n", filename);

	p2p_download(sockfd, filename, total, no);

	pthread_exit(NULL);

	//pthread_create(&tid,NULL, connect_server, (void *)&addr);

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
		char rm[128] = "rm ";
		strcat(rm, file_no);
		system(rm);
	}
	fclose(fp2);
	//chdir("..");
}


int cli_upload(int sockfd, char *filename)
{
	char recvline[1024];
	char ready[] = "ok";
	char start[1024];
	FILE *fp1;
	memset(&recvline, 0, sizeof(recvline));
	int nCount;
	printf("start downloading: %s\n", filename);
	int file_num = 0;
	int server_has_file = 0;
	pthread_t tid[32];
	int thread_num = 0;
	struct p2p_info info[32];

	//sprintf(info.filename, "%s", filename);


	int size;

	list<struct user>::iterator iter;
	for(iter=userfile.begin(); iter!=userfile.end(); iter++){
		//printf("%s, from %s, port %d\n", iter->id, iter->ip, iter->port);
		list<struct file_info>::iterator iter2;
		for(iter2=iter->file.begin();iter2!=iter->file.end(); iter2++){
			if(strcmp(filename, iter2->filename)==0){
				if(strcmp(iter->ip, "server ip")==0){
					server_has_file = 1;
				}
				size = iter2->size;
				file_num++;
				//send(sockfd, &zero, sizeof(zero), 0);
			}
			//printf("%s, size: %d bytes\n", iter2->filename, iter2->size);
		}
		printf("\n");
	}

	//info.total = file_num;
	int no = 0;

	for(iter=userfile.begin(); iter!=userfile.end(); iter++){
		//printf("%s, from %s, port %d\n", iter->id, iter->ip, iter->port);
		list<struct file_info>::iterator iter2;
		for(iter2=iter->file.begin();iter2!=iter->file.end(); iter2++){
			if(strcmp(filename, iter2->filename)==0){
				no++;
				printf("%s, from %s, port %d, has ", iter->id, iter->ip, iter->port);
				printf("%s, size: %d bytes\n", iter2->filename, iter2->size);
				sprintf(info[thread_num].ip, "%s", iter->ip);
				sprintf(info[thread_num].filename, "%s", filename);
				info[thread_num].total = file_num;
				info[thread_num].no = no;
				pthread_create(&tid[thread_num], NULL, p2p_connect, (void *)&info[thread_num]);
				
				thread_num++;
				
			}
			//printf("%s, size: %d bytes\n", iter2->filename, iter2->size);
		}
		printf("\n");
	}

	printf("thread number: %d\n", thread_num);
	for(int k=0; k<thread_num; k++){
		pthread_join(tid[k], NULL);
	}
	//pthread_join(tid[0], NULL);
	//pthread_join(tid[1], NULL);


	return file_num;
	//recompose(file_num, filename);
	/*
	//chdir("file");
	fp1 = fopen(filename, "wb");	
	int i = 0;	
	int num = 0;
	int reply;

	//recv(sockfd, start, 1024, 0);
	while( (nCount = recv(sockfd, recvline, BUF_SIZE, 0)) > 0){
	
		fwrite(recvline, sizeof(char), nCount, fp1); 
		if(nCount!=1024){
			printf("----------------\n");
			printf("%d recv success\n", i);
			printf("nCount is %d, i = %d, success: %d\n", nCount, i, num);
			printf("----------------\n\n");
		}
		send(sockfd, &nCount, sizeof(nCount), 0);
		if(nCount < BUF_SIZE && i!=0) break;
		i ++;
	}
	printf("Download complete!\n");
	fclose(fp1);
	//chdir("..");*/
	
	
}

void ls(int sockfd)
{
	FILE *fp1;
	char noFile[] = "there is no file or directory\n";
	char buffer[2048];
	int nCount;
	int hasFile = 0;
	chdir("file");
	memset(buffer, '\0', sizeof(buffer));
	fp1 = popen("ls", "r");
	while( (nCount = fread(buffer, sizeof(char), BUF_SIZE,  fp1)) >0){
		hasFile = 1;
		send(sockfd, buffer, nCount, 0);
	}
	if(hasFile == 0){
		send(sockfd, noFile, sizeof(noFile), 0);
	}
	fclose(fp1);
	chdir("..");

}

void show(int connfd){
	char info[1024], buffer[1024];
	memset(&info, 0, strlen(info));
	memset(&buffer, 0, strlen(buffer));

	list<struct user>::iterator iter;
	for(iter=userfile.begin(); iter!=userfile.end(); iter++){
		memset(&buffer, 0, strlen(buffer));
		strcat(info, "-------------\n");
		sprintf(buffer, "%s, from %s, port %d\n", iter->id, iter->ip, iter->port);
		strcat(info, buffer);
		list<struct file_info>::iterator iter2;
		for(iter2=iter->file.begin();iter2!=iter->file.end(); iter2++){
			memset(&buffer, 0, strlen(buffer));
			sprintf(buffer, "%s, size: %d bytes\n", iter2->filename, iter2->size);
			strcat(info, buffer);
		}
		strcat(info, "-------------\n");
		printf("-------------\n");
	}

	send(connfd, info, strlen(info), 0);
}

void file_exist()
{
	FILE *fp1;
	char noFile[] = "there is no file or directory\n";
	char buffer[1024];
	int nCount;
	int hasFile = 0;
	int size;
	char *pch;
	struct user server_file;
	struct file_info server_file_info;
	chdir("file");

	sprintf(server_file.id, "sever");
	sprintf(server_file.ip, "server ip");



	memset(buffer, '\0', sizeof(buffer));
	fp1 = popen("ls", "r");
	
	fread(buffer, sizeof(char), BUF_SIZE,  fp1);
	hasFile = 1;
	struct stat st;
	printf("%s", buffer);
	pch = strtok(buffer, "\n");

	while(pch!=NULL){
		sprintf(server_file_info.filename, "%s", pch);	
		stat(pch, &st);
		size = st.st_size;
		server_file_info.size = size;
		server_file.file.push_back(server_file_info);

		printf("%s, size: %d\n", pch, size);
		//memset(pch, 0, sizeof(pch));
		pch = strtok(NULL, "\n");
	}

	userfile.push_back(server_file);


	if(hasFile == 0){
		//send(sockfd, noFile, sizeof(noFile), 0);
	}
	fclose(fp1);
	chdir("..");

}



void cmd(int connfd, int no, char* ip){
	char filename[1024], cmd[1024], dir[1024];
	memset(&dir, 0, sizeof(dir));

	printf("start reading cmd\n");
	while(1){
		memset(&filename, 0, sizeof(filename));
		//printf("start reading cmd\n");
		memset(&cmd, 0, sizeof(cmd));
		read(connfd, cmd, 1024);
		printf("cmd is %s\n", cmd);

		// download cmd
		if(strcmp(cmd, "download\n") == 0){
			
			read(connfd, filename, 1024);
			printf("file name is %s\n", filename);
				
			chdir("file");	
			cli_download(connfd, filename);
			printf("download complete\n\n");
			printf("over\n");
			chdir("..");
			int size;
			read(connfd, &size, sizeof(size));

			renew_file_list(ip, filename, size);

			continue;

		// upload cmd
		} else if(strcmp(cmd, "upload\n") == 0){
			read(connfd, filename, 1024);		
			printf("file name is %s\n", filename);
				
			chdir("file");
			int file_num = cli_upload(connfd, filename);
			printf("upload complete\n\n");
			recompose(file_num, filename);
			

			int size;
			struct stat st;
			char temp_ip[16];
			stat(filename, &st);
			size = st.st_size;
			//printf("size = %d\n", size);
			strcpy(temp_ip, "server ip");
			renew_file_list(temp_ip, filename, size);
			chdir("..");
			continue;

		// ls cmd
		} else if(strcmp(cmd, "ls\n") == 0){
			printf("cmd ls\n");
			ls(connfd);
			continue;

		// show cmd
		} else if(strcmp(cmd, "show\n") == 0){
			show(connfd);
			continue; 

		// remove cmd
		}else if(strcmp(cmd, "rm\n")==0){
			read(connfd, filename, 1024);
			int size = 0;


			renew_file_list(ip, filename, size);
			continue; 

		// over connect
		} else if(strcmp(cmd, "over\n") ==0 ){
			printf("over\n\n");
			//printf("exit connection from %s, port %d\n", inet_ntoa(addr_cln.sin_addr), ntohs(addr_cln.sin_port));
			
			list<struct user>::iterator iter;
			for(iter=userfile.begin(); iter!=userfile.end();iter++){
				if(iter->no==no){
					userfile.erase(iter++);
				}

			}
			
			break;

		// wrong cmd
		} else{
			//printf("wrong cmd\n");
		}
	}

}

void file_record(int connfd, char *id, char *ip, int port, int no){
	
	struct user userfile_temp;
	struct file_info file;
	char filename[1024];
	int size;
	char ack[12];
	int reply = 1;

	userfile_temp.port = port;
	userfile_temp.no = no;
	sprintf(userfile_temp.ip, "%s", ip);
	sprintf(userfile_temp.id, "%s", id);

	//memset(&con, 0, sizeof(con));
	//read(connfd, &con, sizeof(con));
	while(1){
		//read(connfd, ack, strlen(ack));
		//if(strcmp(ack, "end\n")==0) break;

		memset(&filename, 0, sizeof(filename));
		memset(&size, 0 ,sizeof(size));
		printf("-------\n");
		read(connfd, filename, 1024);
		if(strcmp(filename, "endfile")==0) break;

		write(connfd, &reply, sizeof(reply));
		printf("-------\n");
		printf("%s\n", filename);
		//printf("-------\n");

		read(connfd, &size, sizeof(size));
		//printf("-------\n");
		sprintf(file.filename, "%s", filename);
		file.size = size;

		printf("size: %d\n", file.size);
		userfile_temp.file.push_back(file);
		memset(filename, 0, sizeof(filename));
		memset(&size, 0, sizeof(size));

	}

	printf("---------------\n");
	printf("%s, from %s, port %d\n", userfile_temp.id, userfile_temp.ip, 
		userfile_temp.port);
	list<file_info>::iterator iter;
	for(iter=userfile_temp.file.begin(); iter!=userfile_temp.file.end(); iter++){
		printf("%s, size: %d\n", iter->filename, iter->size);
	}
	printf("---------------\n");

	userfile.push_back(userfile_temp);


}


static void *client(void *arg){
	pthread_detach(pthread_self());
	struct address *addr = (struct address *)arg;

	int connfd;
	connfd = addr->sockfd;
	char id[1024];
	struct user userfile_temp;

	


	pthread_detach(pthread_self());
	memset(&id, 0, sizeof(id) );
	recv(connfd, id, sizeof(id), 0);
	printf("user id: %s\n", id);

	userfile_temp.port = addr->port;
	userfile_temp.no = addr->no;
	sprintf(userfile_temp.ip, "%s", addr->ip);
	sprintf(userfile_temp.id, "%s", id);

	file_record(connfd, id, addr->ip, addr->port, addr->no);

	cmd(connfd, userfile_temp.no, userfile_temp.ip);
	close(connfd);

	pthread_exit(NULL);
}


#define default_port 9877

int main(int argc, char **argv)
{

	struct sockaddr_in addr_svr;
	int sockfd, *iptr;
	pid_t childpid;
	uint16_t port = default_port;

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
		
	struct sockaddr_in addr_cln;
	socklen_t sLen = sizeof(addr_cln);
	int i=0;

	file_exist();
	pthread_t tid[32];
	int client_num = 0;
	
	while(1){
		printf("waiting for connect\n");
		struct user userfile_temp;
		struct address addr;
		int connfd;


		connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
		printf("connection from %s, port %d\n", inet_ntoa(addr_cln.sin_addr), ntohs(addr_cln.sin_port));
		
		// store user info
		addr.port = ntohs(addr_cln.sin_port);
		sprintf(addr.ip, "%s", inet_ntoa(addr_cln.sin_addr)); 
		addr.sockfd = connfd;
		addr.no = i;

		if(connfd == -1){
			//cerr << "ERROR: accept()" << endl;
			exit(1);
		}
		
		
		pthread_create(&tid[client_num], NULL, &client, (void*)&addr);
		i++;
		client_num++;
		
	}

	for(int k=0;k<client_num; k++){
		pthread_join(tid[k], NULL);
	}

	return 0;
}

