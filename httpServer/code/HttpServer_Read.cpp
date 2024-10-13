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

}