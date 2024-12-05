#pragma once

#define DEBUG_HTTP 0

#ifdef  ____WIN32_
#include<WinSock2.h>
#include<WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using Socket = SOCKET;
#else
#include<sys/socket.h>
#include<sys/epoll.h>
#include<netinet/tcp.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<cstring>
using Socket = int;
#endif //  ____WIN32_

#include<thread>
#include<mutex>
#include<list>
#include<map>
#include<vector>
#include<condition_variable>

#include<fstream>
#include<iostream>
#include<string>
#include"LogFile.hpp"

#define MAX_EXE_LEN						200
#define MAX_KEEP_ALIVE				3005
#define MAX_THREAD_COUNT		8
#define MAX_PACKAGE_LENGTH		1024//包最大长度
#define MAX_POST_LENGTH			1024*256//最大上传长度
#define MAX_BUF							1024*1024*1000//缓冲区大小 1000MB 服务器上提供给客户端下载的文件大小需要小于该值
#define MAX_ONES_BUF					1024*1024*4//临时缓存 4MB 客户端想要上传的文件大小需要小于该值

#ifndef S_ISREG
#define S_ISREG(m) (((m)&S_IFREG) == S_IFREG)
#endif

namespace http
{
	extern std::string deleteString(std::string s, char c);//删除字符串中的某一个字符

	enum E_RECV_STATE//接收数据状态
	{
		ER_FREE = 0X00,
		ER_HEAD = 0X01,//消息头
		ER_OVER = 0X02,//读取完毕
		ER_ERROR = 0X33
	};

	enum E_SEND_STATE//发送数据状态
	{
		ES_FREE = 0X00,
		ES_SENDING = 0X01,
		ES_OVER = 0X22
	};

	enum E_CONNECT_STATE//连接状态
	{
		 EC_FREE = 0X00,
		 EC_CONNECT = 0X01,
	};

#pragma pack(push, packing)//设置单字节对齐
#pragma pack(1)
	//请求和响应数据结构体
	struct S_HTTP_BASE
	{
		int state = ER_FREE;
		char buf[MAX_BUF];//消息体
		char tempBuf[MAX_ONES_BUF];//组装消息体的临时缓冲区
		std::string temp_str;
		int pos_head;
		int pos_tail;
		int threadid;

		std::string method;//方法
		std::string url;//统一资源定位符
		std::string version;//版本号
		std::map<std::string, std::string> head;//头
		int Content_length;//消息体内容长度
		std::string Content_Type;//消息类型
		std::string Connection;//设置连接

		//响应数据
		int status;
		std::string describe;//描述

		inline void Reset()
		{
			memset(buf, 0, MAX_BUF);
			memset(tempBuf, 0, MAX_ONES_BUF);
			state = 0;
			status = -1;
			pos_head = 0;
			pos_tail = 0;
			method = "";
			url = "";
			version = "";
			Content_length = 0;
			Content_Type = "";
			Connection = "";
			head.clear();
			threadid = 0; 
		}

		inline void Init()
		{
			memset(tempBuf, 0, MAX_ONES_BUF);
			state = 0;
			status = 0;
			method = "";
			url = "";
			version = "";
			Content_length = 0;
			Content_Type = "";
			Connection = "";
			head.clear();
		}

		//设置请求行
		inline void SetRequestLine(std::vector<std::string>& line)
		{
			if (line.size() != 3) return;
			method = line[0];
			url = line[1];
			version = line[2];
		}

		//设置头
		inline void SetHeader(std::string key, std::string value)
		{
			auto it = head.find(key);
			if (it != head.end())//如果找到了key就删除掉,因为红黑树中key不会覆盖
			{
				head.erase(it);
			}
			//std::make_pair 可以接受两个参数，并直接构造一个 std::pair 对象，这样可以避免创建不必要的临时对象。
			head.insert(std::make_pair(key, value));
		}
		//获取头
		inline std::string GetHeader(std::string key)
		{
			auto it = head.find(key);
			if (it != head.end())
			{
				return it->second;
			}
			return "";
		}


		//设置消息体长度
		inline void SetContentLength(std::string value)
		{
			auto s = deleteString(value, ' ');//删除其中的空字符
			int length = atoi(s.c_str());//获取剩余字符长度
			Content_length = length;
		}
		//设置消息类型
		inline void SetContentType(std::string value)
		{
			auto s = deleteString(value, ' ');
			Content_Type = s;
		}
		//设置连接
		inline void SetConnection(std::string value)
		{
			auto s = deleteString(value, ' ');
			Connection = s;
		}

		//设置响应行
		inline void SetResponseLine(int stat, std::string s)
		{
			version = "HTTP/1.1";
			status = stat;
			describe = s;
		}

 	};
#pragma pack(pop,packing)

	void log_UpdateConnect(int a, int b);
	std::vector<std::string> split(std::string srcStr, std::string pattern, bool isadd = false);
	std::vector<std::string> split2(std::string srcStr, std::string pattern);

	bool is_file(const std::string& path);
	void read_file(const std::string& path, std::string& out);
	bool read_Quest(const std::string& path, std::string& out);
	void initPath(); 
	int select_isread(Socket socketfd, int timeout_s, int timeout_u);
}
  

