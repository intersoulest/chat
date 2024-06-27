#include "const.h"
class HttpConnection:public std::enable_shared_from_this<HttpConnection>{
    friend class LogicSystem;
public:
    HttpConnection(tcp::socket socket);
    void Start();

};