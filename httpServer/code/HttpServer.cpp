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
			closesocket(m_Listenfd);//用于关闭一个套接字
			m_Listenfd = INVALID_SOCKET;
		}
		WSACleanup();//用于撤销整个应用程序对 Winsock 的初始化
#else
		close(m_Listenfd);
#endif // ____WIN32_

	}

	bool setNonBlockingSocket(Socket socketfd)
	{
#ifdef  ____WIN32_
		unsigned long u1 = 1;
		int err = ioctlsocket(socketfd, FIONBIO, (unsigned long*)& u1);//设置连接为非阻塞的
		if (err == SOCKET_ERROR) return false;
		return true;
#else
		int flags = fcntl(socketfd, F_GETFL); 
		if (flags < 0) return false;
		flags |= O_NONBLOCK;

		if (fcntl(socketfd, F_GETFL, flags) < 0)//设置文件状态标志
			return false;
		return true;
#endif // ! ____WIN32_

	}

	int HttpSevrer::InitSocket()
	{
#ifdef ____WIN32_ 
		//1.初始化Windows sockets DLL
		WSADATA wsData;
		int errorcode = WSAStartup(MAKEWORD(2,2), &wsData);
		if (errorcode != 0)
		{
			LOG_MSG("WSAStartup error, status:%d\n", errorcode);
			return -1;
		}
#endif
		//2.创建监听socket
		m_Listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if (m_Listenfd < 0)
		{
			LOG_MSG("socket error, status:%d\n", m_Listenfd);
			return -2;
		}

		//3.禁用Nagle算法，没有延迟
		int value = 1;
		setsockopt(m_Listenfd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&value), sizeof(value));

		//4.设置非阻塞
		setNonBlockingSocket(m_Listenfd);

		//5.启用端口号重复绑定
		int flag = 1;
		int ret = setsockopt(m_Listenfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flag), sizeof(int));

		//6.绑定IP端口
		struct sockaddr_in serAddr;
		memset(&serAddr, 0, sizeof(sockaddr_in));

		serAddr.sin_family = AF_INET;//IPv4
		serAddr.sin_port = htons(8080);
#ifdef ____WIN32_
		serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
#else
		serAddr.sin_addr.S_addr = INADDR_ANY;
#endif // ____WIN32_
		int err = ::bind(m_Listenfd, (struct sockaddr*)&serAddr, sizeof(serAddr));//绑定
		if (err < 0)
		{
			LOG_MSG("bind error, status:%d\n", err);
			return -3;
		}

		//7.监听
		//参数指定了套接字可以排队等待接受的连接请求的最大数量。如果所有可用的队列都已满，额外的连接请求可能会被拒绝。SOMAXCONN 通常用于提供一个“足够大”的值，以确保不会无意中拒绝连接请求。
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

		//1.初始化socket
		int err = InitSocket();
		if (err <0) 
		{
#ifdef ____WIN32_
			if (m_Listenfd != INVALID_SOCKET)
			{
				closesocket(m_Listenfd);//用于关闭一个套接字
				m_Listenfd = INVALID_SOCKET;
			}
			WSACleanup();//用于撤销整个应用程序对 Winsock 的初始化
#else
			close(m_Listenfd);
#endif // ____WIN32_
			LOG_MSG("Init socket error in line %d err status:%d\n", __LINE__,err);
			return;
		}
		//2.初始化请求 响应数据对象
		for (int i = 0; i < MAX_THREAD_COUNT; i++)
		{
			m_Request[i] = new S_HTTP_BASE();
			m_Response[i] = new S_HTTP_BASE();
			m_Request[i]->Reset();
			m_Response[i]->Reset();
		}

		//3.初始化mysql库

		//4.运行线程池

		//5.运行主线程监听新的连接

	}

}

