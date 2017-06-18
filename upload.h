#ifndef UPLOAD_H
#define UPLOAD_H

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

class upload{
    public:
        void p2p_upload(int sockfd);
        void upload_file(int sockfd, char* filename);

};


#endif