#include "IDefine.h"

std::string http::deleteString(std::string s, char c)
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
