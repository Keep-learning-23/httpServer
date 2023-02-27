#ifndef HTTP_CONN_HPP
#define HTTP_CONN_HPP

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <algorithm>
#include <sys/uio.h>
#include "server.hpp"
#include "epoll.hpp"

#define MAX_FILENAME_LENGTH 200
#define READ_BUFFER_SIZE 2048       
#define WRITE_BUFFER_SIZE 2048

struct util_timer;

class http_conn {
    friend epoll;
public:
    typedef enum {GET=0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH} METHOD; 
    typedef enum {NO_REQUEST=0, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERVAL_ERROR, CLOSE_CONNECTION} HTTP_CODE;
    typedef enum {HTML=0, JPG, MP3, ZIP, MP4, NONE} FILETYPE;
public:
    http_conn(epoll* epo, server* http_server);
    ~http_conn();
    void init_conn(int sockfd, sockaddr_in address);
    void close_conn();
    void init();
    void process();
    bool read();
    bool write();
    void setsock(int sockfd, struct sockaddr_in address);
    std::string get_line();
    std::string get_left();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);
    void do_request();
    void add_firstline(int index);
    void add_headlines();
    void add_blankline();
    void add_content();
    void unmap();
    void map();
public: 
    static int m_user_count;
    util_timer* timer;
private:
    /* 通信socket */
    int m_sockfd;

    /* 用户的地址 */
    sockaddr_in m_address;
    
    /* epoll */
    epoll* m_epo;

    /* server */
    server* m_server;

    /* 读缓冲区 */
    char m_read_buf[READ_BUFFER_SIZE];
    int m_read_index;
    int m_cur_read_index;
    
    /* 写缓冲区 */
    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_index;
    int m_have_write;
    int m_total_write;

    /* 请求方法 */
    METHOD m_method;

    /* 请求资源url */
    std::string m_url;
    
    /* HTTP请求版本 */
    std::string m_version;

    /* 请求头 */
    std::unordered_map<std::string, std::string> m_request_head;

    /* 请求体 */
    std::string m_request_content;

    /* 请求文件的绝对路径 */
    std::string m_filename;

    /* 请求文件类型 */
    FILETYPE m_file_type;
    /* 该HTTP请求是否保持连接 */
    bool m_linger;

    /* 目标文件被映射到内存中的地址 */
    char* m_file_address;

    /* 目标文件的具体信息 */
    struct stat m_file_stat;

    /* 分散写iov */
    struct iovec iov[2];
};

#endif