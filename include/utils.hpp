#ifndef UTILS_HPP
#define UTILS_HPP
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdio.h>
#include "server.hpp"

namespace utils {
    int setnonblocking(int fd);

    int add_sig(int signo, void (*handler)(int));

    void sigpipe_handler(int signo);

    void sigalarm_handler(int signo);
}

#endif