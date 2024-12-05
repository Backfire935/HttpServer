#include "HttpClient.h"

namespace http
{

	HttpClient::HttpClient()
	{
		InitSocket();
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

	void HttpClient::runSocket()
	{
		while (true)
		{
			//0.�ж����޿ɶ�����
			int err = select_isread(this->socketfd, 0, 10000);
			if (err < 0)
			{
				LOG_MSG(3,"*******************************closesocket select_isread...%d-%d\n", id, err);
				break;
			}
			//1.�Ӱ� ճ������
			if (err > 0)
			{
				//��ȡ����
				err = this->recvSocket();
				if (err < 0)
				{
					LOG_MSG(3, "*******************************closesocket recvSocket...%d-%d\n", id, err);
					break;
				}
			}
			//2.���
			if (m_Response->state <= ER_HEAD)
			{
				analyData();
			}
			//����ֵ����
			if (m_Response->state == ER_ERROR)
			{
				LOG_MSG(3, "*******************************closesocket analy error...%d-%d\n", id, err);
				break;
			}


			//3.���д���� ǰ����������߼�ֻ��Ϊ�˻����̣߳��������������������
			{
				std::unique_lock<std::mutex> guard(this->m_Mutex);
				if (! this->m_HttpDatas.empty())
				{
					S_TEST_BASE* data = this->m_HttpDatas.front();
					this->m_HttpDatas.pop_front();
					this->writeData(data);
					delete data;
				}
			}
			//4.����
			err = this->sendSocket();
			if (err < 0)
			{
				LOG_MSG(3, "*******************************closesocket sendSocket...%d-%d\n", id, err);
				break;
			}

		}
	}



	void HttpClient::run(HttpClient* c)//�̺߳���
	{
		
		while (true)
		{
			S_TEST_BASE* data;
			{
				std::unique_lock<std::mutex> guard(c->m_Mutex);
				while (c->m_HttpDatas.empty())
				{
					LOG_MSG(2, "**************************** thread wait...\n");
					c->m_Condition.wait(guard);//�ȴ�����
				}
				LOG_MSG(2, "**************************** thread awake...\n");
				data = c->m_HttpDatas.front();
				c->m_HttpDatas.pop_front();
			}

			//������
			if (c->state != EC_CONNECT)
			{
				c->ConnectServer();
			}

			//�������� д��������
			//����socket
			c->writeData(data);
			c->runSocket();
			break;
		}
	}

}

