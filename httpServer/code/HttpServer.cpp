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
			LOG_MSG(1,"WSAStartup error, status:%d\n", errorcode);
			return -1;
		}
#endif
		//2.��������socket
		m_Listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if (m_Listenfd < 0)
		{
			LOG_MSG(1,"socket error, status:%d\n", m_Listenfd);
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
			LOG_MSG(1,"bind error, status:%d\n", err);
			return -3;
		}

		//7.����
		//����ָ�����׽��ֿ����Ŷӵȴ����ܵ�������������������������п��õĶ��ж����������������������ܻᱻ�ܾ���SOMAXCONN ͨ�������ṩһ�����㹻�󡱵�ֵ����ȷ�����������оܾ���������
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
			LOG_MSG(1,"Init socket error in line %d err status:%d\n", __LINE__,err);
			return;
		}
		//2.��ʼ���������Ӧ�����ݶ���
		for (int i = 0; i < MAX_THREAD_COUNT; i++)
		{
			m_Request[i] = new S_HTTP_BASE();
			m_Response[i] = new S_HTTP_BASE();
			m_Request[i]->Reset();
			m_Response[i]->Reset();
		}

		//3.��ʼ��mysql��

		//4.�����̳߳�
		runThread();
		//5.�������̼߳����µ�����
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

	//���̼߳����µ����ӣ����ɷ�����
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
			//�����ӡ
			log_UpdateConnect(m_ConnectCount, m_Socketfds.size());

			this->m_Condition.notify_one();
		}
	}

	//****************************************************
	//****************************************************

	//��ʼ���̳߳أ����߳������̷߳���
	void HttpSevrer::runThread()
	{
		//�����߳�
		for (int i = 0; i < MAX_THREAD_COUNT; i++)
			m_Thread[i].reset(new std::thread(HttpSevrer::run, this,  i));
		//����
		for (int i = 0; i < MAX_THREAD_COUNT; i++)
			m_Thread[i]->detach();
	}


	//���������̻߳������
	void HttpSevrer::run(HttpSevrer* serverInstance, int threadId)
	{
		int socketfd = -1;
		while (true)
		{
			{
				std::unique_lock<std::mutex> guard(serverInstance->m_Mutex);
				while (serverInstance->m_Socketfds.empty())//��ֹ��ٻ��ѣ����������û�����ݾ�wait�������ݾ����´���
				{
					LOG_MSG(1,"************************** thread wait...%d \n", threadId);
					serverInstance->m_Condition.wait(guard);
				}
				socketfd = serverInstance->m_Socketfds.front();//��ȡ����ͷ����һ������
				serverInstance->m_Socketfds.pop_front();//ɾ��

				LOG_MSG(1,"************************** thread awake...%d-%d\n", (int)socketfd, threadId);
				//�����ӡ
				log_UpdateConnect(serverInstance->m_ConnectCount, serverInstance->m_Socketfds.size());
			}
			//��ʼ����socketfd
			serverInstance->runSocket(socketfd, threadId);
		}

	}

	//�ڹ����߳� �����µ����ӵ�����
	void HttpSevrer::runSocket(Socket socketfd, int threadId)
	{
#ifdef ____WIN32_
		//������shutdown���û᲻�ɹ�
		setsockopt(socketfd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&m_Listenfd),sizeof(m_Listenfd));
#endif
		//1.���÷�����socket
		setNonBlockingSocket(socketfd);
		//3.����Nagle�㷨��û���ӳ�
		int value = 1;
		setsockopt(m_Listenfd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&value), sizeof(value));

		auto request = m_Request[threadId];
		auto response = m_Response[threadId]; 
		request->Reset();
		response->Reset();
		request->threadid = threadId;		
		response->threadid = threadId;

		std::string closeResult;
		auto starttime = std::chrono::steady_clock::now();//��ȡ��ǰʱ��

		while (true)
		{
			//0.���޿ɶ�����
			int err = select_isread(socketfd, 0, 2000);
			if (err < 0)
			{
				closeResult = "select_isread is null " + err;
				break;//û������
			}
			//1.�Ӱ� ճ��
			if (err > 0)
			{
				//��������
				err = recvSocket(socketfd, request);
				if (err < 0)
				{
					closeResult = "recvSocket " + err;
					break;
				}
			}
			//2.���
			if (request->state <= ER_HEAD)
			{
				//������
				analyData(socketfd, request, response);
			}
			//3.����
			err = sendSocket(socketfd, response, threadId);
			if (err < 0)
			{
				closeResult = "sendSocket";
				break;
			}
			//4.�������
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
			//�������ʱ��
			auto maxSurvivalTime = std::chrono::steady_clock::now() - starttime;//��ȡ��ǰʱ��
			auto durationTime = std::chrono::duration_cast<std::chrono::milliseconds>(maxSurvivalTime);
			if (durationTime.count() > MAX_KEEP_ALIVE)
			{
				closeResult = "timeout"; 
				break;
			}
		}

		//����������
		{
			std::unique_lock<std::mutex> guard(this->m_ConnectMutex);
			m_ConnectCount--;
			LOG_MSG(2,"closesocket:%s  connect:%d-%d \n", closeResult.c_str(), m_ConnectCount, (int)m_Socketfds.size());
		}

		//��������������ر�����
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

