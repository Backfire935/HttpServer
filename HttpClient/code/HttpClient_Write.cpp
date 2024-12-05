#include "HttpClient.h"

namespace http
{
	std::string serverIp = "127.0.0.1";
	int serverport = 8080;
	//��������
	void HttpClient::writeData(S_TEST_BASE* data)
	{
		if (m_Request->pos_head == m_Request->pos_tail)
		{
			m_Request->pos_tail = 0;
			m_Request->pos_head = 0;
		}

		if (m_Request->pos_tail + data->len < MAX_BUF)
		{
			memcpy(&m_Request->buf[m_Request->pos_tail], data->buf, data->len);
			m_Request->pos_tail += data->len;
		}

	}

	//�������
	void HttpClient::pushRequest(std::string method, std::string url, int type, const char* c, const int len)
	{
		char content_length[30];
		memset(content_length, 0, 30);
		sprintf(content_length, "Content-Length: %d\r\n", len);

		std::string content_type;
		switch (type)
		{
		case 1:content_type = "Content-Type:application/protobuf; charset=utf-8\r\n"; break;
		case 2:content_type = "Content-Type:application/binary; charset=utf-8\r\n"; break;
		case 3:content_type = "Content-Type:application/json; charset=utf-8\r\n"; break;
		default:content_type = "Content-Type:application/text; charset=utf-8\r\n"; break;
		}

		std::string stream;
		stream += method + "   " + url + " HTTP/1.1\r\n";//������
		stream += "Host: " + http::serverIp + ":" + std::to_string(http::serverport) + "\r\n";//����ͷ
		stream += content_length;
		stream += content_type;
		stream += "Connection:keep-alive\r\n";
		//����
		stream += "\r\n";


		S_TEST_BASE* data = new S_TEST_BASE();
		memset(data, 0 ,sizeof(S_TEST_BASE));

		int index = 0;
		//������ ����ͷ ����
		memcpy(&data->buf[index], stream.c_str(), stream.size());
		index += stream.size();
		data->len = stream.size();

		if (method == "PUT" || method == "POST")
		{
			memcpy(&data->buf[index], c, len);
			data->len = stream.size() + len;
		}

		{
			std::unique_lock<std::mutex> guard(m_Mutex);
			this->m_HttpDatas.push_back(data);
		}

	} 

	int HttpClient::sendSocket()
	{
		int len = m_Request->pos_tail - m_Request->pos_head;
		if (len <= 0) return 0;

		int sendBytes = send(socketfd, &m_Request->buf[m_Request->pos_head], len, 0);//��������
		if (sendBytes > 0)
		{
			m_Request->pos_head += sendBytes;
			return 0;
		}
		else if (sendBytes < 0)
		{
			int err = WSAGetLastError();
			if (err == WSAEINTR) return 0;
			else if (err == WSAEWOULDBLOCK) return 0;
			else return -1;
		}
		else
		{
			return -2;
		}

	}



}