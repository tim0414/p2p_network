#ifndef P2P_DOWNLOAD_H
#define P2P_DOWNLOAD_H

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



class p2p_download{
    public:
        void download(int sockfd, char *filename, int total, int no);
        void normal_download(int sockfd, char *filename);

};


#endif