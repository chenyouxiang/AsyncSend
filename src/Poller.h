#ifndef POLLER_H
#define POLLER_H

#include <sys/epoll.h>
#include <set>
#include <utility>

namespace AsyncSend
{

class ConnectionBase;
class Connection;
class Poller{
public:
    Poller();
    ~Poller();
    void add_event(ConnectionBase* c, int mask);
    void remove_event(int fd, int mask);
    void remove_all_event(int fd);
    void loop(int timeout);
    void append_active_list(ConnectionBase* c);
    void append_delete_list(ConnectionBase* c);
    int get_active_count();
private:
    int epoll_fd_;
    struct epoll_event events_[1024*10];
    struct epoll_event poll_events_[1024*10];
    std::set<ConnectionBase*> active_list_;
    std::set<ConnectionBase*> delete_list_;
    std::set<std::pair<unsigned long long,ConnectionBase*>> timer_list_;
    unsigned long long get_ticks();
};
}
#endif