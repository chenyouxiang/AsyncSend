#ifndef CONNECTION_H
#define CONNECTION_H

#define ANET_CONNECT_NONE 0
#define ANET_CONNECT_NONBLOCK 1

#include "Buffer.h"
#include "http/parser.h"
#include "base/ConnectionBase.h"

namespace AsyncSend
{
using namespace AsyncSend::http;
class Poller;
class Connection: public ConnectionBase
{
public:
    explicit Connection(Poller* poller);
    virtual ~Connection();
    bool create_connection(const char* ip, int port,char* err);
    bool send(const char* content, size_t length);
    virtual void handle_write_event();
    virtual void handle_read_event();
private:
    int fd_;
    Buffer send_buf_;
    Buffer recv_buf_;
    bool is_close_;
    Poller* poller_;
    parser parser_;
private:
    virtual int get_fd();
    virtual void close_connection();
};
}
#endif