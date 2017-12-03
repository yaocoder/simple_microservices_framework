/**
 * @created:	   		2012/05/16
 *
 * @file				utils.h
 *
 * @author  			wei yao <yaocoder@gmail.com>
 *
 * @version 			1.0
 *
 * @LICENSE
 *
 * @brief				通用工具方法
 *
 */
#ifndef UTILS_H__
#define UTILS_H__

#include <netinet/in.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <iconv.h>

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include <uuid/uuid.h>
using namespace std;

namespace utils
{
static inline std::string GetUUID()
{
	uuid_t uu;
	int i;
	std::string c_uuid;
	char ch[2];
	uuid_generate( uu );
	for(i=0;i<16;i++)
	{
		sprintf(ch,"%02x",uu[i]);
		c_uuid += ch;
	}
	return c_uuid;
}
static inline void ShellCommand(const char *command, char *buf, int length)
{
	FILE   *stream;
	stream = popen(command,"r");
	fread( buf, sizeof(char), length,  stream);
	pclose( stream );
	return;
}

static inline void DeleteElementFrom(const std::string& value, std::vector<std::string>& vec)
{
	std::vector<std::string>::iterator iter;
	for(iter = vec.begin(); iter != vec.end();)
	{
		if((*iter).compare(value) == 0)
			iter = vec.erase(iter);
		else
			++iter;
	}
}

static inline void SplitData(const std::string& str, const std::string& delimiter, std::vector<std::string>& vec_data)
{
	std::string s = str;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos)
	{
		token = s.substr(0, pos);
		vec_data.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
}

static inline std::string ReplaceString(const std::string& str, const std::string& flag, const std::string& replaceFlag)
{
	boost::regex expressionReplace(flag);
	std::string strTemp = boost::regex_replace(str, expressionReplace, replaceFlag);
	return strTemp;
}

static inline bool FindCRLF(const std::string& s)
{
	if(s.find("\r\n") != std::string::npos)
		return true;
	else
		return false;
}

static inline std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter)
{
	std::vector<std::string> vec_temp;
	boost::algorithm::split(vec_temp, str, boost::algorithm::is_any_of(delimiter));
	DeleteElementFrom("", vec_temp);
	return vec_temp;
}

static inline std::string JoinListByDelimiter(const std::vector<std::string>&vec, const std::string& delimiter)
{
	std::string str;
	unsigned int i = 0;
	for(; i < vec.size() - 1; ++i)
	{
		str += vec.at(i) + delimiter;
	}

	str += vec.at(i);

	return str;
}

static inline std::string JoinSetByDelimiter(const std::set<std::string>& set_string, const std::string& delimiter)
{
	std::string str;
	std::set<std::string>::iterator iter;
	if(set_string.size() > 0)
	{
		iter = set_string.begin();
		for(; iter != set_string.end()--; ++iter)
		{
			str += *iter + delimiter;
		}
	}

	return str;
}

template<typename K, typename V>
inline void MapKeyToSet(const std::map<K, V>& map_temp, std::set<K>& set_temp)
{
	typedef typename std::map<K, V>::const_iterator iter_temp;
	iter_temp iter;
	for(iter = map_temp.begin(); iter != map_temp.end(); ++iter)
	{
		set_temp.insert(iter->first);
	}
}


