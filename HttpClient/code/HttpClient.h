#pragma once

#include "IDefine.h"

namespace http
{

	struct S_TEST_BASE
	{
		char buf[1024*256];
		int len;
	};

	class HttpClient
	{
	public:
		HttpClient();
		~HttpClient();

		int id;//压力测试线程输出的ID
		std::mutex m_Mutex;//互斥量
		std::condition_variable m_Condition;//条件变量,用来唤醒线程
		S_HTTP_BASE* m_Request;//请求数据-发送给服务器
		S_HTTP_BASE* m_Response;//响应数据-接收服务器返回的数据

		void writeData(S_TEST_BASE* data);
		void pushRequest(std::string method, std::string url, int type, const char* c, const int len);
	private:
		int state;//客户端连接状态
		int socketfd;//客户端创建的socketfd
		std::shared_ptr<std::thread> m_Thread;
		std::list<S_TEST_BASE*> m_HttpDatas;//发送请求的数据列表

		void InitSocket();
		void ConnectServer();
		static void run(HttpClient* c);
	};

	extern std::string serverIp;
	extern int serverport;
}