#include "IDefine.h"

#ifdef ____WIN32_
#else
#include<sys/stat.h>
#include<unistd.h>
#endif // DEBUG

namespace http
{
	char FileExePath[MAX_EXE_LEN];
	void initPath()
	{
		memset(FileExePath, 0, MAX_EXE_LEN);
#ifdef ____WIN32_
		GetModuleFileNameA(NULL, (LPSTR)FileExePath, MAX_EXE_LEN);
		std::string str(FileExePath);
		size_t pos = str.find_last_of("\\");
		str = str.substr(0, pos+1);
		memcpy(FileExePath, str.c_str(), MAX_EXE_LEN);
		printf("FileExePath:%s \n", FileExePath);
#else
		int ret = readlink("/proc/self/exe", FileExePath, MAX_EXE_LEN);
		std::string str(FileExePath);
		size_t pos = str.find_last_of("/");
		str = str.substr(0, pos + 1);
		memcpy(FileExePath, str.c_str(), MAX_EXE_LEN);
		printf("FileExePath:%s \n", FileExePath);
#endif // ____WIN32_

	}

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
		//std::string::size_type pos = 0;
		int pos = 0;
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
		int  pos = 0;
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
	bool is_file(const std::string& path)
	{
		struct stat st;
		return stat(path.c_str(), &st) >= 0 && S_ISREG(st.st_mode);//��ȷ������·���Ƿ�ָ��һ�����ڵ���ͨ�ļ�
	}
	/**
 * ��ȡ�ļ����ݵ��ַ����С�
 *
 * ���������һ���ļ�����ȡ��ȫ�����ݣ�������洢��һ���ַ����С�
 * �ļ��Զ�����ģʽ�򿪣���������ȷ�������ļ�������Σ����ܱ���ȷ��ȡ��
 *
 * @param path �ļ���·����
 * @param out ���ڴ洢�ļ����ݵ��ַ������á�
 */
	void read_file(const std::string& path, std::string& out)
	{
		std::ifstream fs(path, std::ios_base::binary); // �Զ�����ģʽ���ļ�
		fs.seekg(0, std::ios_base::end);// �ƶ��ļ����Ķ�ȡָ�뵽�ļ�ĩβ���Ա��ȡ�ļ���С

		auto size = fs.tellg(); // ��ȡ�ļ���С
		fs.seekg(0);// ���ļ����Ķ�ȡָ�����¶�λ���ļ���ͷ
		out.resize(static_cast<size_t>(size));// ��������ַ����Ĵ�С����Ӧ�ļ�����
		fs.read(&out[0], static_cast<std::streamsize>(size)); // ���ļ��ж�ȡ���ݵ��ַ���
	}

	//��ȡ�����ļ�
	bool read_Quest(const std::string& filename, std::string& out)
	{
		std::string filepath(FileExePath);
		std::string sub_path = filepath + "res\\" + filename;

		if (sub_path.back() == '/')
		{
			sub_path += "index.html";
		}

		if (is_file(sub_path))
		{
			read_file(sub_path, out);
			return true;
		}
		return false;
	}
}

