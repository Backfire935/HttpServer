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

		int id;//ѹ�������߳������ID
		std::mutex m_Mutex;//������
		std::condition_variable m_Condition;//��������,���������߳�
		S_HTTP_BASE* m_Request;//��������-���͸�������
		S_HTTP_BASE* m_Response;//��Ӧ����-���շ��������ص�����

		void writeData(S_TEST_BASE* data);
		void pushRequest(std::string method, std::string url, int type, const char* c, const int len);
	private:
		int state;//�ͻ�������״̬
		int socketfd;//�ͻ��˴�����socketfd
		std::shared_ptr<std::thread> m_Thread;
		std::list<S_TEST_BASE*> m_HttpDatas;//��������������б�

		void InitSocket();
		void ConnectServer();
		static void run(HttpClient* c);
	};

	extern std::string serverIp;
	extern int serverport;
}