#include "HttpServer.h"
#ifdef ____WIN32_
#include<MSWSock.h>
#else

#endif
#include <chrono>

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
			LOG_MSG(1,"WSAStartup error, status:%d\n", errorcode);
			return -1;
		}
#endif
		//2.创建监听socket
		m_Listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if (m_Listenfd < 0)
		{
			LOG_MSG(1,"socket error, status:%d\n", m_Listenfd);
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
			LOG_MSG(1,"bind error, status:%d\n", err);
			return -3;
		}

		//7.监听
		//参数指定了套接字可以排队等待接受的连接请求的最大数量。如果所有可用的队列都已满，额外的连接请求可能会被拒绝。SOMAXCONN 通常用于提供一个“足够大”的值，以确保不会无意中拒绝连接请求。
		err = listen(m_Listenfd, SOMAXCONN);
		if (err < 0)
		{
			LOG_MSG(1,"listen error, status:%d\n", err);
			return -4;
		}

		return 0;
	}



	void HttpSevrer::runServer()
	{
		initPath();

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
			LOG_MSG(1,"Init socket error in line %d err status:%d\n", __LINE__,err);
			return;
		}
		//2.初始化请求和响应的数据对象
		for (int i = 0; i < MAX_THREAD_COUNT; i++)
		{
			m_Request[i] = new S_HTTP_BASE();
			m_Response[i] = new S_HTTP_BASE();
			m_Request[i]->Reset();
			m_Response[i]->Reset();
		}

		//3.初始化mysql库

		//4.运行线程池
		runThread();
		//5.运行主线程监听新的连接
		runAccept();
	}

	int select_isread(Socket socketfd, int timeout_s, int timeout_u)
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(socketfd, &fds);

		timeval tv;
		tv.tv_sec = timeout_s;
		tv.tv_usec = timeout_u;

		while (true)
		{
			int err = select(socketfd + 1, &fds, NULL, NULL, &tv);
			if (err < 0 && errno == EINTR) continue;
			return err;
		}
		return -1;
	}

	//主线程监听新的连接，并派发任务
	void HttpSevrer::runAccept()
	{
		while (true)
		{
			int value = select_isread(m_Listenfd, 0 ,5000);
			if (value == 0)continue;

			socklen_t clilen = sizeof(struct  sockaddr);
			struct sockaddr_in clientaddr;

			int socketfd = accept(m_Listenfd, (struct sockaddr*) &clientaddr, &clilen);
			if (socketfd < 0)
			{
				if (errno == EMFILE)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));

					LOG_MSG(1,"errno == EMFILE..\n");
					continue;
				}
				LOG_MSG(1,"errno %d %d-%d\n", m_Listenfd, socketfd, errno);
				break;
			}

			{
				std::unique_lock<std::mutex> guard(this->m_Mutex);
				this->m_Socketfds.push_back(socketfd);
			}
			{
				std::unique_lock<std::mutex> guard(this->m_ConnectMutex);
				m_ConnectCount++;
			}
			//输出打印
			log_UpdateConnect(m_ConnectCount, m_Socketfds.size());

			this->m_Condition.notify_one();
		}
	}

	//****************************************************
	//****************************************************

	//初始化线程池，子线程与主线程分离
	void HttpSevrer::runThread()
	{
		//运行线程
		for (int i = 0; i < MAX_THREAD_COUNT; i++)
			m_Thread[i].reset(new std::thread(HttpSevrer::run, this,  i));
		//分离
		for (int i = 0; i < MAX_THREAD_COUNT; i++)
			m_Thread[i]->detach();
	}


	//单个工作线程唤醒入口
	void HttpSevrer::run(HttpSevrer* serverInstance, int threadId)
	{
		int socketfd = -1;
		while (true)
		{
			{
				std::unique_lock<std::mutex> guard(serverInstance->m_Mutex);
				while (serverInstance->m_Socketfds.empty())//防止虚假唤醒，如果链表中没有数据就wait，有数据就往下处理
				{
					LOG_MSG(1,"************************** thread wait...%d \n", threadId);
					serverInstance->m_Condition.wait(guard);
				}
				socketfd = serverInstance->m_Socketfds.front();//获取链表头部第一个数据
				serverInstance->m_Socketfds.pop_front();//删除

				LOG_MSG(1,"************************** thread awake...%d-%d\n", (int)socketfd, threadId);
				//输出打印
				log_UpdateConnect(serverInstance->m_ConnectCount, serverInstance->m_Socketfds.size());
			}
			//开始处理socketfd
			serverInstance->runSocket(socketfd, threadId);
		}

	}

	//在工作线程 处理新的连接的数据
	void HttpSevrer::runSocket(Socket socketfd, int threadId)
	{
#ifdef ____WIN32_
		//不设置shutdown调用会不成功
		setsockopt(socketfd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&m_Listenfd),sizeof(m_Listenfd));
#endif
		//1.设置非阻塞socket
		setNonBlockingSocket(socketfd);
		//3.禁用Nagle算法，没有延迟
		int value = 1;
		setsockopt(m_Listenfd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&value), sizeof(value));

		auto request = m_Request[threadId];
		auto response = m_Response[threadId]; 
		request->Reset();
		response->Reset();
		request->threadid = threadId;		
		response->threadid = threadId;

		std::string closeResult;
		auto starttime = std::chrono::steady_clock::now();//获取当前时间

		while (true)
		{
			//0.有无可读数据
			int err = select_isread(socketfd, 0, 2000);
			if (err < 0)
			{
				closeResult = "select_isread is null " + err;
				break;//没有数据
			}
			//1.接包 粘包
			if (err > 0)
			{
				//接受数据
				err = recvSocket(socketfd, request);
				if (err < 0)
				{
					closeResult = "recvSocket " + err;
					break;
				}
			}
			//2.解包
			if (request->state <= ER_HEAD)
			{
				//解析包
				analyData(socketfd, request, response);
			}
			//3.发包
			err = sendSocket(socketfd, response, threadId);
			if (err < 0)
			{
				closeResult = "sendSocket";
				break;
			}
			//4.发送完毕
			if (response->state == ES_OVER)
			{
				if (request->state == ER_ERROR)
				{
					closeResult = "request error";
					break;
				}
				if (request->Connection == "close")
				{
					closeResult = "close";
					break;
				}
				response->state = ES_FREE;
				request->Init();
			}

			//*********************************************************
			//生存最大时间
			auto maxSurvivalTime = std::chrono::steady_clock::now() - starttime;//获取当前时间
			auto durationTime = std::chrono::duration_cast<std::chrono::milliseconds>(maxSurvivalTime);
			if (durationTime.count() > MAX_KEEP_ALIVE)
			{
				closeResult = "timeout"; 
				break;
			}
		}

		//上锁连接数
		{
			std::unique_lock<std::mutex> guard(this->m_ConnectMutex);
			m_ConnectCount--;
			LOG_MSG(2,"closesocket:%s  connect:%d-%d \n", closeResult.c_str(), m_ConnectCount, (int)m_Socketfds.size());
		}

		//输出队列人数并关闭连接
		log_UpdateConnect(m_ConnectCount, m_Socketfds.size());
#ifdef ____WIN32_
		shutdown(socketfd, SD_BOTH);
		closesocket(socketfd);
#else
		shutdown(socketfd, SHUT_RDWR);
		close(socketfd);
#endif
	}

}

