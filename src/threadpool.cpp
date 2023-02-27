#include "../include/threadpool.hpp"

threadpool::threadpool()
    : m_thread_number(8), m_max_request(10000), m_stop(false){
    
    m_threads = new pthread_t[m_thread_number];
    if(!m_threads) {
        throw std::exception();
    }
    for(int i = 0;i < m_thread_number;++i) {
        printf("创建第%d个线程...\n", i+1);
        if(pthread_create(m_threads+i, nullptr, work, this) != 0) {
            delete [] m_threads;
            throw std::exception();
        }

        if(pthread_detach(m_threads[i]) != 0) {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

threadpool::threadpool(int thread_number, int max_request)
    : m_thread_number(thread_number), m_max_request(max_request){
        if(thread_number <= 0 || max_request <= 0) {
            throw std::exception();
        }
    m_threads = new pthread_t[m_thread_number];
    if(!m_threads) {
        throw std::exception();
    }

    for(int i = 0;i < m_thread_number;++i) {
        if(pthread_create(m_threads+i, nullptr, work, nullptr)) {
            throw std::exception();
        }

        if(pthread_detach(m_threads[i])) {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

void* threadpool::work(void* arg) {
    threadpool* pool = (threadpool*)arg;
    pool->run();
    return pool;
}

bool threadpool::append(http_conn* request) {
    m_queuemutex.lock();
    if(m_workqueue.size() == m_max_request) {
        m_queuemutex.unlock();
        return false;
    }
    m_workqueue.push(request);
    m_queuemutex.unlock();
    m_queuesem.post();
    return true;
}

void threadpool::run() {
    while (!m_stop) {
        m_queuesem.wait();
        m_queuemutex.lock();
        if(m_workqueue.empty()) {
            m_queuemutex.unlock();
            continue;
        }
        http_conn* request = m_workqueue.front();
        m_workqueue.pop();
        m_queuemutex.unlock();
        if(!request) {
            continue;
        }
        request->process();
    }
}

threadpool::~threadpool(){
    m_stop = true;
    delete [] m_threads;
}