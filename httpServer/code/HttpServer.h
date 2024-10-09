#pragma once
#include "IDefine.h"

namespace http
{
	class HttpSevrer
	{
	private:
		int					m_ConnectCount; //连接数量
		Socket				m_Listenfd;			//监听socket
		std::mutex		m_Mutex;				//互斥量
		std::mutex		m_ConnectMutex;//连接互斥量
		std::list<int>		m_Socketfds;		//新的连接链表
		std::condition_variable				m_Condition;//条件变量
		std::shared_ptr<std::thread>	m_Thread[MAX_THREAD_COUNT];

		S_HTTP_BASE* m_Request[MAX_THREAD_COUNT];
		S_HTTP_BASE* m_Response[MAX_THREAD_COUNT];
		//db::MySqlConnector* m_MySQl[MAX_THREAD_COUNT];

	public:
		HttpSevrer();
		virtual ~HttpSevrer();

		int InitSocket();
		void runServer();

	};
}

