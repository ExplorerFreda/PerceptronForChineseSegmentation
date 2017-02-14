#include"stdafx.h"
#include"Data.h"
#include<sstream>
#include<cassert>

using namespace std;

void Data::Input(RawDataReader* raw_data_reader) {
	raw_data_reader->Input(&tags_, &strings_, &dict_);
}

void Data::Output(int idx) {
	for (int i = 0; i < (int)strings_[idx].size(); i++) {
		cout << strings_[idx][i] << tags_[idx][i] << endl;
	}
}

void RawDataReader::Input(vector< vector<int> >* tags, vector< vector<string> >* strings, set<string>* dict) {
	ifstream fin(input_file_name_.c_str());
	string str;
	vector<string> str_processed;
	vector<int> tag;
	while (getline(fin, str)) {
		stringstream ss(str);
		string word;
		while (ss >> word) {
			dict->insert(word);
		}

		while (str.length() > 0 && str[str.length() - 1] == ' ') {
			str = str.substr(0, str.length() - 1);
		}
		while (str.length() > 0 && str[0] == ' ') {
			str = str.substr(1, str.length() - 1);
		}
		if (!str.length()) {
			continue;
		}
		str_processed.clear();
		tag.clear();
		str_processed.push_back("xxx");
		tag.push_back(-1);
		str_processed.push_back("xxx");
		tag.push_back(-1); //在句子前加入两个无意义单词避免越界
		for (int i = 0, del = 2; i < (int)str.length(); i += del) {
			if (i == 0) {
				if ((char)str[i] < 0) {
					str_processed.push_back(str.substr(i, 2));
					tag.push_back(START);
					del = 2;
				}
				else {
					str_processed.push_back(str.substr(i, 1));
					tag.push_back(START);
					del = 1;
				}
			}
			else if (str[i] == ' ') {
				tag[tag.size() - 1] |= END;
				i += 2;
				if ((char)str[i] < 0) {
					str_processed.push_back(str.substr(i, 2));
					tag.push_back(START);
					del = 2;
				}
				else {
					str_processed.push_back(str.substr(i, 1));
					tag.push_back(START);
					del = 1;
				}
			}
			else {
				if ((char)str[i] < 0) {
					str_processed.push_back(str.substr(i, 2));
					tag.push_back(NORMAL);
					del = 2;
				}
				else {
					str_processed.push_back(str.substr(i, 1));
					tag.push_back(NORMAL);
					del = 1;
				}
			}
		}
		tag[tag.size() - 1] |= END;
		str_processed.push_back("xxx");
		tag.push_back(-1);
		str_processed.push_back("xxx");
		tag.push_back(-1); //在句子后加入两个无意义单词避免越界
		tags->push_back(tag);
		strings->push_back(str_processed);
	}
	fin.close();
}