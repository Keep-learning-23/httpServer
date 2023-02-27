#ifndef LOCKER_HPP
#define LOCKER_HPP

#include <exception>
#include <pthread.h>
#include <semaphore.h>

/* 封装信号量的类 */
class sem {
public:
    sem();
    ~sem();
    bool wait();
    bool post();
private:
    sem_t m_sem;
};


class mutex {
public:
    mutex();
    ~mutex();
    bool lock();
    bool unlock();
private:
    pthread_mutex_t m_mutex;
};

class cond {
public:
    cond();
    ~cond();
    bool wait();
    bool signal();
private:
    pthread_cond_t m_cond;
    pthread_mutex_t m_mutex;
};

#endif