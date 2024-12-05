#pragma once

#define DEBUG_HTTP 0

#ifdef  ____WIN32_
#include<WinSock2.h>
#include<WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using Socket = SOCKET;
#else
#include<sys/socket.h>
#include<sys/epoll.h>
#include<netinet/tcp.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<cstring>
using Socket = int;
#endif //  ____WIN32_

#include<thread>
#include<mutex>
#include<list>
#include<map>
#include<vector>
#include<condition_variable>

#include<fstream>
#include<iostream>
#include<string>
#include"LogFile.hpp"

#define MAX_EXE_LEN						200
#define MAX_KEEP_ALIVE				3005
#define MAX_THREAD_COUNT		8
#define MAX_PACKAGE_LENGTH		1024//����󳤶�
#define MAX_POST_LENGTH			1024*256//����ϴ�����
#define MAX_BUF							1024*1024*1000//��������С 1000MB ���������ṩ���ͻ������ص��ļ���С��ҪС�ڸ�ֵ
#define MAX_ONES_BUF					1024*1024*4//��ʱ���� 4MB �ͻ�����Ҫ�ϴ����ļ���С��ҪС�ڸ�ֵ

#ifndef S_ISREG
#define S_ISREG(m) (((m)&S_IFREG) == S_IFREG)
#endif

namespace http
{
	extern std::string deleteString(std::string s, char c);//ɾ���ַ����е�ĳһ���ַ�

	enum E_RECV_STATE//��������״̬
	{
		ER_FREE = 0X00,
		ER_HEAD = 0X01,//��Ϣͷ
		ER_OVER = 0X02,//��ȡ���
		ER_ERROR = 0X33
	};

	enum E_SEND_STATE//��������״̬
	{
		ES_FREE = 0X00,
		ES_SENDING = 0X01,
		ES_OVER = 0X22
	};

	enum E_CONNECT_STATE//����״̬
	{
		 EC_FREE = 0X00,
		 EC_CONNECT = 0X01,
	};

#pragma pack(push, packing)//���õ��ֽڶ���
#pragma pack(1)
	//�������Ӧ���ݽṹ��
	struct S_HTTP_BASE
	{
		int state = ER_FREE;
		char buf[MAX_BUF];//��Ϣ��
		char tempBuf[MAX_ONES_BUF];//��װ��Ϣ�����ʱ������
		std::string temp_str;
		int pos_head;
		int pos_tail;
		int threadid;

		std::string method;//����
		std::string url;//ͳһ��Դ��λ��
		std::string version;//�汾��
		std::map<std::string, std::string> head;//ͷ
		int Content_length;//��Ϣ�����ݳ���
		std::string Content_Type;//��Ϣ����
		std::string Connection;//��������

		//��Ӧ����
		int status;
		std::string describe;//����

		inline void Reset()
		{
			memset(buf, 0, MAX_BUF);
			memset(tempBuf, 0, MAX_ONES_BUF);
			state = 0;
			status = -1;
			pos_head = 0;
			pos_tail = 0;
			method = "";
			url = "";
			version = "";
			Content_length = 0;
			Content_Type = "";
			Connection = "";
			head.clear();
			threadid = 0; 
		}

		inline void Init()
		{
			memset(tempBuf, 0, MAX_ONES_BUF);
			state = 0;
			status = 0;
			method = "";
			url = "";
			version = "";
			Content_length = 0;
			Content_Type = "";
			Connection = "";
			head.clear();
		}

		//����������
		inline void SetRequestLine(std::vector<std::string>& line)
		{
			if (line.size() != 3) return;
			method = line[0];
			url = line[1];
			version = line[2];
		}

		//����ͷ
		inline void SetHeader(std::string key, std::string value)
		{
			auto it = head.find(key);
			if (it != head.end())//����ҵ���key��ɾ����,��Ϊ�������key���Ḳ��
			{
				head.erase(it);
			}
			//std::make_pair ���Խ���������������ֱ�ӹ���һ�� std::pair �����������Ա��ⴴ������Ҫ����ʱ����
			head.insert(std::make_pair(key, value));
		}
		//��ȡͷ
		inline std::string GetHeader(std::string key)
		{
			auto it = head.find(key);
			if (it != head.end())
			{
				return it->second;
			}
			return "";
		}


		//������Ϣ�峤��
		inline void SetContentLength(std::string value)
		{
			auto s = deleteString(value, ' ');//ɾ�����еĿ��ַ�
			int length = atoi(s.c_str());//��ȡʣ���ַ�����
			Content_length = length;
		}
		//������Ϣ����
		inline void SetContentType(std::string value)
		{
			auto s = deleteString(value, ' ');
			Content_Type = s;
		}
		//��������
		inline void SetConnection(std::string value)
		{
			auto s = deleteString(value, ' ');
			Connection = s;
		}

		//������Ӧ��
		inline void SetResponseLine(int stat, std::string s)
		{
			version = "HTTP/1.1";
			status = stat;
			describe = s;
		}

 	};
#pragma pack(pop,packing)

	void log_UpdateConnect(int a, int b);
	std::vector<std::string> split(std::string srcStr, std::string pattern, bool isadd = false);
	std::vector<std::string> split2(std::string srcStr, std::string pattern);

	bool is_file(const std::string& path);
	void read_file(const std::string& path, std::string& out);
	bool read_Quest(const std::string& path, std::string& out);
	void initPath(); 
	int select_isread(Socket socketfd, int timeout_s, int timeout_u);
}
  

