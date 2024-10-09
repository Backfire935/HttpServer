#pragma once
#include "IDefine.h"

namespace http
{
	class HttpSevrer
	{
	private:
		int					m_ConnectCount; //��������
		Socket				m_Listenfd;			//����socket
		std::mutex		m_Mutex;				//������
		std::mutex		m_ConnectMutex;//���ӻ�����
		std::list<int>		m_Socketfds;		//�µ���������
		std::condition_variable				m_Condition;//��������
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

