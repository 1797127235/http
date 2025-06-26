#pragma once
#include <iostream>
#include <ctime>
#include <cstdarg>
#include <fstream>
#include <cstring>
#include <string>


#ifdef _WIN32
// 防止 windows.h 包含 winsock.h，避免与 winsock2.h 冲突
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
// 防止包含 winsock.h
#define _WINSOCKAPI_
#include <windows.h>
#undef _WINSOCKAPI_  // 取消定义，允许后续正确包含
#include <process.h>
#include <memory>
#else
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#endif
namespace log_ns
{
    // 使用宏定义代替枚举以避免MSVC编译器问题
    #define DEBUG 1
    #define INFO 2
    #define WARNING 3
    #define ERROR_ 4
    #define FATAL 5

    std::string LevelToString(int level)
    {
        switch (level)
        {
        case 1:  // DEBUG
            return "DEBUG";
        case 2:  // INFO
            return "INFO";
        case 3:  // WARNING
            return "WARNING";
        case 4:  // ERROR_
            return "ERROR";
        case 5:  // FATAL
            return "FATAL";
        default:
            return "UNKNOWN";
        }
    }

    std::string GetCurrTime()
    {
        time_t now = time(nullptr);
        struct tm curr_time;
        
#ifdef _WIN32
        localtime_s(&curr_time, &now);
#else
        struct tm *tm_ptr = localtime(&now);
        curr_time = *tm_ptr;
#endif
        
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "%d-%02d-%02d %02d:%02d:%02d",
                 curr_time.tm_year + 1900,
                 curr_time.tm_mon + 1,
                 curr_time.tm_mday,
                 curr_time.tm_hour,
                 curr_time.tm_min,
                 curr_time.tm_sec);
        return buffer;
    }

    // 获取进程ID的跨平台函数
    int GetProcessId()
    {
#ifdef _WIN32
        return _getpid();
#else
        return getpid();
#endif
    }

    class logmessage
    {
    public:
        std::string _level;
        int _id;
        std::string _filename;
        int _filenumber;
        std::string _curr_time;
        std::string _message_info;
    };

#define SCREEN_TYPE 1
#define FILE_TYPE 2

    const std::string glogfile = "./log.txt";
    
    // 跨平台互斥锁
#ifdef _WIN32
    static CRITICAL_SECTION glock;
    static bool glock_initialized = false;
    
    void InitializeLock() {
        if (!glock_initialized) {
            InitializeCriticalSection(&glock);
            glock_initialized = true;
        }
    }
    
    void CleanupLock() {
        if (glock_initialized) {
            DeleteCriticalSection(&glock);
            glock_initialized = false;
        }
    }
#else
    static pthread_mutex_t glock = PTHREAD_MUTEX_INITIALIZER;
    
    // Linux 下的 LockGuard 类
    class LockGuard {
    public:
        explicit LockGuard(pthread_mutex_t* mutex) : mutex_(mutex) {
            pthread_mutex_lock(mutex_);
        }
        
        ~LockGuard() {
            pthread_mutex_unlock(mutex_);
        }
        
    private:
        pthread_mutex_t* mutex_;
        LockGuard(const LockGuard&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;
    };
#endif

    class Log
    {
    public:
        Log(const std::string &logfile = glogfile) : _logfile(logfile), _type(SCREEN_TYPE)
        {
#ifdef _WIN32
            InitializeLock();
#endif
        }
        
        void Enable(int type)
        {
            _type = type;
        }
        
        void FlushLogToScreen(const logmessage &lg)
        {
            printf("[%s][%d][%s][%d][%s] %s\n",
                   lg._level.c_str(),
                   lg._id,
                   lg._filename.c_str(),
                   lg._filenumber,
                   lg._curr_time.c_str(),
                   lg._message_info.c_str());
        }
        
        void FlushLogToFile(const logmessage &lg)
        {
            std::ofstream out(_logfile, std::ios::app);
            if (!out.is_open())
                return;
            char logtxt[2048];
            snprintf(logtxt, sizeof(logtxt), "[%s][%d][%s][%d][%s] %s\n",
                     lg._level.c_str(),
                     lg._id,
                     lg._filename.c_str(),
                     lg._filenumber,
                     lg._curr_time.c_str(),
                     lg._message_info.c_str());
            out.write(logtxt, strlen(logtxt));
            out.close();
        }
        
        void FlushLog(const logmessage &lg)
        {
            // 跨平台锁
#ifdef _WIN32
            EnterCriticalSection(&glock);
#else
            LockGuard lockguard(&glock);
#endif
            
            switch (_type)
            {
            case SCREEN_TYPE:
                FlushLogToScreen(lg);
                break;
            case FILE_TYPE:
                FlushLogToFile(lg);
                break;
            }
            
#ifdef _WIN32
            LeaveCriticalSection(&glock);
#endif
        }
        
        void logMessage(std::string filename, int filenumber, int level, const char *format, ...)
        {
            logmessage lg;

            lg._level = LevelToString(level);
            lg._id = GetProcessId();
            lg._filename = filename;
            lg._filenumber = filenumber;
            lg._curr_time = GetCurrTime();

            va_list ap;
            va_start(ap, format);
            char log_info[1024];
            vsnprintf(log_info, sizeof(log_info), format, ap);
            va_end(ap);
            lg._message_info = log_info;

            // 打印出来日志
            FlushLog(lg);
        }
        
        ~Log()
        {
#ifdef _WIN32
            CleanupLock();
#endif
        }

    private:
        int _type;
        std::string _logfile;
    };

    static Log lg;

#define LOG(Level, Format, ...)                                        \
    do                                                                 \
    {                                                                  \
        lg.logMessage(__FILE__, __LINE__, Level, Format, ##__VA_ARGS__); \
    } while (0)
    
#define EnableScreen()          \
    do                          \
    {                           \
        lg.Enable(SCREEN_TYPE); \
    } while (0)
    
#define EnableFILE()          \
    do                        \
    {                         \
        lg.Enable(FILE_TYPE); \
    } while (0)
}