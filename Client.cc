#include"Socket.hpp"
#include"ServerMain.hpp"
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    int16_t port = atoi(argv[1]);

    auto tsvr=std::make_unique<TcpServer>();
    
    while(true)
    {

    }

}