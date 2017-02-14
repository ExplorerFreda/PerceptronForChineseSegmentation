#pragma once
#include"Data.h"
#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<cctype>
#include<cstring>
#include<sstream>
#include<fstream>
#include<set>
#include<map>
using namespace std;

//结构化感知器
class SPSegmenter {
public:
	SPSegmenter() {}
	SPSegmenter(int max_iteration) :max_iteration_(max_iteration) {}
	void Train(Data* train_data);
	void LoadModel(const char* file_name);
	void Test(const char* test_file_name, const char* answer_file_name);
private:
	bool Iterate(Data* train_data);
	//CheckPunc: 判断是否为标点符号
	bool CheckPunc(const string& word);
	//GenerateFeatures: 根据模板构造特征
	vector< vector<string> > GenerateFeatures(vector<string>* words);
	//WordType: 标记一个字符的类型
	int WordType(const string& word);
	//SegmentByFeature: 根据当前参数和特征进行分词，同时用于训练和测试
	vector<int> SegmentByFeature(vector< vector<string> >* features);
	//MakeTag: 将[pos,pos+length)标记为一个词
	void MakeTag(int pos, int length, vector<int>*tag);
	//ComputeScore: 给定某位置相关的特征，求该位置标记为type的分数
	double ComputeScore(int idx, int type, vector< vector<string> >* features);
	//Correct: 训练过程中根据特征使用标准答案tags对结果为results的序列标记结果的参数进行更新，返回错误标记个数
	int Update(vector<int>* tags, vector<int>* results, vector< vector<string> >* features);
	int max_iteration_;
	map<string, double> weight_;
	map<string, double> global_weight_;
};

//非结构化感知器
class NSPSegmenter {
public:
	NSPSegmenter() {}
	NSPSegmenter(int max_iteration) :max_iteration_(max_iteration) {}
	void Train(Data* train_data);
	void LoadModel(const char* file_name);
	void Test(const char* test_file_name, const char* answer_file_name);
private:
	bool Iterate(Data* train_data);
	//CheckPunc: 判断是否为标点符号
	bool CheckPunc(const string& word);
	//GenerateFeatures: 根据模板构造特征
	vector< vector<string> > GenerateFeatures(vector<string>* words);
	//WordType: 标记一个字符的类型
	int WordType(const string& word);
	//SegmentByFeature: 根据当前参数和特征进行分词，同时用于训练和测试
	vector<int> SegmentByFeature(vector< vector<string> >* features);
	//MakeTag: 将[pos,pos+length)标记为一个词
	void MakeTag(int pos, int length, vector<int>*tag);
	//ComputeScore: 给定某位置相关的特征，求该位置标记为type的分数
	double ComputeScore(int idx, int type, vector< vector<string> >* features);
	//Correct: 训练过程中根据特征使用标准答案tags对结果为results的序列标记结果的参数进行更新，返回错误标记个数
	int Update(vector<int>* tags, vector<int>* results, vector< vector<string> >* features);
	int max_iteration_;
	map<string, double> weight_;
	map<string, double> global_weight_;
};

//正向最大匹配算法
class FMMSegmenter {
public:
	FMMSegmenter() {}
	FMMSegmenter(Data* data) :data_(data) {}
	void Test(const char* test_file_name, const char* answer_file_name);
private:
	Data* data_;
};