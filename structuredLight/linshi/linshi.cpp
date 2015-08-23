// linshi.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>

using namespace std;

std::string azip(const std::string& input);
std::string aunzip(const std::string& input);
std::string base64encode(const std::string& input);
std::string base64decode(const std::string& input);
bool bzip(const std::string& input, std::string* output);
bool bunzip(const std::string& input, std::string* output);

class Base
{
public:
	virtual string encode(string);
	virtual string decode(string);
};

class A : public Base
{
	virtual string encode(string input) {return azip(input); }
	virtual string decode(string input) {return aunzip(input); }
};

class B : public Base
{
	virtual string encode(string input) { return base64encode(input); }
	virtual string decode(string input) { return base64decode(input); }
};

class C : public Base
{
	virtual string encode(string input) { string t; bzip(input, &t); return t; }
	virtual string decode(string input) { string t; bunzip(input, &t); return t; }
};

template<class T1, class T2>
void P(string src, string dst)
{
	string temp;
	temp = T1.decode(src);
	dst = T2.encode(temp);
}


