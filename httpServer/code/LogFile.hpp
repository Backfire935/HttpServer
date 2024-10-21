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
        logThread.detach();
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
    void processLogs() {
        std::ofstream ofs(logFile, std::ios::app);
        if (!ofs.is_open()) {
            std::cerr << "Failed to open log file: " << logFile << std::endl;
            return;
        }

        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            condVar.wait(lock, [this] { return !logQueue.empty() || exitFlag; });

            while (!logQueue.empty()) {
                ofs << logQueue.front() << std::endl;
                logQueue.pop();
            }

            if (exitFlag) break;
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

//windows下设置cmd
#ifdef ____WIN32_ 
#define LOG_MSG(level,fmt, ...)  {\
        static Logger logger("log.txt");\
		HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE); \
		if (level == LOG_ERROR) \
		{	\
		SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_RED);	\
		}	\
		else if (level == LOG_WARNING) \
		{	\
		SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_BLUE);	\
		}	\
		else \
		{\
		SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);	\
		}	\
		 /*获取当前时间点*/  \
		auto now = std::chrono::system_clock::now();\
		/*转换为time_t类型*/ \
		std::time_t now_c = std::chrono::system_clock::to_time_t(now);\
		/*转换为tm结构体*/ \
		struct std::tm now_tm = *std::localtime(&now_c);\
		auto since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());\
		/*获取毫秒*/ \
		auto millis = since_epoch.count() % 1000;\
		std::string LogTime = std::to_string(now_tm.tm_year + 1900) + "/" + \
                          std::to_string(now_tm.tm_mon + 1) + "/" + \
                          std::to_string(now_tm.tm_mday) + " " + \
                          std::to_string(now_tm.tm_hour) + ":" + \
                          std::to_string(now_tm.tm_min) + ":" + \
                          std::to_string(now_tm.tm_sec) + ":" + \
                          std::to_string(millis) + ". "; \
        printf("%s", LogTime.c_str()); \
        printf(fmt, __VA_ARGS__); \
        logger.logMessage(LogTime.c_str(),fmt, __VA_ARGS__);\
}
#else
printf(fmt, __VA_ARGS__);
#endif 
//printf("%s", LogTime.c_str()); \
//printf(fmt, __VA_ARGS__); \
//printf(LogTime, fmt, __VA_ARGS__); \