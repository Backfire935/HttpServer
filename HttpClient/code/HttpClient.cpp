#include "HttpClient.h"

namespace http
{

	std::string serverIp = "127.0.0.1";
	int serverport = 8080;

	HttpClient::HttpClient()
	{
	}
	HttpClient::~HttpClient()
	{
	}

	void HttpClient::InitSocket()
	{
		//����socket
		socketfd = -1;
		state = EC_FREE;

		m_Request = new S_HTTP_BASE();
		m_Response = new S_HTTP_BASE();
		m_Request->Reset();
		m_Response->Reset();

		WSADATA wsData;//��ʼ��WSA
		int errorcode = WSAStartup(MAKEWORD(2, 2), &wsData);
		if (errorcode != 0) return;

		//�����߳�
		m_Thread.reset(new std::thread(HttpClient::run, this));
		m_Thread->detach();

	}

	void HttpClient::ConnectServer()
	{
		if (state == EC_CONNECT)return;
		if (socketfd != -1) closesocket(socketfd);
		socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//����socket

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(serverport);
		addr.sin_addr.S_un.S_addr = inet_addr(serverIp.c_str());
		int ret = connect(socketfd, (sockaddr*)&addr, sizeof(addr));//���ӷ�����
		if (ret == -1)//����ʧ��
		{
			LOG_MSG(1,"connect failed ...%d\n",id);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			ConnectServer();
			return;
		}

		state = EC_CONNECT;
		//setNonblockingSocket(socketfd);//���÷�����
	}

	void HttpClient::run(HttpClient* c)//�̺߳���
	{
		
	}

}

