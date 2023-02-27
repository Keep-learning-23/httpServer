#ifndef SORTED_TIMER_HPP
#define SORTED_TIMER_HPP

#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include "http_conn.hpp"

#define TIME_SLOT 10
#define TIME_STAY 300

class http_conn;
struct util_timer;
class sorted_timers;

void timer_handler(int signo);

struct util_timer {
    util_timer() : t(time(nullptr)+TIME_STAY), user_data(nullptr), 
        call_back(nullptr), prev(nullptr), next(nullptr) {}
    ~util_timer() {}
    time_t t;
    http_conn* user_data;
    void* (*call_back) (http_conn*);
    util_timer* prev;
    util_timer* next;
};

class sorted_timers {
public:
    sorted_timers();
    ~sorted_timers();
    void add_timer(util_timer* timer);
    void remove_timer(util_timer* timer);
    void adjust_timer(util_timer* timer);
    void tick();
private:
    util_timer* head;
    util_timer* tail;
    int size;
};

#endif