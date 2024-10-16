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

		int recvSocket(Socket socketfd, S_HTTP_BASE* quest);
		int analyData(Socket socketfd, S_HTTP_BASE* quest, S_HTTP_BASE* response);//������Ϣͷ
		int readBody(Socket socketfd, S_HTTP_BASE* quest, S_HTTP_BASE* response);//������Ϣ��
		void writeData(S_HTTP_BASE* request, S_HTTP_BASE* response, const char* body, int size);
	public:
		HttpSevrer();
		virtual ~HttpSevrer();

		int InitSocket();
		void runAccept();
		void runServer();
		void runThread();
		void runSocket(Socket socketfd, int id);


		static void run(HttpSevrer* serverInstance, int threadId);

	};
}

