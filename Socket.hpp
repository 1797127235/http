#pragma once

// 必须先定义这些宏再包含任何Windows头文件
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
// 防止包含 winsock.h
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
// 自动链接 ws2_32 库
#pragma comment(lib, "ws2_32.lib")
#endif

#include"Log.hpp"
#include"InetAddr.hpp"
#include <stdexcept>
#include<functional>
#include<memory>

namespace socket_ns
{
    using namespace log_ns;
    // 为了避免在成员函数中使用作用域限定符的问题，直接引入枚举值
    const int LOG_DEBUG = 1;      // LogLevel::DEBUG
    const int LOG_INFO = 2;       // LogLevel::INFO 
    const int LOG_WARNING = 3;    // LogLevel::WARNING
    const int LOG_ERROR = 4;      // LogLevel::ERROR_
    const int LOG_FATAL = 5;      // LogLevel::FATAL
    class Socket;
    using SockPtr = std::shared_ptr<Socket>;
    using SockSPtr = std::shared_ptr<Socket>;
    //默认TCP监听队列长度
    const static int gbacklog = 8;


    // Windows Socket 初始化管理类
    class WinSocketInit
    {
    private:
        static bool s_initialized;
        static int s_refCount;
        
    public:
        static bool Initialize()
        {
            if (!s_initialized)
            {
                WSADATA wsaData;
                int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
                if (result != 0)// 初始化失败
                {
                    return false;
                }
                s_initialized = true;
            }
            s_refCount++;
            return true;
        }
        
        static void Cleanup()
        {
            s_refCount--;
            if (s_refCount <= 0 && s_initialized)
            {
                WSACleanup();
                s_initialized = false;
                s_refCount = 0;
            }
        }
    };



    //模板方法模式


    class Socket
    {
    public:
        virtual void CreateSocketOrDie() = 0;
        virtual void CreateBindOrDie(int16_t port) = 0;
        virtual void CreateListenOrDie(int backlog = gbacklog) = 0;
        virtual SockPtr Accepter(InetAddr *clientaddr) = 0;
        virtual bool Connector(const std::string &ip, int16_t port) = 0;
        virtual int Send(const std::string & in) = 0;
        virtual int Recv(std::string * out) = 0;

        virtual void Close() = 0;
        virtual SOCKET GetSockfd() const = 0;
       

    public:
        void BuildingListenSocket(int16_t port, int backlog = gbacklog)
        {
            CreateSocketOrDie();
            CreateBindOrDie(port);
            CreateListenOrDie(backlog);
        }

        bool BuildClientSocket(const std::string &ip, int16_t port)
        {
            CreateSocketOrDie();
            if (!Connector(ip, port))
            {
                return false;
            }
            return true;
        }
    };


    class TcpSocket : public Socket
    {
    public:
        void CreateSocketOrDie() override
        {
            // 创建TCP套接字
            _sockfd= socket(AF_INET, SOCK_STREAM, 0);
            if (_sockfd == INVALID_SOCKET)
            {
                LOG(LOG_ERROR, "Failed to create TCP socket");
                throw std::runtime_error("Failed to create TCP socket");
            }
            LOG(LOG_INFO, "TCP socket created successfully, sockfd: %d", _sockfd);
        }
        void CreateBindOrDie(int16_t port) override
        {
            // 绑定套接字到地址和端口
            sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port);

