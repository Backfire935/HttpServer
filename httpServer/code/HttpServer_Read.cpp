#include"HttpServer.h"

namespace http {

	//�������� ճ������
	int HttpSevrer::recvSocket(Socket socketfd, S_HTTP_BASE* request)
	{
		memset(request->tempBuf, 0, MAX_ONES_BUF);

		int recvBytes = recv(socketfd, request->tempBuf, MAX_ONES_BUF, 0);
		if (recvBytes > 0)
		{
			if (request->pos_head == request->pos_tail)
			{
				request->pos_head = 0;
				request->pos_tail = 0;
			}
			if (request->pos_tail + recvBytes >= MAX_BUF) return -1;//���������ݳ����˻��������������
			memcpy(&request->buf[request->pos_tail], request->tempBuf, recvBytes);
			request->pos_tail += recvBytes;
			return 0;
		}
#ifdef ____WIN32_
		if (recvBytes < 0) 
		{
			int err = WSAGetLastError();
			if (err == WSAEINTR) return 0;//��ʾ�����������жϣ����磬�����̱߳�ȡ������һ���źű����ݸ�����
			else if (err == WSAEWOULDBLOCK) return 0;//��ʾ��ǰû�����ݿɶ��������߿�����Ҫ�Ժ��ٴγ��ԡ�
			else return -1;//��ʾ������һ�������Ĵ��󣬵�������Ҫ�����������
		}
		else if (recvBytes == 0)//�Զ��Ѿ��ر�����
		{
			return -2;
		}
#else
		if (recvBytes < 0)
		{
			if (errno == EINTR) return 0;//��ʾ�����������жϣ����磬�����̱߳�ȡ������һ���źű����ݸ�����
			else if (errno == EAGAIN) return 0;//��ʾ��ǰû�����ݿɶ��������߿�����Ҫ�Ժ��ٴγ��ԡ�
			else return -1;//��ʾ������һ�������Ĵ��󣬵�������Ҫ�����������
		}
		else if (recvBytes == 0)//�Զ��Ѿ��ر�����
		{
			return -2;
		}
#endif // ____WIN32_

	}

	int HttpSevrer::analyData(Socket socketfd, S_HTTP_BASE* request, S_HTTP_BASE* response)
	{
		//�ȴ�����Ϣͷ
		if (request->state >= ER_OVER) return 0;
		if (request->state == ER_HEAD)//ͷ������
		{
			if ((request->method != "POST" && request->method != "PUT") && (request->Content_length <=0 || request->Content_length > MAX_POST_LENGTH))
			{
				request->state = ER_ERROR;
				//���ش���
				return -1;
			}
			//��ȡ��Ϣ��
			readBody(socketfd, request, response);
			return 0;
		}
		if (request->state != ER_FREE) return 0;

		//1.û�н���������
		int length = request->pos_tail - request->pos_head;
		request->temp_str.clear();
		request->temp_str.assign(&request->buf[request->pos_head],length);

		//2.���������� ����ͷ �������
		int pos = request->temp_str.find("\r\n\r\n");//����һ������
		if (pos < 0)
		{
			if (request->method != "PUT")
			{
				if (length >= MAX_PACKAGE_LENGTH)//��������Ҫ�İ�
				{
					request->state == ER_ERROR;

					//���ش���
					response->SetResponseLine(403,"Failed");
					this->writeData(request, response, "err", 3);
					return -1;
				}
			}
			return 0;
		}

		length = pos + 4;//��Ϊ�س����лس����� \r\n\r\n
		request->temp_str.clear();//�������
		request->temp_str.assign(&request->buf[request->pos_head], length);
		std::vector<std::string> arr = split(request->temp_str, "\r\n", false);//�ָ��ַ���

		//2.1��ȡ����������
		std::vector<std::string> line = split(arr[0], " ",true);
		request->SetRequestLine(line);
		//2.2����ͷ
		for ( int i =1; i< arr.size()-1; i++)
		{
			std::vector<std::string> head = split2(arr[i], ":");
			if (head.size() == 2)
			{
				request->SetHeader(head[0],head[1]);//���������
				if (head[0] == "Content-Length")//�������
				{
					request->SetContentLength(head[1]);
				}
				if (head[0] == "Content-Type")//�������
				{
					std::vector<std::string> aaa = split2(head[1], ":");
					if (aaa.size() > 0) request->SetContentType(aaa[0]);
				}
				if (head[0] == "Connection")
				{
					request->SetConnection(head[1]);
				}
			}
		}
		//3.����ƫ��λ��
		request->pos_head += (pos + 4);

		//���***********************************************************
#ifdef DEBUG_HTTP
		LOG_MSG("Reques======================%d-%d\n",(int)socketfd, request->threadid);
		LOG_MSG("%s %s %s\n", request->method.c_str(), request->url.c_str(), request->version.c_str());

		auto it = request->head.begin();
		while (it != request->head.end())
		{
			auto a = it->first;
			auto b = it->second;
			LOG_MSG("%s:%s\n",a.c_str(), b.c_str());
			++it;
		}
		LOG_MSG("\r\n");

#endif // DEBUG_HTTP

		//���***********************************************************

		//4.�ύ����post��put����
		if (request->method != "POST" && request->method != "PUT")
		{
			request->state = ER_OVER;
			//��ȡ����
			response->SetResponseLine(200, "OK");
			this->writeData(request, response, "ok", 2);
			return 0;
		}

		request->state = ER_HEAD;
		if (request->Content_length <= 0 || request->Content_length > MAX_POST_LENGTH)
		{
			request->state = ER_ERROR;
			//���ش���
			response->SetResponseLine(402, "Failed");
			this->writeData(request, response, "err",3);
			return -1;
		}

		//ͷ�����ˣ���ȡ��Ϣ��
		readBody(socketfd, request, response);
		return 0;
	}

	int HttpSevrer::readBody(Socket socketfd, S_HTTP_BASE* request, S_HTTP_BASE* response)
	{
		int length = request->pos_tail - request->pos_head;
		if (length < request->Content_length) return 0;//��Ҫ�����ȴ����

		//0.������ϴ���Դ
		if (strcmp(request->method.c_str(), "PUT") == 0)
		{
			return 0;
		}
		//1.����protobuf
		if (strcmp(request->Content_Type.c_str(), "application/protobuf") == 0)
		{
			return 0;
		}
		//2.�������������ݽṹ
		if (strcmp(request->Content_Type.c_str(), "application/binary") == 0)
		{
			return 0;
		}
		//3.����json
		if (strcmp(request->Content_Type.c_str(), "application/json") == 0)
		{
			return 0;
		}

		std::string body(request->buf, request->pos_head, request->Content_length);
		request->pos_head += request->Content_length;
		request->state = ER_OVER;
		LOG_MSG("readBody %d-%d %s-%d \n", request->pos_head, request->pos_tail, body.c_str(), body.size());

		//��Ӧ���ݸ�ǰ��
		response->SetResponseLine(200, "OK");
		this->writeData(request, response, "ok", 2);
		return 0;
	}

}