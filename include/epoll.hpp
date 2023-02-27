#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <unistd.h>
#include <sys/epoll.h>
#include <exception>
#include "utils.hpp"
#define MAX_EVENT_NUMBER 10000

class epoll {
public:
    epoll();
    ~epoll();
    void addfd(int fd, bool one_shot);
    void removefd(int fd);
    void modfd(int fd, int ev);
    int wait();
    void epoll_close();
    epoll_event* events;
private:
    int epollfd;
};

#endif