#include"HttpServer.h"

namespace http 
{
	
	//获取响应行 响应头 空行
	std::string getResponseStr(S_HTTP_BASE* request, S_HTTP_BASE* response)
	{
		std::string stream;
		//1.响应行
		stream += response->version + " " + std::to_string(response->status) + " " + response->describe + "\r\n";
		//2.响应头
		auto it = request->head.begin();
		while (it != request->head.end())
		{
			auto key = it->first;
			auto value = it->second;
			stream += key + ":" + value + "\r\n";
			++it;
		}
		//3.响应空行
		stream += "\r\n";
		return stream;
	}

	//封包 写数据
	void HttpSevrer::writeData(S_HTTP_BASE* request, S_HTTP_BASE* response, const char* body, int size)
	{
		if (response->state != ES_FREE) return;
		if (body == NULL) return;
		if (size <= 0 || size > MAX_ONES_BUF) return;

		//1.设置响应数据消息体长度
		request->SetHeader("Content-Length", std::to_string(size));
		//2.设置响应
		response->state = ES_SENDING;

		std::string stream = getResponseStr(request, response);
		int size2 = stream.size();//消息头长度

#ifdef DEBUG_HTTP
		LOG_MSG("Response=======================%d\n", request->threadid);
		LOG_MSG("%s%s\n", stream.c_str(), body);
#endif // DEBUG_HTTP
		//填充数据
		if (response->pos_tail + size2 + size < MAX_BUF)
		{
			memcpy(&response->buf[response->pos_tail], stream.c_str(), size2);
			response->pos_tail += size2;//消息头长度

			memcpy(&response->buf[response->pos_tail], body, size);
			response->pos_tail += size;//消息体长度

		}
	 }
}