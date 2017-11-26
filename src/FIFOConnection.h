#ifndef FIFO_CONNECTION_H
#define FIFO_CONNECTION_H

#include "base/ConnectionBase.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <string>
#include "Buffer.h"

namespace AsyncSend
{
class Poller;

class FIFOConnection: public ConnectionBase
{
public:
	FIFOConnection(Poller* poller, std::string& fifo_path);
	virtual ~FIFOConnection();
	virtual void handle_write_event();
    virtual void handle_read_event();
private:
	int fd_;
	Poller* poller_;
	std::string fifo_path_;
	Buffer recv_buf_;
	virtual int get_fd();
	virtual void close_connection(){
		
	}
};
}
#endif