#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP
#include <queue>
#include <exception>
#include <pthread.h>
#include <stdio.h>
#include "locker.hpp"
#include "http_conn.hpp"

class threadpool {
public:
    threadpool();
    threadpool(int thread_number, int max_request);
    ~threadpool();
    bool append(http_conn* request);
private:
    static void* work(void* arg);
    void run();
private:
    int m_thread_number;                    /* 线程数 */
    int m_max_request;                      /* 最大连接数 */
    pthread_t* m_threads;                   /* 线程号 */
    std::queue<http_conn*> m_workqueue;     /* 工作队列 */
    mutex m_queuemutex;                     /* 工作队列的互斥锁 */
    sem m_queuesem;                         /* 工作队列的信号量 */
    bool m_stop;                            /* 是否结束线程 */
};

#endif