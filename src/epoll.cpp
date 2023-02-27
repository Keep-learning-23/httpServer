#include "../include/epoll.hpp"

epoll::epoll() {
    events = new epoll_event[MAX_EVENT_NUMBER];
    if(!events) {
        throw std::exception();
    }
    epollfd = epoll_create(5);
}

epoll::~epoll() {
    delete [] events;
}

void epoll::addfd(int fd, bool one_shot) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP | EPOLLRDHUP;

    if(one_shot) {
        event.events |= EPOLLONESHOT;
    }

    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);

    // 设置文件描述符非阻塞
    utils::setnonblocking(fd);
}

void epoll::removefd(int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void epoll::modfd(int fd, int event) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = event | EPOLLONESHOT | EPOLLHUP | EPOLLERR | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

int epoll::wait() {
    return epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
}

void epoll::epoll_close() {
    close(epollfd);
}