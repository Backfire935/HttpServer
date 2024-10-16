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
		if (size <= 0 || size > MAX_ONES_BUF) return;

		//1.������Ӧ������Ϣ�峤��
		request->SetHeader("Content-Length", std::to_string(size));
		//2.������Ӧ
		response->state = ES_SENDING;

		std::string stream = getResponseStr(request, response);
		int size2 = stream.size();//��Ϣͷ����

#ifdef DEBUG_HTTP
		LOG_MSG("Response=======================%d\n", request->threadid);
		LOG_MSG("%s%s\n", stream.c_str(), body);
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
}