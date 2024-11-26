
#include "HttpClient.h"

int main()
{
	http::HttpClient* client = new http::HttpClient();
	client->id = 0;
	client->pushRequest("POST","hi", 0, "123456", 6);
	return 0;
}