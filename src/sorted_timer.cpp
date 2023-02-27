#include "../include/sorted_timer.hpp"

sorted_timers timer_lst;

void timer_handler(int signo) {
    /* 定时处理任务 */
    /* 每次接收到SIGALRM信号 */
    timer_lst.tick();
    alarm( TIME_SLOT );
}

void sorted_timers::tick() {
    time_t t = time(nullptr);
    util_timer* temp = head;
    while (temp)
    {
        if(temp->t <= t) {
            temp->call_back(temp->user_data);
            remove_timer(temp);
            temp = head;
        }else{
            break;
        }
    }
}

sorted_timers::sorted_timers() : head(nullptr), tail(nullptr), size(0){

}

sorted_timers::~sorted_timers() {
    util_timer* temp;
    while(head) {
        temp = head;
        head = head->next;
        head->prev = nullptr;
        delete temp;
    }
}

void sorted_timers::add_timer(util_timer* timer) {
    if(head == nullptr && tail == nullptr) {
        /* 链表里面没有计时器 */
        head = timer;
        tail = timer;
        ++size;
        return;
    }
    if(head == tail) {
        /* 链表里面只有一个计时器 */
        if(tail->t <= timer->t) {
            /* 放到尾部 */
            tail->next = timer;
            timer->prev = head;
            tail = timer;
        }else{
            /* 放到首部 */
            timer->next = head;
            head->prev = timer;
            head = timer;
        }
        ++size;
        return;
    }
    /* 链表里面至少有两个计时器 */
    if(tail->t <= timer->t) {
        /* 放在尾部 */
        tail->next = timer;
        timer->prev = tail;
        tail = timer;
    }else if(head->t > timer->t) {
        /* 放在首部 */
        timer->next = head;
        head->prev = timer;
        head = timer;
    }else{
        /* 放在链表中间 */
        util_timer* temp = head;
        while(temp && temp->t <= timer->t) {
            temp = temp->next;
        }
        timer->next = temp->next;
        temp->next->prev = timer;
        temp->next = timer;
        timer->prev = temp;
    }
    ++size;
}

void sorted_timers::remove_timer(util_timer* timer) {
    if(timer == nullptr) {
        return;
    }
    if(head == nullptr && tail == nullptr) {
        /* 没有定时器，说明出错了 */
        throw std::exception();
    }
    if(head == timer && tail == timer) {
        /* 只有一个定时器 */
        head = nullptr;
        tail = nullptr;
        delete timer;
        --size;
        return;
    }
    /* 至少有两个定时器 */
    if(head == timer) {
        /* 删除头部定时器 */
        head = head->next;
        head->prev = nullptr;
        delete timer;
    }else if(tail == timer) {
        /* 删除尾部定时器 */
        tail = tail->prev;
        tail->next = nullptr;
        delete timer;
    }else{
        /* 删除中间的定时器 */
        util_timer* prev = timer->prev;
        util_timer* next = timer->next;
        prev->next = next;
        next->prev = prev;
        delete timer;
    }
    --size;
}  

void sorted_timers::adjust_timer(util_timer* timer) {
    if(!timer) {
        return;
    }
    util_timer* timer_copy = new util_timer();
    timer_copy->call_back = timer->call_back;
    timer_copy->user_data = timer->user_data;
    timer->user_data->timer = timer_copy;
    add_timer(timer_copy);
    remove_timer(timer);
}