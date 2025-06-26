#pragma once
#include<iostream>
#include<string>
#include<vector>
#include"Log.hpp"
using namespace log_ns;
const static std::string base_use="\r\r";
class HttpRequest
{
private:
    std::string _req_line;//请求行
    std::vector<std::string> _req_headers;//请求头
    std::string _blank_line;  //空行
    std::string _body_text;  //请求体
};

class HttpResponse
{

};

class HttpServer{
public:
    HttpServer()
    {

    }
    std::string HandlerHttpRequest(std::string &reqstr)
    {
        std::cout << "---------------------------------------" << std::endl;
        std::cout << reqstr;
        std::string responsestr = "HTTP/1.1 200 OK\r\n";
        responsestr += "Content-Type: text/html\r\n";
        responsestr += "\r\n";
        responsestr += "<html><h1>hello Linux, hello bite!</h1></html>";
        return responsestr;
     }
    

};