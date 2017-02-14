#pragma once
#include<vector>
#include<cstdio>
#include<string>
#include<iostream>
#include<set>
#include<fstream>
#include<algorithm>
using namespace std;
class Data;
class RawDataReader;
const int NORMAL = 0;
const int START = 1;
const int END = 2;
const int SINGLE = 3;

class Data {
public:
	Data() {}
	~Data() {}
	//Input: 从RawDataReader中读入数据并获取标准标注
	void Input(RawDataReader* raw_data_reader);
	int size() { return strings_.size(); }
	vector<string> instance(int idx) { return strings_[idx]; }
	vector<int> tag(int idx) { return tags_[idx]; }
	bool FindInDict(const string& word) { return dict_.find(word) != dict_.end();  }
	int ComputeMaxWordLength() {
		int len = 0;
		for (set<string>::iterator it = dict_.begin(); it != dict_.end(); it++) {
			len = max(len, (int)it->length());
		}
		return len;
	}
	//Output: DEBUG ONLY. 输出第i条句子的tag.
	void Output(int idx);
private:
	vector< vector<string> > strings_;
	vector< vector<int> > tags_;
	set< string > dict_;
};

class RawDataReader {
public:
	RawDataReader () {}
	RawDataReader(const char* file_name) {
		input_file_name_ = file_name;
	}
	~RawDataReader() {}
	//Input: 读入数据并计算标准标注
	void Input(vector< vector<int> >* tags, vector< vector<string> >* strings, set<string>* dict);
private:
	string input_file_name_;
};