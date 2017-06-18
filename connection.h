#ifndef CONNECTION_H
#define CONNECTION_H

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

class connection{
    public:
        uint16_t port;
        char *ip;
        int sockfd;
        struct sockaddr_in addr_svr;
        void client_init();
        int server_init();
        int connect_to_server();
        void inputID();
};

#endif