template<typename T> std::string toString(const T& value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

static inline std::string int2str(int v)
{
	std::stringstream ss;
	ss << v;
	return ss.str();
}

static inline std::string NowtimeString()
{
    time_t time_now = time(NULL);
    struct tm* tm_now = localtime(&time_now);
    char time_str[sizeof("yyyymmddhhmmss")] = {0};
    strftime(time_str, sizeof(time_str), "%Y%m%d%H%M%S", tm_now);
    return time_str;
}

static inline std::string NowtimeStringDay()
{
    time_t time_now = time(NULL);
    struct tm* tm_now = localtime(&time_now);
    char time_str[sizeof("yyyymmddhhmmss")] = {0};
    strftime(time_str, sizeof(time_str), "%Y-%m-%d", tm_now);
    return time_str;
}

static inline std::string GetCurrentDayString()
{
	time_t time_;
	struct tm *ptm;
	time_ = time(NULL);
	ptm = localtime(&time_);
	char temp[100] =
	{ 0 };
	snprintf(temp, sizeof(temp), "%d-%d-%d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
	std::string str;
	str.assign(temp);
	return str;
}

static inline long int GetTimeStampWithMicrosecond()
{
	timeval tm;

	gettimeofday(&tm,NULL);

	return 1000000ULL * tm.tv_sec + tm.tv_usec;
}

static inline long int GetCurMidnightTStampWithMicro()
{
    time_t time_now = time(NULL);
    struct tm* tm_now = localtime(&time_now);
    tm_now->tm_hour = 0;
    tm_now->tm_min = 0;
    tm_now->tm_sec = 0;
    return mktime(tm_now) * 1000000ULL;
}

static inline std::string GetCurrentTimeWithMicrosecond()
{
    time_t time_now = time(NULL);
    struct tm* tm_now = localtime(&time_now);
    char time_str[sizeof("yyyymmddhhmmss")] = {0};
    strftime(time_str, sizeof(time_str), "%Y%m%d%H%M%S", tm_now);

	struct timeval tmval;
	gettimeofday(&tmval,NULL);

	return std::string(time_str) + int2str(tmval.tv_usec);
}

static inline std::string GetCurrentTime()
{
	time_t time_now = time(NULL);
	struct tm* tm_now = localtime(&time_now);
	char time_str[100] ={ 0 };
	strftime(time_str, sizeof(time_str), "%Y年%m月%d日 %H:%M:%S", tm_now);
	return time_str;
}

static inline std::string GetMonthString(const std::string& month)
{
	int mon = atoi(month.c_str());
	switch(mon)
	{
	case 1:
		return "January";
	case 2:
		return "February";
	case 3:
		return "March";
	case 4:
		return "April";
	case 5:
		return "May";
	case 6:
		return "June";
	case 7:
		return "July";
	case 8:
		return "August";
	case 9:
		return "September";
	case 10:
		return "October";
	case 11:
		return "November";
	case 12:
		return "December";
	default:
		break;
	}
	return NULL;
}

static inline unsigned long GetTickCount()
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
	{
		//error
	}

	return (ts.tv_sec*1000 + ts.tv_nsec/(1000*1000));
}

template<class T>
class Singleton: private T
{
public:
	static T &Instance()
	{
		static Singleton<T> _instance;
		return _instance;
	}
private:
	Singleton()
	{
	}
	~Singleton()
	{
	}
};

template<typename T>
T& G()
{
	return Singleton<T>::Instance();
}

template<typename T> inline void SafeDeleteArray(T*& p)
{
	if (NULL != p)
	{
		delete[] p;
		p = 0;
	}
}

template<typename T> inline void SafeDelete(T*& p)
{
	if (NULL != p)
	{
		delete p;
		p = 0;
	}
}

template<typename T> inline void SafeFree(T*& p)
{
	if (NULL != p)
	{
		free(p);
		p = 0;
	}
}

static inline void Unicode2UTF8OneWord(std::string& obj)
{
	int count = 0;
	int index = 0;
	while((index = int(obj.find("\\u00", index))) != int(std::string::npos))
	{
		count++;
		index++;
	}

	char ch[count+1];
	index = 0;
	std::string substr;
	for(int i = 0; i < count; i++)
	{
		index = obj.find("\\u00", index);
		index += 4;
		substr = obj.substr(index, 2); 
		ch[i] = strtol(substr.c_str(), NULL, 16);
	}

	ch[count] = '\0';
	obj = ch; 
}

static inline void Unicode2UTF8(std::string& obj)
{
	int beg = 0;
	std::string substr;

	while((beg = int(obj.find("\\u00", 0))) != int(std::string::npos))
	{
		substr = obj.substr(beg, 6);
		Unicode2UTF8OneWord(substr);
		obj.replace(beg, 6, substr);
	}
}

static inline bool code_convert(const char* from , const char* to, const char* instr, size_t inlen, char* outstr, size_t outlen)
{
	char** in = (char**)&instr;
	char** out = &outstr;
	iconv_t cd;
	if((cd = iconv_open(to,from) ) < 0 )
	{
		std::cout << "iconv open error" << std::endl;
		return false;
	}
	if(iconv(cd, in, &inlen, out, &outlen) < 0)
	{
		return false;
	}

	iconv_close(cd);

	return true;
}

static inline std::string encrypt(const char* ch)
{
	char* p = (char*)ch;
	int i = 0;
	while(*p != '\0')
		*p++ ^= i++;

	std::string  str = ch;
	return str;
}

static inline std::string decrypt(const char* ch)
{
	char* p = (char*)ch;
	int i = 0;
	while(*p != '\0')
		*p++ ^= i++;

	std::string  str = ch;
	return str;
}

static inline bool read_string(char*& des_str, int& des_len, int& des_pos, char* src_str)
{
	int len = strlen(des_str + des_pos);
	if(len >= 15 || 0 == len) {
		printf("parse len wrong. str=%s\n", des_str);
		return false;
	}

	memcpy(src_str, des_str + des_pos, len + 1);
	des_len -= (len + 1);
	des_pos += (len + 1);

	return true;
}

static inline bool read_char(char*& des_str, int& des_len, int& des_pos, char& value)
{
	if(des_len < sizeof(value)) {
		printf("parse read_char wrong.");
		return false;
	}

	memcpy(&value, des_str + des_pos, sizeof(value));
	des_len -= sizeof(value);
	des_pos += sizeof(value);

	return true;
}

static inline bool read_int(char*& des_str, int& des_len, int& des_pos, int& value)
{
	if(des_len < sizeof(value))
		return false;
	memcpy(&value, des_str + des_pos, sizeof(value));
	des_len -= sizeof(value);
	des_pos += sizeof(value);

	return true;
}

static inline bool read_uint(char*& des_str, int& des_len, int& des_pos, unsigned int& value)
{
	if(des_len < sizeof(value)) {
		printf("parse uint wrong.");
		return false;
	}

	memcpy(&value, des_str + des_pos, sizeof(value));
	des_len -= sizeof(value);
	des_pos += sizeof(value);

	return true;
}

static inline bool read_ushort(char*& des_str, int& des_len, int& des_pos, unsigned short& value)
{
	if(des_len < sizeof(value)) {
		printf("parse uint wrong.");
		return false;
	}

	memcpy(&value, des_str + des_pos, sizeof(value));
	des_len -= sizeof(value);
	des_pos += sizeof(value);

	return true;
}

static inline bool write_int(char*& des_str, int& des_len, int& des_pos, int& value)
{
	memcpy(des_str + des_pos, &value, sizeof(value));
	des_len += sizeof(value);
	des_pos += sizeof(value);

	return true;
}

}
#endif
