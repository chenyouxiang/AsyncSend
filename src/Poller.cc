#include "Poller.h"
#include "base/ConnectionBase.h"
#include "Connection.h"
#include<unistd.h>
#include <iostream>
#include <sys/time.h>

using namespace AsyncSend;

Poller::Poller(){
    epoll_fd_ = epoll_create(1024*10);
}

Poller::~Poller(){
    close(epoll_fd_);
}

void Poller::add_event(ConnectionBase* c, int mask){
    int fd = c->get_fd();
    if(fd < 0){
        return;
    }
    struct epoll_event& ev = events_[fd];
    int op = ev.data.fd ==0?EPOLL_CTL_ADD:EPOLL_CTL_MOD;
    mask |= EPOLLET;
    ev.events |= mask;
    ev.data.fd = fd;
    ev.data.ptr = c;
    epoll_ctl(epoll_fd_ ,op, fd, &ev);
}

void Poller::remove_event(int fd, int mask){
    if(fd < 0){
        return;
    }
    struct epoll_event& ev = events_[fd];
    int op = EPOLL_CTL_MOD;
    ev.events &= ~mask;
    ev.data.fd = fd;
    epoll_ctl(epoll_fd_ ,op, fd, &ev);
}

void Poller::remove_all_event(int fd){
    if(fd < 0){
        return;
    }
    struct epoll_event& ev = events_[fd];
    if(ev.data.fd == 0){
        return;
    }
    int op = EPOLL_CTL_MOD;
    ev.events = 0;
    epoll_ctl(epoll_fd_ ,op, fd, &ev);
    memset(&events_[fd],0,sizeof(events_[fd]));
}

void Poller::append_active_list(ConnectionBase* c){
    std::cout<<"append connection to active list:"<<c->get_fd()<<"\r\n";
    active_list_.insert(c);
    auto now=get_ticks();
    //插入超时队列，超时时间360s
    timer_list_.insert({now + 1000 * 360, c});
}
void Poller::append_delete_list(ConnectionBase* c){
    auto iter = active_list_.find(c);
    if(iter != active_list_.end()){
        active_list_.erase(iter);
        std::cout<<"remove connection from active list:"<<c->get_fd()<<"\r\n";
    }else{
        return;
    }
    std::cout<<"append connection to delete list:"<<c->get_fd()<<"\r\n";
    delete_list_.insert(c);
}

/**
 * [Poller::loop description]
 * @param timeout [单位毫秒]
 */
void Poller::loop(int timeout){
    //std::cout<<"start epoll_wait\r\n";
    int retval = epoll_wait(epoll_fd_,poll_events_,1024*10,timeout);
    //std::cout<<"epoll_wait count:"<<retval<<"\r\n";
    
    //处理事件
    if (retval > 0) {
        for(int i=0; i<retval; i++){
            struct epoll_event& e = poll_events_[i];
            ConnectionBase* c =static_cast<ConnectionBase*>(e.data.ptr);
            if (c && e.events & EPOLLIN) {
                c->handle_read_event();
            }
            if (c && e.events & EPOLLOUT) {
                c->handle_write_event();
            };
        }
    }

    //连接超时控制
    auto now = get_ticks();
    auto timeout_iter = timer_list_.lower_bound({now,nullptr}); 

    for(auto iter = timer_list_.begin(); iter != timer_list_.end(); ){
        if(iter == timeout_iter){
            break;
        }
        auto found = active_list_.find(iter->second);
        if(found == active_list_.end()){
            timer_list_.erase(iter++);
            continue;
        }
        std::cout<<"connection timeout:"<<iter->second->get_fd()<<"\r\n";
        iter->second->close_connection();
        timer_list_.erase(iter++);
    }

    //删除连接
    for(auto iter = delete_list_.begin(); iter!=delete_list_.end(); iter++){
        auto fd = (*iter)->get_fd();
        std::cout<<"delete connection:"<<fd<<"\r\n";
        delete *iter;
    }
    delete_list_.clear();
}

int Poller::get_active_count(){
    return active_list_.size();
}

unsigned long long Poller::get_ticks(){
    struct timeval now = { 0 };
    gettimeofday( &now,NULL );
    unsigned long long u = now.tv_sec;
    u *= 1000;
    u += now.tv_usec / 1000;
    return u;
}