#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<sstream>
#include<fstream>
#include"Log.hpp"
//#define TEST
using namespace log_ns;
const static std::string base_sep="\r\n";
const static std::string base_key=": ";
const static std::string prefixpath="wwwroot";
const static std::string home_path="index.html";
const static std::string http_version="HTTP/1.0";
class HttpRequest
{
public:
    HttpRequest():_blank_line(base_sep),_path(prefixpath)
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
    void ParseReqLine()//解析请求行
    {
        std::stringstream ss(_req_line);
        ss>>_method>>_url>>_version;

        _path+=_url;
        if(_path.back()!='/')
        {
            _path+='/';
        }
         _path+=home_path;

    }
    void ParseReqHeaders()//解析请求头
    {
        for(auto &header:_req_headers)
        {
            auto pos=header.find(base_key);
            if(pos==std::string::npos)
            {
                continue;
            }
            std::string key=header.substr(0,pos);
            std::string value=header.substr(pos+base_key.size());
            if(key.empty()||value.empty())
            {
                continue;
            }
            _headers_kv[key]=value;
        }
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
        ParseReqLine();
        ParseReqHeaders();
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

        std::cout<<"method:"<<_method<<std::endl;
        std::cout<<"url:"<<_url<<std::endl;
        std::cout<<"version:"<<_version<<std::endl;

    }
    std::string Getpath()
    {
        return _path;
    }
private:
    std::string _req_line;//请求行
    std::vector<std::string> _req_headers;//请求头
    std::string _blank_line;  //空行
    std::string _body_text;  //请求体

    //进一步反序列化更具体的属性字段
    std::string _method;
    std::string _url;
    std::string _version;
    std::string _path;
    std::unordered_map<std::string,std::string> _headers_kv;
};

//响应报文
class HttpResponse
{
public:
    HttpResponse():_blank_line(base_sep),_http_version(http_version)
    {}
    ~HttpResponse(){}
    std::string Serialize()
    {
        std::stringstream ss;
        ss<<_http_version<<" "<<_status_code<<" "<<_status_desc<<base_sep;
        for(auto &header:_headers_kv)
        {
            ss<<header.first<<":"<<header.second<<base_sep;
        }
        ss<<base_sep<<_resp_body_text;
        return ss.str();
    }
    void AddCode(std::string code,std::string &desc)
    {
        _status_code=code;
        _status_desc=desc;
    }
    void AddHeader(std::string &key,std::string &value)
    {
        _headers_kv[key]=value;
    }
    void AddBody(std::string &body)
    {
        _resp_body_text=body;
    }
private:
    std::string _status_line;//状态行
    std::string _status_code;
    std::string _status_desc;
    std::unordered_map<std::string,std::string> _headers_kv;
    std::string _blank_line;

    std::string _http_version;
    std::string _status_msg;
    std::string _content_type;
    std::string _resp_body_text;
};


class HttpServer{
public:
    HttpServer()
    {

    }
    std::string GetFileContent(std::string &path)
    {
        std::ifstream ifs(path.c_str());
        if(!ifs.is_open())
        {
            return "";
        }
        std::ostringstream oss;
        oss << ifs.rdbuf();
        return oss.str();
        
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
        std::cout<<"responsestr: "<<responsestr.size()<<std::endl; 
        return responsestr;
#else
        HttpRequest req;
        req.Deserialization(reqstr);
        HttpResponse resp;
        std::string content=GetFileContent(req.Getpath());
        resp.AddCode(std::string("200"),std::string("OK"));
        //resp.AddHeader(std::string("Content-Type"), std::string("text/html"));
        resp.AddHeader(std::string("Content-Length"),std::to_string(content.size()));
        resp.AddBody(content);
        return resp.Serialize();
#endif
    }
    

    ~HttpServer(){}

};