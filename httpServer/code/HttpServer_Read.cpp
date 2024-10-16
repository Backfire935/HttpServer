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

	int HttpSevrer::analyData(Socket socketfd, S_HTTP_BASE* request, S_HTTP_BASE* response)
	{
		//先处理消息头
		if (request->state >= ER_OVER) return 0;
		if (request->state == ER_HEAD)//头解析完
		{
			if ((request->method != "POST" && request->method != "PUT") && (request->Content_length <=0 || request->Content_length > MAX_POST_LENGTH))
			{
				request->state = ER_ERROR;
				//返回错误
				return -1;
			}
			//读取消息体
			readBody(socketfd, request, response);
			return 0;
		}
		if (request->state != ER_FREE) return 0;

		//1.没有解析过数据
		int length = request->pos_tail - request->pos_head;
		request->temp_str.clear();
		request->temp_str.assign(&request->buf[request->pos_head],length);

		//2.解析请求行 请求头 请求空行
		int pos = request->temp_str.find("\r\n\r\n");//代表一个空行
		if (pos < 0)
		{
			if (request->method != "PUT")
			{
				if (length >= MAX_PACKAGE_LENGTH)//不是我们要的包
				{
					request->state == ER_ERROR;

					//返回错误
					response->SetResponseLine(403,"Failed");
					this->writeData(request, response, "err", 3);
					return -1;
				}
			}
			return 0;
		}

		length = pos + 4;//因为回车换行回车换行 \r\n\r\n
		request->temp_str.clear();//重新填充
		request->temp_str.assign(&request->buf[request->pos_head], length);
		std::vector<std::string> arr = split(request->temp_str, "\r\n", false);//分割字符串

		//2.1获取请求行数据
		std::vector<std::string> line = split(arr[0], " ",true);
		request->SetRequestLine(line);
		//2.2解析头
		for ( int i =1; i< arr.size()-1; i++)
		{
			std::vector<std::string> head = split2(arr[i], ":");
			if (head.size() == 2)
			{
				request->SetHeader(head[0],head[1]);//处理别的情况
				if (head[0] == "Content-Length")//正常情况
				{
					request->SetContentLength(head[1]);
				}
				if (head[0] == "Content-Type")//正常情况
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
		//3.设置偏移位置
		request->pos_head += (pos + 4);

		//输出***********************************************************
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

		//输出***********************************************************

		//4.提交的是post或put请求
		if (request->method != "POST" && request->method != "PUT")
		{
			request->state = ER_OVER;
			//获取数据
			response->SetResponseLine(200, "OK");
			this->writeData(request, response, "ok", 2);
			return 0;
		}

		request->state = ER_HEAD;
		if (request->Content_length <= 0 || request->Content_length > MAX_POST_LENGTH)
		{
			request->state = ER_ERROR;
			//返回错误
			response->SetResponseLine(402, "Failed");
			this->writeData(request, response, "err",3);
			return -1;
		}

		//头读完了，读取消息体
		readBody(socketfd, request, response);
		return 0;
	}

	int HttpSevrer::readBody(Socket socketfd, S_HTTP_BASE* request, S_HTTP_BASE* response)
	{
		int length = request->pos_tail - request->pos_head;
		if (length < request->Content_length) return 0;//需要继续等待解包

		//0.如果是上传资源
		if (strcmp(request->method.c_str(), "PUT") == 0)
		{
			return 0;
		}
		//1.解析protobuf
		if (strcmp(request->Content_Type.c_str(), "application/protobuf") == 0)
		{
			return 0;
		}
		//2.解析二进制数据结构
		if (strcmp(request->Content_Type.c_str(), "application/binary") == 0)
		{
			return 0;
		}
		//3.解析json
		if (strcmp(request->Content_Type.c_str(), "application/json") == 0)
		{
			return 0;
		}

		std::string body(request->buf, request->pos_head, request->Content_length);
		request->pos_head += request->Content_length;
		request->state = ER_OVER;
		LOG_MSG("readBody %d-%d %s-%d \n", request->pos_head, request->pos_tail, body.c_str(), body.size());

		//响应数据给前端
		response->SetResponseLine(200, "OK");
		this->writeData(request, response, "ok", 2);
		return 0;
	}

}