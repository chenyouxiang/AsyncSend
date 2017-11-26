#ifndef CONNECTION_BASE_H
#define CONNECTION_BASE_H
namespace AsyncSend{
    class ConnectionBase {
    public:
        virtual ~ConnectionBase(){

        };
        virtual void handle_write_event()=0;
        virtual void handle_read_event()=0;
        friend class Poller;
    private:
        virtual int get_fd()=0;
        virtual void close_connection()=0;
    };
}
#endif