
#include "HttpClient.h"

int main()
{
	http::HttpClient* client = new http::HttpClient();
	client->id = 0;
	client->pushRequest("GET","123456.txt", 0, "123456.txt", 10);

	//唤醒工作线程
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
	
}