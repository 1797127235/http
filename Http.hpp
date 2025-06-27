#pragma once
#include<iostream>
#include<string>
#include<vector>
#include"Log.hpp"
using namespace log_ns;
const static std::string base_sep="\r\r";
class HttpRequest
{
public:
    HttpRequest():_blank_line(base_sep)
    {}
    ~HttpRequest(){}
    std::string GetLine(std::string &reqstr)
    {
        auto pos=reqstr.find(base_sep);
        if(pos==std::string::npos)
        {
            return "";
        }
        std::string line=reqstr.substr(0,pos);
        if(line.empty())//读到空行
        {
            return base_sep;
        }
        reqstr.erase(0,pos+base_sep.size());
        return line;
    }
    void Deserialization(std::string &reqstr)
    {
        _req_line=GetLine(reqstr);
        std::string header;
        do{
            header=GetLine(reqstr);
            if(header.empty()) break;
            else if(header==base_sep) break;
            else{
                _req_headers.push_back(header);
            }
        }while(true);

        if(!reqstr.empty())
        {
            _body_text=reqstr;
        }
    }
    void Print()
    {
        std::cout<<"###"<<_req_line<<std::endl;
        for(auto &header:_req_headers)
        {
            std::cout<<"@@"<<header<<std::endl;
        }
        std::cout<<"***"<<_blank_line;
        std::cout<<"$$$"<<_body_text<<std::endl;
    }
private:
    std::string _req_line;//请求行
    std::vector<std::string> _req_headers;//请求头
    std::string _blank_line;  //空行
    std::string _body_text;  //请求体
};

class HttpResponse
{
private:
    std::string _status_line;//状态行
    std::vector<std::string> _resp_headers;
    std::string _blank_line;
    std::string _resp_body_text;
};

class HttpServer{
public:
    HttpServer()
    {

    }
    std::string HandlerHttpRequest(std::string &reqstr)
    {
#ifdef TEST
        std::cout << "---------------------------------------" << std::endl;
        std::cout << reqstr;
        std::string responsestr = "HTTP/1.1 200 OK\r\n";
        responsestr += "Content-Type: text/html\r\n";
        responsestr += "\r\n";
        responsestr += "<html><h1>hello Linux, hello bite!</h1></html>";
        return responsestr;
#else
        HttpRequest req;
        req.Deserialization(reqstr);
        req.Print();

        return "";
#endif
    }
    

    ~HttpServer(){}

};