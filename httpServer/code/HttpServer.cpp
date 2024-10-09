#include "HttpServer.h"

namespace http 
{
	HttpSevrer::HttpSevrer()
	{

	}

	HttpSevrer::~HttpSevrer()
	{
#ifdef ____WIN32_
		if (m_Listenfd != INVALID_SOCKET)
		{
			closesocket(m_Listenfd);//���ڹر�һ���׽���
			m_Listenfd = INVALID_SOCKET;
		}
		WSACleanup();//���ڳ�������Ӧ�ó���� Winsock �ĳ�ʼ��
#else
		close(m_Listenfd);
#endif // ____WIN32_

	}

	bool setNonBlockingSocket(Socket socketfd)
	{
#ifdef  ____WIN32_
		unsigned long u1 = 1;
		int err = ioctlsocket(socketfd, FIONBIO, (unsigned long*)& u1);//��������Ϊ��������
		if (err == SOCKET_ERROR) return false;
		return true;
#else
		int flags = fcntl(socketfd, F_GETFL); 
		if (flags < 0) return false;
		flags |= O_NONBLOCK;

		if (fcntl(socketfd, F_GETFL, flags) < 0)//�����ļ�״̬��־
			return false;
		return true;
#endif // ! ____WIN32_

	}

	int HttpSevrer::InitSocket()
	{
#ifdef ____WIN32_ 
		//1.��ʼ��Windows sockets DLL
		WSADATA wsData;
		int errorcode = WSAStartup(MAKEWORD(2,2), &wsData);
		if (errorcode != 0)
		{
			LOG_MSG("WSAStartup error, status:%d\n", errorcode);
			return -1;
		}
#endif
		//2.��������socket
		m_Listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if (m_Listenfd < 0)
		{
			LOG_MSG("socket error, status:%d\n", m_Listenfd);
			return -2;
		}

		//3.����Nagle�㷨��û���ӳ�
		int value = 1;
		setsockopt(m_Listenfd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&value), sizeof(value));

		//4.���÷�����
		setNonBlockingSocket(m_Listenfd);

		//5.���ö˿ں��ظ���
		int flag = 1;
		int ret = setsockopt(m_Listenfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flag), sizeof(int));

		//6.��IP�˿�
		struct sockaddr_in serAddr;
		memset(&serAddr, 0, sizeof(sockaddr_in));

		serAddr.sin_family = AF_INET;//IPv4
		serAddr.sin_port = htons(8080);
#ifdef ____WIN32_
		serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
#else
		serAddr.sin_addr.S_addr = INADDR_ANY;
#endif // ____WIN32_
		int err = ::bind(m_Listenfd, (struct sockaddr*)&serAddr, sizeof(serAddr));//��
		if (err < 0)
		{
			LOG_MSG("bind error, status:%d\n", err);
			return -3;
		}

		//7.����
		//����ָ�����׽��ֿ����Ŷӵȴ����ܵ�������������������������п��õĶ��ж����������������������ܻᱻ�ܾ���SOMAXCONN ͨ�������ṩһ�����㹻�󡱵�ֵ����ȷ�����������оܾ���������
		err = listen(m_Listenfd, SOMAXCONN);
		if (err < 0)
		{
			LOG_MSG("listen error, status:%d\n", err);
			return -4;
		}

		return 0;
	}

	void HttpSevrer::runServer()
	{
		m_ConnectCount = 0;

		//1.��ʼ��socket
		int err = InitSocket();
		if (err <0) 
		{
#ifdef ____WIN32_
			if (m_Listenfd != INVALID_SOCKET)
			{
				closesocket(m_Listenfd);//���ڹر�һ���׽���
				m_Listenfd = INVALID_SOCKET;
			}
			WSACleanup();//���ڳ�������Ӧ�ó���� Winsock �ĳ�ʼ��
#else
			close(m_Listenfd);
#endif // ____WIN32_
			LOG_MSG("Init socket error in line %d err status:%d\n", __LINE__,err);
			return;
		}
		//2.��ʼ������ ��Ӧ���ݶ���
		for (int i = 0; i < MAX_THREAD_COUNT; i++)
		{
			m_Request[i] = new S_HTTP_BASE();
			m_Response[i] = new S_HTTP_BASE();
			m_Request[i]->Reset();
			m_Response[i]->Reset();
		}

		//3.��ʼ��mysql��

		//4.�����̳߳�

		//5.�������̼߳����µ�����

	}

}

