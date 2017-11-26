#include "parser.h"
#include <iostream>

using namespace AsyncSend::http;

static int empty_cb (http_parser *p){
    return 0;
}
static int empty_data_cb (http_parser *p, const char *buf, size_t len){
    return 0;
}

parser::parser(Buffer* buf):_buf(buf),_body(),_is_complete(false)
{
    http_parser_init(&_parser,HTTP_RESPONSE);
    _parser.data = this;
    _option = http_parser_settings {
        on_message_begin: empty_cb,
        on_url: empty_data_cb,
        on_header_field: empty_data_cb,
        on_header_value: empty_data_cb,
        on_headers_complete: &parser::on_headers_complete,
        on_body: &parser::on_body_complete,
        on_message_complete: &parser::on_message_end,
        on_reason: empty_data_cb,
        on_chunk_header: empty_cb,
        on_chunk_complete: empty_cb
    };
}

void parser::execute(){
    http_parser_execute(&_parser,&_option,_buf->peek(),_buf->readableBytes());
}

int parser::on_body_complete(http_parser* p, const char *at, size_t length){
    auto self = static_cast<parser*>(p->data);
    self->_body.append(at,length);
    self->_length += length;
    // char s[1024];
    // char content[length+1];
    // strncpy(content,at,length);
    // content[length] = '\0';
    // sprintf(s,"on_body_complete:%d,\r\n%s\r\n",length,content);
    // std::cout<<s;
    self->_is_complete = true;
    return 0;
} 

int parser::on_headers_complete(http_parser* p, const char *at, size_t length){
    auto self = static_cast<parser*>(p->data);
    self->_length += length;
    // char s[1024];
    // sprintf(s,"on_headers_complete:%d\r\n",length);
    // std::cout<<s;    
    return 0;
}

int parser::on_message_end(http_parser* p){
    auto self = static_cast<parser*>(p->data);
    self->_is_complete = true;
    //std::cout<<"message end\r\n";
}


