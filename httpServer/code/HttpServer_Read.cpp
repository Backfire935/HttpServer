#include"HttpServer.h"

namespace http {

	//接收数据 粘包处理
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
			if (request->pos_tail + recvBytes >= MAX_BUF) return -1;//传来的数据超过了缓冲区的最大容量
			memcpy(&request->buf[request->pos_tail], request->tempBuf, recvBytes);
			request->pos_tail += recvBytes;
			return 0;
		}
#ifdef ____WIN32_
		if (recvBytes < 0) 
		{
			int err = WSAGetLastError();
			if (err == WSAEINTR) return 0;//表示阻塞操作被中断，例如，由于线程被取消或者一个信号被传递给进程
			else if (err == WSAEWOULDBLOCK) return 0;//表示当前没有数据可读，调用者可能需要稍后再次尝试。
			else return -1;//表示发生了一个真正的错误，调用者需要处理这个错误
		}
		else if (recvBytes == 0)//对端已经关闭连接
		{
			return -2;
		}
#else
		if (recvBytes < 0)
		{
			if (errno == EINTR) return 0;//表示阻塞操作被中断，例如，由于线程被取消或者一个信号被传递给进程
			else if (errno == EAGAIN) return 0;//表示当前没有数据可读，调用者可能需要稍后再次尝试。
			else return -1;//表示发生了一个真正的错误，调用者需要处理这个错误
		}
		else if (recvBytes == 0)//对端已经关闭连接
		{
			return -2;
		}
#endif // ____WIN32_

	}

}