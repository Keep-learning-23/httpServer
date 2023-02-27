#include "../include/http_conn.hpp"

int http_conn::m_user_count = 0;

static std::string root_path = "../resource";

static const char* firstline[] = {"HTTP/1.1 200 OK\r\n", "HTTP/1.1 404 Not Found\r\n",
    "HTTP/1.1 400 Bad Request\r\n", "HTTP/1.1 403 Forbidden"};

http_conn::http_conn(epoll* epo, server* http_server) 
    : m_epo(epo), m_server(http_server){

}

http_conn::~http_conn() {

}

void http_conn::init() {
    bzero(m_read_buf, READ_BUFFER_SIZE);
    m_read_index = 0;
    m_cur_read_index = 0;
    bzero(m_write_buf, WRITE_BUFFER_SIZE);
    m_write_index = 0;
    m_have_write = 0;
    m_total_write = 0;
    m_method = GET;

    m_url.clear();
    m_version.clear();
    m_request_head.clear();
    m_request_content.clear();
    m_filename.clear();

    /* 保持连接 */
    m_linger = true;

    m_file_address = nullptr;
    iov[0].iov_base = nullptr;
    iov[0].iov_len = 0;
    iov[1].iov_base = nullptr;
    iov[1].iov_len = 0;
}

void http_conn::init_conn(int sockfd, sockaddr_in address) {
    m_sockfd = sockfd;
    m_address = address;
}

void http_conn::close_conn() {
    /* 断开连接 */
    if(m_sockfd != -1) {
        m_epo->removefd(m_sockfd);
        init();
        m_user_count--;
        char ip[16];
        printf("%s:%d已断开连接...\n", inet_ntop(AF_INET, &m_address.sin_addr.s_addr,
            ip, 16), ntohs(m_address.sin_port));
    }else{
        printf("m_sockfd == -1...\n");
    }
}

http_conn::HTTP_CODE http_conn::process_read() {
    HTTP_CODE ret = NO_REQUEST;
    if(strlen(m_read_buf) == 0) {
        return NO_REQUEST;
    }
    std::string line = get_line();
    std::istringstream iss(line);
    std::string method;
    if(iss >> method) {
        if(method == "GET") {
            m_method = GET;
            ret = GET_REQUEST;
        }else if(method == "POST") {
            m_method = POST;
        }else if(method == "HEAD") {
            m_method = HEAD;
        }else if(method == "PUT") {
            m_method = PUT;
        }else if(method == "DELETE") {
            m_method = DELETE;
        }else if(method == "TRACE") {
            m_method = TRACE;
        }else if(method == "OPTIONS") {
            m_method = OPTIONS;
        }else if(method == "CONNECT") {
            m_method = CONNECT;
        }else if(method == "PATCH") {
            m_method = PATCH;
        }else{
            m_method = GET;
            ret = GET_REQUEST;
        }
    }else{
        return BAD_REQUEST;
    }

    if(iss >> m_url) {
        m_filename = root_path + m_url;
        if(m_url == "/") {
            m_filename += "index.html";
        }
    }else{
        return BAD_REQUEST;
    }

    /* 判断文件是否存在 */
    int temp = stat(m_filename.c_str(), &m_file_stat);
    if(temp == -1) {
        if(errno == ENOENT) {
            return NO_RESOURCE;
        }else{
            return BAD_REQUEST;
        }
    }
    /* 判断文件是否是文件夹 */
    if(S_ISDIR( m_file_stat.st_mode )) {
        /* 请求的是一个文件夹 */
        return NO_RESOURCE;
    }

    /* 判断是否有权限 */
    if(!(m_file_stat.st_mode & S_IROTH)) {
        return FORBIDDEN_REQUEST;
    }

    if(iss >> m_version) {

    }else{
        return BAD_REQUEST;
    }

    std::string key, value;
    while (true)
    {
        line = get_line();
        if(line.empty()) {
            break;
        }
        int pos;
        for(int i = 0;i < line.size();++i) {
            if(line[i] == ':') {
                pos = i;
                break;
            }
        }
        key = std::string(line.begin(), line.begin()+pos);
        value = std::string(line.begin()+pos+2, line.end());
        m_request_head.insert({key, value});
    }

    m_request_content = get_left();
    return ret;
}

std::string http_conn::get_line() {
    std::string ans;
    while (m_cur_read_index < m_read_index)
    {
        if(m_read_buf[m_cur_read_index] != '\r'
            && m_read_buf[m_cur_read_index] != '\n') {
            ans.push_back(m_read_buf[m_cur_read_index]);
            m_cur_read_index++;
        }else if(m_read_buf[m_cur_read_index] == '\r') {
            m_cur_read_index++;
            continue;
        }else if(m_read_buf[m_cur_read_index] == '\n') {
            m_cur_read_index++;
            break;
        }
    }
    return ans;
}

std::string http_conn::get_left() {
    std::string ans;
    while(m_cur_read_index < m_read_index) {
        ans.push_back(m_read_buf[m_cur_read_index++]);
    }
    return ans;
}

void http_conn::add_firstline(int index) {
    int ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index
    , "%s", firstline[index]);
    m_write_index += ret;
}

