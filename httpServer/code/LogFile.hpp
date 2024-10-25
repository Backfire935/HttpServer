#ifdef ____WIN32_
#include<windows.h>
#elif _WIN32 
#include<windows.h>
#endif
#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <string>
#include <atomic>

class Logger {
public:
    Logger(const std::string& filename) : logFile(filename), exitFlag(false) {
        logThread = std::thread(&Logger::processLogs, this);
        //logThread.detach();
    }

    ~Logger() {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            exitFlag = true;
        }
        condVar.notify_all();
        logThread.join();
    }


    template<typename... Args>
    std::string formatString(const std::string& fmt, Args&&... args) {
        std::ostringstream stream;
        using expander = int[];
        (void)expander {
            0, (void(stream << args), 0)...
        };
        return stream.str();
    }

    template<typename... Args>
    void  logMessage(const std::string& fmt, Args&&... args)
    {
        {
            std::string message = formatString(fmt, std::forward<Args>(args)...);
            std::lock_guard<std::mutex> lock(queueMutex);
            logQueue.push(message);
        }
        condVar.notify_one();
    }

private:
    // 获取可执行文件的路径
    std::string getExecutablePath() {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        return std::string(buffer);
    }

    void processLogs() {
        std::string exePath = getExecutablePath();
        std::string exeDir = exePath.substr(0, exePath.find_last_of("\\/")); // 获取可执行文件所在目录

        // 构造日志文件夹路径
        std::string logDir = exeDir +  "\\log";
        // 创建日志文件夹（如果不存在）
        CreateDirectoryA(logDir.c_str(), NULL);
        std::string logFilePath = logDir + "\\" + logFile;//文件夹路径加上文件名
        printf("%s\n", logFilePath.c_str());
        std::ofstream ofs(logFilePath, std::ios::app);
        if (!ofs.is_open()) {
            std::cerr << "Failed to open log file: " << logFilePath << std::endl;
            return;
        }

        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            condVar.wait(lock, [this] { return !logQueue.empty() || exitFlag; });

            while (!logQueue.empty()) {
                ofs << logQueue.front();
                ofs.flush();  // 强制刷新输出缓冲区
                logQueue.pop();
            }

            if (exitFlag) 
            {
                break;
            }
        }
    }

    std::string logFile;
    std::thread logThread;
    std::queue<std::string> logQueue;
    std::mutex queueMutex;
    std::condition_variable condVar;
    std::atomic<bool> exitFlag;
};

// 定义日志级别
#define LOG_NORMAL		1
#define LOG_WARNING	2
#define LOG_ERROR			3

//windows下设置cmd #define中注释和代码不能在一行
#ifdef ____WIN32_
#define LOG_MSG(level,fmt, ...)  {\
 /* 获取当前时间点 */  \
        auto now = std::chrono::system_clock::now();\
        /* 转换为 time_t 类型 */ \
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);\
        /* 转换为 tm 结构体 */ \
        struct std::tm now_tm = *std::localtime(&now_c);\
        auto since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());\
        /* 获取毫秒 */ \
        auto millis = since_epoch.count() % 1000;\
        std::string LogTime = std::to_string(now_tm.tm_year + 1900) + "/" + \
                              std::to_string(now_tm.tm_mon + 1) + "/" + \
                              std::to_string(now_tm.tm_mday) + " " + \
                              std::to_string(now_tm.tm_hour) + ":" + \
                              std::to_string(now_tm.tm_min) + ":" + \
                              std::to_string(now_tm.tm_sec) + ":" + \
                              std::to_string(millis) + ". "; \
        std::string filename = std::to_string(now_tm.tm_year + 1900) + \
                              std::to_string(now_tm.tm_mon + 1) +  \
                              std::to_string(now_tm.tm_mday) + ".log";\
        static Logger logger(filename);\
        HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE); \
        std::string LogLevelString; \
        if (level == LOG_ERROR) {\
            SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_RED);\
            LogLevelString = " Error ";\
        } else if (level == LOG_WARNING) {\
            SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_BLUE);\
            LogLevelString = " Warn ";\
        } else {\
            SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);\
            LogLevelString = " Normal ";\
        }\
        printf("%s", LogTime.c_str()); \
        printf("%s",LogLevelString.c_str());\
        /* 格式化 fmt 字符串 */ \
      /* 动态分配 buffer 而不是使用固定大小的数组 */ \
    /* 初始化一个大小为1024的vector<char> */ \
    std::vector<char> buffer(1024);   \
    int neededSize = std::snprintf(buffer.data(), buffer.size(), fmt, __VA_ARGS__); \
    if (neededSize >= static_cast<int>(buffer.size())) { \
        /* 如果需要更大的空间，重新调整buffer大小 */\
        buffer.resize(neededSize + 1);  \
        std::snprintf(buffer.data(), buffer.size(), fmt, __VA_ARGS__); \
    } \
        printf("%s", buffer.data()); \
        logger.logMessage("%s%s%s", LogTime.c_str(),LogLevelString.c_str(), std::string(buffer.data())); \
    }
#else
printf(fmt, __VA_ARGS__);
#endif
