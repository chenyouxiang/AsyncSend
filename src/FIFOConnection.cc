#include "FIFOConnection.h"
#include "Poller.h"
#include <iostream>
#include <limits.h>

using namespace AsyncSend;

FIFOConnection::FIFOConnection(Poller* poller, std::string& fifo_path):poller_(poller),fifo_path_(fifo_path){
    if((mkfifo(fifo_path.c_str(), O_CREAT|O_EXCL)<0)&&(errno!=EEXIST))    
    {  
        std::cout<<"mkfifo failed:"<<fifo_path<<"\r\n";
        exit(0);  
    }
    fd_=open(fifo_path.c_str(), O_RDONLY|O_NONBLOCK, 0);
    if(-1 == fd_) {
        std::cout<<"open fifo failed:"<<fifo_path<<"\r\n";
        exit(0);
    }
    poller_->add_event(this,EPOLLIN);
}

FIFOConnection::~FIFOConnection(){
    close(fd_);
    unlink(fifo_path_.c_str());
}

void FIFOConnection::handle_read_event(){
    char tmp_buf[PIPE_BUF + 1];
    int nreadn = read(fd_,tmp_buf ,PIPE_BUF);
    if(nreadn > 0){
        tmp_buf[nreadn] = '\0';
        std::cout<<"read fifo:"<<tmp_buf<<"\r\n";
    }
}

void FIFOConnection::handle_write_event(){

}

int FIFOConnection::get_fd(){
    return fd_;
}