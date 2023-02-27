#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

static const char* request = "GET / HTTP/1.1\r\nConnection: Keep-alive\r\n\r\nrequest body";

int setnonblocking(int fd) {
    int old_flag = fcntl(fd, F_GETFL);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_flag);
    return old_flag;
}

void addfd(int epollfd, int fd) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLOUT | EPOLLET | EPOLLERR | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void num_connect(int num, int epollfd, const char* ip, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
    int ret;
    int count = 0;/* 计算成功连接的个数 */
    for(int i = 0;i < num;++i) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        ret = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        if(ret == -1) {
            perror("connect");
            /* 没有连接上的 */
        }else{
            /* 连接上的 */
            count++;
            addfd(epollfd, sock);
        }
    }
    printf("同时发起%d次连接,%d已连接,成功率为%f%%...\n", num, count, count*1.0/num*100);
}

void close_conn(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

int main(int argc, char* argv[]) {
    if(argc != 4) {
        printf("usage : %s ip_address port connection_time\n", basename(argv[0]));
        return -1;
    }
    char* ip = argv[1];
    int port = atoi(argv[2]);
    int num = atoi(argv[3]);
    int epollfd = epoll_create(5);
    if(epollfd == -1) {
        perror("epoll_create");
        return -1;
    }
    epoll_event* events = new epoll_event[10000];
    if(!events) {
        printf("epoll_event内存分配失败...\n");
        return -1;
    }
    num_connect(num, epollfd, ip, port);
    int number, ret;
    char buffer[1024];
    while (true)
    {
        number = epoll_wait(epollfd, events, 10000, 2000);
        for(int i = 0;i < number;++i) {
            int fd = events[i].data.fd;
            if(events[i].events & EPOLLIN) {
                ret = read(fd, buffer, 10240-1);
                printf("fd:%d接收到%d个字节的数据...\n", fd, ret);
                epoll_event event;
                event.data.fd = fd;
                event.events = EPOLLOUT | EPOLLET | EPOLLERR;
                epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
            }else if(events[i].events & EPOLLOUT) {
                ret = write(fd, request, strlen(request));
                printf("fd:%d写入%d个字节的数据...\n", fd, ret);
                epoll_event event;
                event.data.fd = fd;
                event.events = EPOLLIN | EPOLLET | EPOLLERR;
                epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
            }else if(events[i].events & (EPOLLERR | EPOLLRDHUP)) {
                close_conn(epollfd, fd);
            }
        }
    }
    return 0;
}