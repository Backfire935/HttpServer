#include"HttpServer.h"

namespace http 
{
	
	//��ȡ��Ӧ�� ��Ӧͷ ����
	std::string getResponseStr(S_HTTP_BASE* request, S_HTTP_BASE* response)
	{
		std::string stream;
		//1.��Ӧ��
		stream += response->version + " " + std::to_string(response->status) + " " + response->describe + "\r\n";
		//2.��Ӧͷ
		auto it = request->head.begin();
		while (it != request->head.end())
		{
			auto key = it->first;
			auto value = it->second;
			stream += key + ":" + value + "\r\n";
			++it;
		}
		//3.��Ӧ����
		stream += "\r\n";
		return stream;
	}

	//��� д����
	void HttpSevrer::writeData(S_HTTP_BASE* request, S_HTTP_BASE* response, const char* body, int size)
	{
		if (response->state != ES_FREE) return;
		if (body == NULL) return;
		if (size <= 0 || size > MAX_BUF || strlen(body) > MAX_BUF)
		{
			LOG_MSG(3, ",������MAX_BUF��С:%d �ļ���С%d--%dkb.�������������С\n", MAX_BUF, strlen(body), size);//�����������ˣ�����������
			return;
		}

		//1.������Ӧ������Ϣ�峤��
		request->SetHeader("Content-Length", std::to_string(size));
		//2.������Ӧ
		response->state = ES_SENDING;

		std::string stream = getResponseStr(request, response);
		int size2 = stream.size();//��Ϣͷ����

#ifdef DEBUG_HTTP
		if (strlen(body) > size)
		{
			LOG_MSG(2, "�������ַ�������δ����Ϊ�ļ���С,�ļ�����ʧ��\n");
			return;
		}
		LOG_MSG(1,"��Ϣ���С�� %d %d ��Ϣͷ��С�� %d\n", strlen(body), size, size2);
		LOG_MSG(1,"Response=======================%d\n", request->threadid);
		LOG_MSG(1,"%s%s\n", stream.c_str(), body);

#endif // DEBUG_HTTP
		//�������
		if (response->pos_tail + size2 + size < MAX_BUF)
		{
			memcpy(&response->buf[response->pos_tail], stream.c_str(), size2);
			response->pos_tail += size2;//��Ϣͷ����

			memcpy(&response->buf[response->pos_tail], body, size);
			response->pos_tail += size;//��Ϣ�峤��

		}
	 }
	int HttpSevrer::sendSocket(Socket socketfd, S_HTTP_BASE* response, int threadId)
	{
		if (response->state != ES_SENDING) return 0;
		int len = response->pos_tail - response->pos_head;
		if (len <= 0) return 0;

		int sendBytes = send(socketfd, &response->buf[response->pos_head], len, 0);
		if (sendBytes > 0)
		{
			response->pos_head += sendBytes;
			if (response->pos_head == response->pos_tail)
			{
				//��ʼ����Ӧ����
				response->Reset();
				response->state = ES_OVER;
			}
			LOG_MSG(2, "��������ݷ������,����������%dkb--%.1lf MB!\n", sendBytes, sendBytes/1024);
			return 0;
		}

		LOG_MSG(1,"sendSocket err %d-%d\n",len ,sendBytes);
#ifdef  ____WIN32_
		if (sendBytes < 0)
		{
			int err = WSAGetLastError();
			if (err == WSAEINTR) return 0;
			else if (err == WSAEWOULDBLOCK) return 0;
			else return -1;
		}
		else if (sendBytes == 0)
		{
			return -2;
		}
#else
		if (sendBytes < 0)//����
			if (errno == EINTR) return 0;//���ź��ж�
			else if (errno == EAGAIN) return 0;//û������ ���Ժ�����
			else return -1;
		else if(sendBytes == 0)
			{
				return -2;
		    }
#endif //  ____WIN32_
		
	}
}