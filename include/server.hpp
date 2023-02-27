#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include "utils.hpp"


#define DEFAULT_PORT 80
#define IP_LENGTH 16

class http_conn;

class server {
    friend http_conn;
public:
    server(const char* ip, int port);
    server();
    ~server();
    void init();
    int get_listenfd();
    int http_accept(sockaddr_in* client, socklen_t* len);
private:
    int m_listenfd;
    char m_ip[IP_LENGTH];
    int m_port;
    sockaddr_in m_address;
};

#endif