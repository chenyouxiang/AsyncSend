#ifndef PARSER_H
#define PARSER_H
#include "http_parser.h"
#include "../Buffer.h"

namespace AsyncSend
{
namespace http
{
class parser{
public:
    parser(Buffer* buf);
    void execute();
    void reset(Buffer* buf){
        _buf = buf;
        _length = 0;
        _is_complete = false;
    }
    size_t get_length(){
        return _length;
    }

    bool is_complete(){
        return _is_complete;
    }

    std::string& get_body(){
        return _body;
    }
private:
    static int on_body_complete(http_parser* p, const char *at, size_t length);
    static int on_headers_complete(http_parser* p, const char *at, size_t length);
    static int on_message_end(http_parser* p);
    Buffer* _buf;
    std::string _body;
    size_t _length;
    http_parser _parser;
    http_parser_settings _option;
    bool _is_complete;
};
}
}
#endif