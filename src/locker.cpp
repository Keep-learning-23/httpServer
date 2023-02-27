#include "../include/locker.hpp"

sem::sem() {
    if(sem_init(&m_sem, 0, 0) != 0) {
        throw std::exception();
    }
}

sem::~sem() {
    sem_destroy(&m_sem);
}

bool sem::wait() {
    return sem_wait(&m_sem) == 0;
}

bool sem::post() {
    return sem_post(&m_sem) == 0;
}

mutex::mutex() {
    if(pthread_mutex_init(&m_mutex, nullptr) != 0) {
        throw std::exception();
    }
}

mutex::~mutex() {
    pthread_mutex_destroy(&m_mutex);
}

bool mutex::lock() {
    return pthread_mutex_lock(&m_mutex) == 0;
}

bool mutex::unlock() {
    return pthread_mutex_unlock(&m_mutex) == 0;
}

cond::cond() {
    if(pthread_mutex_init(&m_mutex, nullptr) != 0) {
        throw std::exception();
    }
    if(pthread_cond_init(&m_cond, nullptr) != 0) {
        pthread_mutex_destroy(&m_mutex);
        throw std::exception();
    }
}

cond::~cond() {
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}

bool cond::wait() {
    pthread_mutex_lock(&m_mutex);
    int ret = pthread_cond_wait(&m_cond, &m_mutex);
    pthread_mutex_unlock(&m_mutex);
    return ret == 0;
}

bool cond::signal() {
    return pthread_cond_signal(&m_cond) == 0;
}