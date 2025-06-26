#pragma once
#include"Socket.hpp"
#include<thread>
using namespace socket_ns;

using server_t = std::function<std::string(std::string &requeststr)>;
class TcpServer
{
public:
    TcpServer(int16_t port, server_t func):
        _listen_sock(std::make_shared<TcpSocket>()),
        _is_running(false),
        _func(func)
    {
        _listen_sock->BuildingListenSocket(port);
    }
    
    class ThreadData
    {
    public:
        SockPtr _sockfd;
        TcpServer *_self;
        InetAddr _addr;
    public:
        ThreadData(SockPtr sockfd, TcpServer *self, const InetAddr &addr):
            _sockfd(sockfd),
            _self(self),
             _addr(addr)
        {}
    };

    void loop()
    {
        _is_running = true;
        while(_is_running)
        {
            InetAddr peer_addr;
            SockPtr sockfd = _listen_sock->Accepter(&peer_addr);
            if(sockfd == nullptr)
            {
                continue;
            }

            std::thread t(std::bind(&TcpServer::Execute,this,sockfd,peer_addr));
            t.detach();
        }
    }
    void Execute(SockPtr sockfd, const InetAddr &addr)
    {
        std::string requeststr;
        int n=sockfd->Recv(&requeststr);
        if(n<=0)
        {
            sockfd->Close();
            return;
        }
        
        std::string response = _func(requeststr);
        sockfd->Send(response);

        sockfd->Close();
    }

private:
    SockPtr _listen_sock;
    bool _is_running;
    server_t _func;
};