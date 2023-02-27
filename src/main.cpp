#include "../include/epoll.hpp"
#include "../include/threadpool.hpp"
#include "../include/utils.hpp"
#include "../include/http_conn.hpp"
#include "../include/locker.hpp"
#include "../include/server.hpp"
#include "../include/sorted_timer.hpp"

#define MAX_FD 65536

extern sorted_timers timer_lst;

void* call_back(http_conn* arg) {
    arg->close_conn();
    return nullptr;
}

int main(int argc, char* argv[]) {
    /* 设置信号处理函数 */
    utils::add_sig(SIGALRM, timer_handler);
    server* http_server;
    threadpool* pool;
    epoll* epo;

    if(argc == 3) {
        http_server = new server(argv[1], atoi(argv[2]));
    }else{
        http_server = new server();
    }
    http_server->init();

    pool = new threadpool();

    epo = new epoll();
    epo->addfd(http_server->get_listenfd(), false);

    utils::add_sig(SIGPIPE, utils::sigpipe_handler);

/* 提前构造一个用户数组，数量为65536个，使用sockfd作为索引，所以0,1,2,3并不作为用户 */
    std::allocator<http_conn> alloc;
    http_conn* users = alloc.allocate(MAX_FD);
    for(int i = 0;i < MAX_FD;++i) {
        alloc.construct(users+i, epo, http_server);
    }

    int fd, connfd;

    /* 用户信息 */
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    char ip[IP_LENGTH];
    int port;

    /* 开始定时 */
    alarm(TIME_SLOT);

    while (true)
    {
        /* epoll默认阻塞在这里，可能会被信号中断 */
        int number = epo->wait();
        if((number < 0) && (errno != EINTR)) {
            printf("epoll failure\n");
            break;
        }

        for(int i = 0;i < number;++i) {
            fd = epo->events[i].data.fd;
            if(fd == http_server->get_listenfd()) {
                /* 有用户连接进来 */
                connfd = http_server->http_accept(&client, &len);
                if(connfd == -1) {
                    perror("accept");
                    continue;
                }
                users[connfd].init();
                users[connfd].setsock(connfd, client);
                port = ntohs(client.sin_port);
                inet_ntop(AF_INET, &client.sin_addr.s_addr, ip, IP_LENGTH);
                printf("%s:%d已连接,fd:%d...\n", ip, port, connfd);

                http_conn::m_user_count++;
                epo->addfd(connfd, true);

                util_timer* timer = new util_timer();
                timer->call_back = call_back;
                timer->user_data = &users[connfd];
                users[connfd].timer = timer;
                timer_lst.add_timer(timer);
            }else if(epo->events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                users[fd].close_conn();
                timer_lst.remove_timer(users[fd].timer);
            }else if(epo->events[i].events & EPOLLIN) {
                if(!users[fd].read()) {
                    users[fd].close_conn();
                    timer_lst.remove_timer(users[fd].timer);
                }else{
                    pool->append(&users[connfd]);
                    timer_lst.adjust_timer(users[fd].timer);
                }
            }else if(epo->events[i].events & EPOLLOUT) {
                if(!users[fd].write()) {
                    users[fd].close_conn();
                    timer_lst.remove_timer(users[fd].timer);
                }else{
                    timer_lst.adjust_timer(users[fd].timer);
                }
            }
        }
    }

    alloc.deallocate(users, MAX_FD);
    epo->epoll_close();
    close(http_server->get_listenfd());
    delete pool;
    delete http_server;
    delete epo;
    return 0;
}