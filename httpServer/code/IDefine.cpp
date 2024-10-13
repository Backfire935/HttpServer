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
				s.erase(index, 1);//传入 要删除的子字符串的起始索引  要删除的字符数量
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
		//"123,123,"这样数组就有两个元素了

		int size = srcStr.size();
		for (int i = 0; i < size; i++)
		{
			pos = srcStr.find(pattern, i);//分隔符
			if (pos < size && pos > -1)
			{
				std::string s = srcStr.substr(i, pos - i);//第二个参数是长度
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

		pos = srcStr.find(pattern);//分隔符
		if (pos < 0) return arr;

		std::string s = srcStr.substr(0, pos);
		arr.push_back(s);

		if (pos + 1 >= size) return arr;

		s = srcStr.substr(pos + 1);
		arr.push_back(s);

		return arr;
	}
}

