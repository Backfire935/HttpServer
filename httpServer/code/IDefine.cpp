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
		//std::string::size_type pos = 0;
		int pos = 0;
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
		int  pos = 0;
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
	bool is_file(const std::string& path)
	{
		struct stat st;
		return stat(path.c_str(), &st) >= 0 && S_ISREG(st.st_mode);//于确定给定路径是否指向一个存在的普通文件
	}
	/**
 * 读取文件内容到字符串中。
 *
 * 这个函数打开一个文件，读取其全部内容，并将其存储在一个字符串中。
 * 文件以二进制模式打开，这样可以确保无论文件内容如何，都能被正确读取。
 *
 * @param path 文件的路径。
 * @param out 用于存储文件内容的字符串引用。
 */
	void read_file(const std::string& path, std::string& out)
	{
		std::ifstream fs(path, std::ios_base::binary); // 以二进制模式打开文件
		fs.seekg(0, std::ios_base::end);// 移动文件流的读取指针到文件末尾，以便获取文件大小

		auto size = fs.tellg(); // 获取文件大小
		fs.seekg(0);// 将文件流的读取指针重新定位到文件开头
		out.resize(static_cast<size_t>(size));// 调整输出字符串的大小以适应文件内容
		fs.read(&out[0], static_cast<std::streamsize>(size)); // 从文件中读取内容到字符串
	}

	//读取请求文件
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

