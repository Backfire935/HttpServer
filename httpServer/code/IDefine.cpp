#include "IDefine.h"

std::string http::deleteString(std::string s, char c)
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
