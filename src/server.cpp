#include "../include/server.hpp"

server::server(const char* ip, int port) : m_port(port) {
    if(port < 0) {
        printf("端口号不能为负数");
        throw std::exception();
    }
    bzero(m_ip, IP_LENGTH);
    memcpy(m_ip, ip, strlen(ip));
    bzero(&m_address, sizeof(m_address));
}

server::server() :m_port(DEFAULT_PORT) {
    bzero(m_ip, IP_LENGTH);
    bzero(&m_address, sizeof(m_address));
}

server::~server() {

}

void server::init() {
    char ip[16] = "0.0.0.0";
    if(strlen(m_ip) != 0) {
        memcpy(ip, m_ip, 16);
    }
    printf("%s:%d\n", ip, m_port);
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_listenfd == -1) {
        perror("socket");
        throw std::exception();
    }
    int reuse = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
    int ret;
    m_address.sin_family = AF_INET;
    if(strlen(m_ip) == 0) {
        /* 采用默认ip地址 */
        m_address.sin_addr.s_addr = htons(INADDR_ANY);
    }else{
        ret = inet_pton(AF_INET, m_ip, &m_address.sin_addr.s_addr);
        if(ret == -1) {
            perror("inet_pton");
            throw std::exception();
        }
    }
    m_address.sin_port = htons(m_port);
    ret = bind(m_listenfd, (struct sockaddr*)&m_address, sizeof(m_address));
    if(ret == -1) {
        perror("bind");
        throw std::exception();
    }
    ret = listen(m_listenfd, 20);
    if(ret == -1) {
        perror("listen");
        throw std::exception();
    }
}

int server::get_listenfd() {
    return m_listenfd;
}

int server::http_accept(sockaddr_in* client, socklen_t* len) {
    return accept(m_listenfd, (struct sockaddr*)client, len);
}