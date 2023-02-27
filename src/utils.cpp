#include "../include/utils.hpp"

int utils::setnonblocking(int fd) {
    int old_flag = fcntl(fd, F_GETFL);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_flag);
    return old_flag;
}

int utils::add_sig(int signo, void (*handler)(int)) {
    signal(signo, handler);
    return 0;
}

void utils::sigpipe_handler(int signo) {
    printf("接收到信号...\n");
}