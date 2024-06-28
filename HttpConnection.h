#include "const.h"
class HttpConnection:public std::enable_shared_from_this<HttpConnection>{
    friend class LogicSystem;
public:
    HttpConnection(tcp::socket socket);
    void Start();
private:
    void CheckDeadline();
    void WriteResponse();
    void HandleReq();

    tcp::socket _socket;
    // 用来接受数据
    beast::flat_buffer  _buffer{ 8192 };
    // 用来解析请求
    http::request<http::dynamic_body> _request;
    // 用来回应客户端
    http::response<http::dynamic_body> _response;
    // 用来做定时器判断请求是否超时
    net::steady_timer deadline_{
        _socket.get_executor(), std::chrono::seconds(60) };
};