#include <iostream>
#include "Buffer.h"
#include "Connection.h"
#include "Poller.h"
#include <unistd.h>
#include "Cmdline.h"
#include <sys/msg.h>
#include <signal.h>
#include "FIFOConnection.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace AsyncSend;
using namespace boost::property_tree;  

/**
 * [send_http_msg 发送http消息]
 * @param poller [description]
 * @param msg    [description]
 * @param size   [description]
 */
void send_http_msg(Poller& poller, char* msg, int size){
     try{

        //解析json
        std::stringstream json_str(string(msg,size));
        ptree json;  
        read_json(json_str, json);  
        string host = json.get<string>("host");
        string url = json.get<string>("url");
        string body = json.get<string>("body");
        std::cout<<"get json from php,host:"<<host<<",url:"<<url<<",body:"<<body<<"\r\n";

        int port = 80;
        auto found = host.find(":");
        if(found != string::npos){
            port = std::atoi(host.substr(found+1).c_str());
            host = host.substr(0,found);
            std::cout<<"host:"<<host<<"port:"<<port<<"\r\n";
        }
        //拼http协议
        std::stringstream http;
        http<<"POST "<<url<<" HTTP/1.1\r\n"<<"Host:"<<host<<"\r\nUser-Agent: curl/7.29.0\r\nAccept: */*\r\nContent-Type:application/x-www-form-urlencoded\r\nContent-Length:"<<body.size()<<"\r\n\r\n"<<body;

        //开始发送
        auto connection = new Connection(&poller);
        char err[200];
        if(!connection->create_connection(host.c_str(), port, err)) {
            std::cout<<"create connection failed,host:"<<host<<"\r\n";
            delete connection;
            return;
        }
        std::string content = http.str();
        std::cout<<"start send content:\r\n"<<content<<"\r\n";
        connection->send(content.c_str(),content.size());
    }  
    catch(ptree_error & e) {  
       std::cout<<"send http msg failed\r\n";
    }
}

bool g_is_quit = false;

void sig_quit(int sig){
    g_is_quit = true;
}

int main(int argc, char *argv[]){
    
    signal(SIGALRM, sig_quit);
    signal(SIGQUIT, sig_quit);

    //解析命令行参数
    cmdline::parser parser;
    parser.add<int>("msg-key", 'k', "msg key", true, 0);
    parser.add<int>("msg-size", 's', "msg size", true, 0);
    parser.parse_check(argc, argv);
    int msg_key = parser.get<int>("msg-key");
    int msg_size = parser.get<int>("msg-size");

    //事件循环器
    Poller poller;

    //创建消息队列
    int msg_id = msgget(msg_key, 0666|IPC_CREAT);
    if(-1 == msg_id){
        std::cout<<"create msg queue failed\r\n";
        exit(0);
    }

    //消息队列数据结构
    struct php_buf {
        long mtype;
        char msg[0];
    };
    php_buf* buf =static_cast<php_buf*>(malloc(msg_size + sizeof(long)+1));
    

    //事件循环
    while(!g_is_quit){
        poller.loop(1000);
        int nread = 0;
        int msg_count = 0;
        do{
            //限制活动连接加新增的消息不能超过100，防止连接过多，远端服务器压力过大
            if(100 < msg_count + poller.get_active_count()){
                break;
            }
            nread = msgrcv(msg_id, buf ,msg_size ,0, IPC_NOWAIT);
            if(nread > 0){
                send_http_msg(poller, buf->msg, nread);
            }
            msg_count++;
        }while(nread > 0 && msg_count<100);
    }

    msgctl(msg_id, IPC_RMID,NULL);
    free(buf);
    std::cout<<"end\r\n";
}