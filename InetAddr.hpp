#pragma once

// 必须先定义这些宏再包含任何Windows头文件
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
// Linux headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <iostream>
#include <cstring>
#include <string>

class InetAddr
{
private:
    sockaddr_in _addr;
    std::string _ip;
    int16_t _port;

public:
    InetAddr()
    {}
    InetAddr(sockaddr_in addr) : _addr(addr)
    {
        char ip_buf[32];
        ::inet_ntop(AF_INET,&addr.sin_addr,ip_buf,sizeof ip_buf);
        //_ip = inet_ntoa(addr.sin_addr);
        _ip=std::string(ip_buf);
        _port = ntohs(addr.sin_port);
    }

    bool operator==(const InetAddr& other) const
    {
        return _ip == other._ip && _port == other._port;
    }

    std::string get_ip() const
    {
        return _ip; 
    }

    int16_t get_port() const
    {
        return _port;
    }

    sockaddr_in get_addr() const
    {
        return _addr;
    }

    std::string AddrStr() const
    {
        return _ip + ":" + std::to_string(_port);
    }
};

