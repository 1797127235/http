#include "ServerMain.hpp"
#include "Http.hpp"
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    int16_t port = atoi(argv[1]);
    HttpServer http_server;
    auto func=std::bind(&HttpServer::HandlerHttpRequest,&http_server,std::placeholders::_1);
    std::unique_ptr<TcpServer> tsvr=std::make_unique<TcpServer>(port,func);
    tsvr->loop();
    return 0;
}