void http_conn::add_headlines() {
    int ret;
    ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index
    , "Host: %s\r\n", m_server->m_ip);
    m_write_index += ret;
    switch (m_file_type)
    {
    case HTML:
        ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index,
            "Content-Type: text/html\r\n");
        m_write_index += ret;
        ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index,
            "Content-Length: %ld\r\n", m_file_stat.st_size);
        m_write_index += ret;
        break;
    case JPG:
        ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index,
            "Content-Type: image/jpeg\r\n");
        m_write_index += ret;
        ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index,
            "Content-Length: %ld\r\n", m_file_stat.st_size);
        m_write_index += ret;
        break;
    case MP3:
        ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index,
            "Content-Type: audio/mpeg\r\n");
        m_write_index += ret;
        ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index,
            "Content-Length: %ld\r\n", m_file_stat.st_size);
        m_write_index += ret;
        break;
    case MP4:
        ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index,
            "Content-Type: video/mp4\r\n");
        m_write_index += ret;
        ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index,
            "Content-Length: %ld\r\n", m_file_stat.st_size);
        m_write_index += ret;
        break;
    case ZIP:
        ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index,
            "Content-Type: application/octet-stream\r\n");
        m_write_index += ret;
        ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index,
            "Content-Length: %ld\r\n", m_file_stat.st_size);
        m_write_index += ret;
        break;
    case NONE:
        break;
    default:
        break;
    }
}

void http_conn::add_blankline() {
    int ret = snprintf(m_write_buf+m_write_index, WRITE_BUFFER_SIZE-m_write_index
    , "\r\n");
    m_write_index += ret;
}

void http_conn::add_content() {
    iov[0].iov_base = m_write_buf;
    iov[0].iov_len = m_write_index;
    iov[1].iov_base = m_file_address;
    iov[1].iov_len = m_file_stat.st_size;
    m_total_write = m_write_index + m_file_stat.st_size;
    m_have_write = 0;
}

bool http_conn::process_write(HTTP_CODE ret) {
    switch (ret)
    {
    case GET_REQUEST:
        {
            map();
            add_firstline(0);
            add_headlines();
            add_blankline();
            add_content();
        }
        break;
    case BAD_REQUEST:
        break;
    case NO_RESOURCE:
        break;
    case FORBIDDEN_REQUEST:
        add_firstline(3);
        add_headlines();
        break;
    default:
        break;
    }
    return true;
}

void http_conn::process() {
    /* 子线程通过信号量通知得到任务 */
    /* 处理接收缓冲区的HTTP请求 */
    HTTP_CODE ret = process_read();
    if(ret == NO_REQUEST) {
        m_epo->modfd(m_sockfd, EPOLLIN);
        return;
    }
    bool write_ret = process_write(ret);
    if(!write_ret) {
        close_conn();
        return;
    }
    m_epo->modfd(m_sockfd, EPOLLOUT);
}

bool http_conn::read() {
    if(m_read_index >= READ_BUFFER_SIZE) {
        return false;
    }
    /* 循环读取缓冲区的数据（一次性全部读出来） */
    int recv_bytes = 0;
    while (true)
    {
        /* m_sockfd是非阻塞的 */
        recv_bytes = recv(m_sockfd, m_read_buf+m_read_index, 
                        READ_BUFFER_SIZE-m_read_index, 0);
        if(recv_bytes == -1) {
            if((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                break;
            }
            /* 读取操作出错 */
            return false;
        } else if(recv_bytes == 0) {
            /* 读到了EOF */
            return false;
        } else {
            m_read_index += recv_bytes;
        }
    }
    return true;
}

void http_conn::setsock(int sockfd, struct sockaddr_in address) {
    m_sockfd = sockfd;
    m_address = address;
}

bool http_conn::write() {
    if(m_have_write == m_total_write) {
        /* 已经写完了 */
        m_epo->modfd(m_sockfd, EPOLLIN);
        init();
        return true;
    }
    int bytes;
    while (true)
    {
        bytes = writev(m_sockfd, iov, 2);
        if(bytes == -1) {
            if(errno == EAGAIN) {
                m_epo->modfd(m_sockfd, EPOLLOUT);
                return true;
            }
            unmap();
            return false;
        }
        /* 本次writev向tcp缓冲区写了bytes个数据 */
        m_have_write += bytes;
        
        if(m_have_write == m_total_write) {
            /* 本次http响应写完 */
            unmap();
            if(!m_linger) {
                close_conn();
            }
            init();
            m_epo->modfd(m_sockfd, EPOLLIN);
            return true;
        }
        if(bytes > iov[0].iov_len) {
            iov[0].iov_len = 0;
            iov[1].iov_base = m_file_address + m_have_write - m_write_index;
            iov[1].iov_len = m_total_write - m_have_write;
        }else{
            iov[0].iov_base = m_write_buf + m_have_write;
            iov[0].iov_len -= bytes;
        }
    }
}

void http_conn::unmap() {
    if(m_file_address) {
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = nullptr;
    }
}

void http_conn::map() {
    int fd = open(m_filename.c_str(), O_RDONLY);
    m_file_address = (char*)mmap(nullptr, m_file_stat.st_size, PROT_READ,
        MAP_PRIVATE, fd, 0);
    close(fd);
    std::string type;
    for(auto it = m_filename.rbegin();it != m_filename.rend();++it) {
        if(*it == '.') {
            break;
        }
        type.push_back(*it);
    }
    std::reverse(type.begin(), type.end());
    if(type == "html") {
        m_file_type = HTML;
    }else if(type == "jpg") {
        m_file_type = JPG;
    }else if(type == "mp3") {
        m_file_type = MP3;
    }else if(type == "mp4") {
        m_file_type = MP4;
    }else if(type == "zip") {
        m_file_type = ZIP;
    }else{
        m_file_type = NONE;
    }
}