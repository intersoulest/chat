#include "HttpConnection.h"
#include "LogicSystem.h"
#include <iostream>
HttpConnection::HttpConnection(tcp::socket socket)
    :_socket(std::move(socket)){  
    }

void HttpConnection::Start(){
    auto self = shared_from_this();
    //第一个参数为异步可读的数据流，大家可以理解为socket.
    //第二个参数为一个buffer，用来存储接受的数据，因为http可接受文本，图像，音频等多种资源文件，所以是Dynamic动态类型的buffer
    //第三个参数是请求参数，我们一般也要传递能接受多种资源类型的请求参数。
    //第四个参数为回调函数，接受成功或者失败，都会触发回调函数，我们用lambda表达式就可以了
    http::async_read(_socket,_buffer,_request,[self](beast::error_code ec,
    std::size_t bytes_transferred){
        try
        {
            if(ec){
                std::cout <<"http read err is" << ec.message() << std::endl;
                return;
            }
                    //处理读到的数据
        boost::ignore_unused(bytes_transferred);
        self->HandleReq();
        self->CheckDeadline();
        }

        catch(std::exception& exp)
        {
            std::cout << "exception is " << exp.what() << std::endl;
        }
        

    });
}
void HttpConnection::HandleReq() {
    //设置版本
    _response.version(_request.version());
    //设置为短链接
    _response.keep_alive(false);
    if (_request.method() == http::verb::get) {
        bool success = LogicSystem::GetInstance()->HandleGet(_request.target().to_string(), shared_from_this());
        if (!success) {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "url not found\r\n";
            WriteResponse();
            return;
        }
        _response.result(http::status::ok);
        _response.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    }
}
//因为http是短链接，所以发送完数据后不需要再监听对方链接，直接断开发送端即可。
void HttpConnection::WriteResponse() {
    auto self = shared_from_this();
    _response.content_length(_response.body().size());
    http::async_write(
        _socket,
        _response,
        [self](beast::error_code ec, std::size_t)
        {
            self->_socket.shutdown(tcp::socket::shutdown_send, ec);//关闭发送端
            self->deadline_.cancel();
        });
}
void HttpConnection::CheckDeadline() {
    auto self = shared_from_this();
    deadline_.async_wait(
        [self](beast::error_code ec)
        {
            if (!ec)
            {
                // Close socket to cancel any outstanding operation.
                self->_socket.close(ec);
            }
        });
}