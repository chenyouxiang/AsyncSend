#include "Connection.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include "Poller.h"
#include <iostream>

using namespace AsyncSend;
using namespace AsyncSend::http;

Connection::Connection(Poller* poller):is_close_(true),poller_(poller),parser_(nullptr){

}

Connection::~Connection(){
    std::cout<<"connection end:"<<fd_<<"\r\n";
    close_connection();
}

/**
 * [Connection::create_connection 创建连接]
 * @param  host [ip或域名]
 * @param  port [端口]
 * @param  err  [错误信息输出]
 * @return      [description]
 */
bool Connection::create_connection(const char* host, int port, char* err){
    if ((fd_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        sprintf(err, "creating socket: %s", strerror(errno));
        return false;
    }
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    if (inet_aton(host, &sa.sin_addr) == 0) {
        struct hostent *he;
        he = gethostbyname(host);
        if (he == NULL) {
            sprintf(err, "can't resolve: %s", host);
            close(fd_);
            return false; 
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }

    int flags;
    if ((flags = fcntl(fd_, F_GETFL)) == -1) {
        sprintf(err, "fcntl(F_GETFL): %s", strerror(errno));
        close(fd_);
        return false;
    }
    if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
        sprintf(err, "fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
        close(fd_);
        return false;
    }
    
    if (connect(fd_, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        if (errno == EINPROGRESS){
            is_close_ = false;
            return true;
        }
        sprintf(err, "connect: %s", strerror(errno));
        close(fd_);
        return false; 
    }
    is_close_ = false;
    return true;
}

/**
 * [Connection::send 发送接口]
 * @param content [发送内容]
 * @param length  [长度]
 */
bool Connection::send(const char* content, size_t length){
    if(is_close_){
        return false;
    }
    send_buf_.append(content,length);
    poller_->append_active_list(this);
    poller_->add_event(this,EPOLLOUT);
    std::cout<<"start send, fd:"<<fd_<<"\r\n";
    return true;
}

/**
 * [Connection::handle_write_event 处理写事件]
 */
void Connection::handle_write_event(){
    std::cout<<"handle write event\r\n";
    if(is_close_){
        return;
    }
    if(send_buf_.readableBytes() == 0){
        return;
    }
    int nwriten = 0;
    while(send_buf_.readableBytes()>0){
         nwriten = write(fd_,send_buf_.peek() ,send_buf_.readableBytes());
         if(-1 == nwriten){
            if(EAGAIN != errno){
                std::cout<<"write error close:"<<strerror(errno)<<"\r\n";
                close_connection();
            }
            break;
         }
         send_buf_.retrieve(nwriten);
    }
    if(send_buf_.readableBytes() == 0){
        poller_->remove_event(fd_,EPOLLOUT);
        poller_->add_event(this,EPOLLIN);
        std::cout<<"send complete, fd:"<<fd_<<"\r\n";
    }
}

/**
 * [Connection::handle_read_event 处理读事件]
 */
void Connection::handle_read_event(){
    std::cout<<"handle read event\r\n";
    if(is_close_){
        return;
    }
    int nreadn = 0;
    do{
        char tmp_buf[1024];
        nreadn = read(fd_,tmp_buf ,1024);
        if(0 == nreadn){
            std::cout<<"coneection has been close by peer\r\n";
            close_connection();
        }else if(-1 == nreadn){
            if(EAGAIN != errno){
                std::cout<<"read error close:"<<strerror(errno)<<"\r\n";
                close_connection();
            }
        }
        if(nreadn > 0){
            recv_buf_.append(tmp_buf,nreadn);
        }
    }while(nreadn > 0);

    string s(recv_buf_.peek(),recv_buf_.readableBytes());

    std::cout<<"read return raw content:\r\n"<<s<<"\r\n";

    parser_.reset(&recv_buf_);
    parser_.execute();
    if(parser_.is_complete()){
        close_connection();
        std::cout<<"read complete, fd:"<<fd_<<",content:\r\n"<<parser_.get_body()<<"\r\n";
    }
}

void Connection::close_connection(){
    if(is_close_){
        return;
    }
    poller_->remove_all_event(fd_);
    close(fd_);
    is_close_ = true;
    poller_->append_delete_list(this);
}

 int Connection::get_fd(){
    return fd_;
 }