            if(bind(_sockfd,(sockaddr*)&addr,sizeof addr) == SOCKET_ERROR)
            {
                LOG(LOG_ERROR, "Failed to bind TCP socket to port %d", port);
                throw std::runtime_error("Failed to bind TCP socket");
            }
            LOG(LOG_INFO, "TCP socket bound to port %d successfully", port);
        }
        void CreateListenOrDie(int backlog) override
        {
            if (listen(_sockfd, backlog) == SOCKET_ERROR)
            {
                LOG(LOG_ERROR, "Failed to listen on TCP socket");
                throw std::runtime_error("Failed to listen on TCP socket");
            }
            LOG(LOG_INFO, "TCP socket is now listening, sockfd: %d", _sockfd);
        }
        // 接受连接,同时返回一个新的TcpSocket实例
        // clientaddr用于存储客户端的地址信息
        SockPtr Accepter(InetAddr *clientaddr) override
        {
            if(clientaddr == nullptr)
            {
                LOG(LOG_ERROR, "Client address pointer is null");
                return nullptr; // 返回空指针表示参数错误
            }
            // 接受连接
            sockaddr_in client_addr;
            int addrlen= sizeof client_addr;
            SOCKET client_sockfd = ::accept(_sockfd, (sockaddr*)&client_addr, &addrlen);
            if (client_sockfd == INVALID_SOCKET)
            {
                LOG(LOG_ERROR, "Failed to accept connection on TCP socket");
                return nullptr; // 返回空指针表示接受连接失败
            }
            LOG(LOG_INFO, "Accepted connection on TCP socket, client sockfd: %d", client_sockfd);

            *clientaddr= InetAddr(client_addr);
            // 返回一个新的TcpSocket实例，表示已接受的连接
            return std::make_shared<TcpSocket>(client_sockfd);
        }
        // 连接到服务器
        bool Connector(const std::string &ip, int16_t port) override
        {
            // 创建服务器地址结构
            sockaddr_in server_addr;
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0)
            {
                LOG(LOG_ERROR, "Invalid IP address: %s", ip.c_str());
                return false;
            }
            // 连接到服务器
            if (connect(_sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
            {
                LOG(LOG_ERROR, "Failed to connect to server %s:%d", ip.c_str(), port);
                return false;
            }
            LOG(LOG_INFO, "Connected to server %s:%d successfully", ip.c_str(), port);
            return true;
        }


        int Send(const std::string & in) override
        {
            // 发送数据
            int bytes_sent = send(_sockfd, in.c_str(), static_cast<int>(in.size()), 0);
            if (bytes_sent == SOCKET_ERROR)
            {
                LOG(LOG_ERROR, "Failed to send data on TCP socket");
                throw std::runtime_error("Failed to send data on TCP socket");
            }
            //LOG(INFO, "Sent %zu bytes on TCP socket", bytes_sent);
            return bytes_sent;
        }

        int Recv(std::string * out) override
        {
            // 接收数据 
            char buffer[4096]; // 缓冲区大小可以根据需要调整
            int bytes_received = recv(_sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received == SOCKET_ERROR)
            {
                LOG(LOG_ERROR, "Failed to receive data on TCP socket");
                throw std::runtime_error("Failed to receive data on TCP socket");
            }
            if (bytes_received == 0)
            {
                LOG(LOG_INFO, "Connection closed by peer");
                return 0; // 连接已关闭
            }
            buffer[bytes_received] = '\0'; // 添加字符串结束符
            *out += buffer; //todo 
            return bytes_received;
        }

        void Close() override
        {
            if (_sockfd != INVALID_SOCKET)
            {
                closesocket(_sockfd);
                LOG(LOG_INFO, "TCP socket closed, sockfd: %d", _sockfd);
                _sockfd = INVALID_SOCKET; // 重置套接字描述符
            }

        }

        SOCKET GetSockfd() const override
        {
            return _sockfd;
        }

        // SOCKET Sockfd() const override
        // {
        //     return _sockfd;
        // }

        TcpSocket() : _sockfd(INVALID_SOCKET)
        {
            if (!WinSocketInit::Initialize())
            {
                throw std::runtime_error("Failed to initialize Windows Socket");
            }
        }

        explicit TcpSocket(SOCKET sockfd) : _sockfd(sockfd)
        {
            if (!WinSocketInit::Initialize())
            {
                throw std::runtime_error("Failed to initialize Windows Socket");
            }
        }

        ~TcpSocket()
        {
            Close();
            WinSocketInit::Cleanup();
        }
    private:
        SOCKET _sockfd; // 套接字描述符
    };

    //静态成员变量初始化
    int WinSocketInit::s_refCount = 0;
    bool WinSocketInit::s_initialized = false;
}