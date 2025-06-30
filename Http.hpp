#pragma once
#include<iostream>
#include<string>
#include<vector>
#include <string.h>
#include<unordered_map>
#include<cstring>
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
const static std::string suffixsep=".";
const static std::string arg_sep="?";

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


        if(_stricmp(_method.c_str(),"GET")==0)
        {
            auto pos=_url.find(arg_sep);
            if(pos!=std::string::npos)
            {
                _body_text+=_url.substr(pos+arg_sep.size());
                _url.resize(pos);
            }
        }

        _path+=_url;
        if(_path.back()!='/')
        {
            _path+='/';
        }
         _path+=home_path;
        std::cout<<"path: "<<_path<<std::endl;
        //获取资源后缀
         auto pos=_path.rfind(suffixsep);
         if(pos!=std::string::npos)
         {
            _suffix=_path.substr(pos);
         }
         else
         {
            _suffix=".default";
         }

    }

    std::string GetSuffix()
    {
        return _suffix;
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
            _body_text+=reqstr;
        }
        //进一步反序列化
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
    std::string _suffix;//资源后缀
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
        _mime_type[".html"]="text/html";
        _mime_type[".jpg"]="image/jpeg";
        _mime_type[".png"]="image/png";
        _mime_type[".css"]="text/css";
        _mime_type[".js"]="application/javascript";
        _mime_type[".ico"]="image/x-icon";
        _mime_type[".txt"]="text/plain";
        _mime_type[".mp4"]="video/mp4";
        _mime_type[".default"]="text/html";


        _status_code_msg["200"]="OK";
        _status_code_msg["404"]="Not Found";
        _status_code_msg["500"]="Internal Server Error";
        _status_code_msg["400"]="Bad Request";
        _status_code_msg["403"]="Forbidden";
        _status_code_msg["405"]="Method Not Allowed";
        _status_code_msg["408"]="Request Timeout";
        _status_code_msg["414"]="Request-URI Too Long";
        _status_code_msg["501"]="Not Implemented";
        _status_code_msg["502"]="Bad Gateway";
         
    }
    std::string GetFileContent(const std::string &path)
    {
        // std::ifstream ifs(path.c_str());
        // if(!ifs.is_open())
        // {
        //     return "";
        // }
        // std::ostringstream oss;
        // oss << ifs.rdbuf();
        // return oss.str();

        std::ifstream in(path,std::ios::binary);
        if(!in.is_open()) return std::string();
        in.seekg(0,in.end);
        int filesize = in.tellg();
        in.seekg(0,in.beg);

        std::string content;
        content.resize(filesize);
        in.read((char*)content.data(),filesize);
        in.close();
        return content;
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
        if(content.empty())
        {
            resp.AddCode(std::string("404"),_status_code_msg["404"]);
        }
        else
        {
            resp.AddCode(std::string("200"),_status_code_msg["200"]);
        }
        //添加Content-Type
        resp.AddHeader(std::string("Content-Type"),_mime_type[req.GetSuffix()]);
        //添加报文长度
        resp.AddHeader(std::string("Content-Length"),std::to_string(content.size()));
        resp.AddBody(content);
        return resp.Serialize();
#endif
    }
    

    ~HttpServer(){}
private:
    std::unordered_map<std::string,std::string> _mime_type;

    std::unordered_map<std::string,std::string> _status_code_msg;

};