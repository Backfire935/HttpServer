#include "IDefine.h"

namespace http
{
	std::string deleteString(std::string s, char c)
	{
		//std::string sss;
		if (!s.empty())
		{
			int index = 0;
			while ((index = s.find(c, index)) >= 0)
			{
				s.erase(index, 1);//���� Ҫɾ�������ַ�������ʼ����  Ҫɾ�����ַ�����
			}
			//sss = s;
		}
		return s;
		//return sss;
	}

	void log_UpdateConnect(int a, int b)
	{
#ifdef ____WIN32_
		char ss[50];
		memset(ss, 0, 50);
		sprintf_s(ss, "connect:%d queue:%d", a, b);
		SetWindowTextA(GetConsoleWindow(), ss);
#endif // ____WIN32_

	}

	std::vector<std::string> split(std::string srcStr, std::string pattern, bool isadd)
	{
		std::string::size_type pos = 0;
		std::vector<std::string> arr;
		if (isadd) srcStr += pattern;
		//"123,123,"���������������Ԫ����

		int size = srcStr.size();
		for (int i = 0; i < size; i++)
		{
			pos = srcStr.find(pattern, i);//�ָ���
			if (pos < size && pos > -1)
			{
				std::string s = srcStr.substr(i, pos - i);//�ڶ��������ǳ���
				arr.push_back(s);
				i = pos + pattern.size() - 1;
			}
		}

		return arr;

	}
	std::vector<std::string> split2(std::string srcStr, std::string pattern)
	{
		std::string::size_type pos = 0;
		std::vector<std::string> arr;
		int size = srcStr.size();

		pos = srcStr.find(pattern);//�ָ���
		if (pos < 0) return arr;

		std::string s = srcStr.substr(0, pos);
		arr.push_back(s);

		if (pos + 1 >= size) return arr;

		s = srcStr.substr(pos + 1);
		arr.push_back(s);

		return arr;
	}
}

