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

//�ṹ����֪��
class SPSegmenter {
public:
	SPSegmenter() {}
	SPSegmenter(int max_iteration) :max_iteration_(max_iteration) {}
	void Train(Data* train_data);
	void LoadModel(const char* file_name);
	void Test(const char* test_file_name, const char* answer_file_name);
private:
	bool Iterate(Data* train_data);
	//CheckPunc: �ж��Ƿ�Ϊ������
	bool CheckPunc(const string& word);
	//GenerateFeatures: ����ģ�幹������
	vector< vector<string> > GenerateFeatures(vector<string>* words);
	//WordType: ���һ���ַ�������
	int WordType(const string& word);
	//SegmentByFeature: ���ݵ�ǰ�������������зִʣ�ͬʱ����ѵ���Ͳ���
	vector<int> SegmentByFeature(vector< vector<string> >* features);
	//MakeTag: ��[pos,pos+length)���Ϊһ����
	void MakeTag(int pos, int length, vector<int>*tag);
	//ComputeScore: ����ĳλ����ص����������λ�ñ��Ϊtype�ķ���
	double ComputeScore(int idx, int type, vector< vector<string> >* features);
	//Correct: ѵ�������и�������ʹ�ñ�׼��tags�Խ��Ϊresults�����б�ǽ���Ĳ������и��£����ش����Ǹ���
	int Update(vector<int>* tags, vector<int>* results, vector< vector<string> >* features);
	int max_iteration_;
	map<string, double> weight_;
	map<string, double> global_weight_;
};

//�ǽṹ����֪��
class NSPSegmenter {
public:
	NSPSegmenter() {}
	NSPSegmenter(int max_iteration) :max_iteration_(max_iteration) {}
	void Train(Data* train_data);
	void LoadModel(const char* file_name);
	void Test(const char* test_file_name, const char* answer_file_name);
private:
	bool Iterate(Data* train_data);
	//CheckPunc: �ж��Ƿ�Ϊ������
	bool CheckPunc(const string& word);
	//GenerateFeatures: ����ģ�幹������
	vector< vector<string> > GenerateFeatures(vector<string>* words);
	//WordType: ���һ���ַ�������
	int WordType(const string& word);
	//SegmentByFeature: ���ݵ�ǰ�������������зִʣ�ͬʱ����ѵ���Ͳ���
	vector<int> SegmentByFeature(vector< vector<string> >* features);
	//MakeTag: ��[pos,pos+length)���Ϊһ����
	void MakeTag(int pos, int length, vector<int>*tag);
	//ComputeScore: ����ĳλ����ص����������λ�ñ��Ϊtype�ķ���
	double ComputeScore(int idx, int type, vector< vector<string> >* features);
	//Correct: ѵ�������и�������ʹ�ñ�׼��tags�Խ��Ϊresults�����б�ǽ���Ĳ������и��£����ش����Ǹ���
	int Update(vector<int>* tags, vector<int>* results, vector< vector<string> >* features);
	int max_iteration_;
	map<string, double> weight_;
	map<string, double> global_weight_;
};

//�������ƥ���㷨
class FMMSegmenter {
public:
	FMMSegmenter() {}
	FMMSegmenter(Data* data) :data_(data) {}
	void Test(const char* test_file_name, const char* answer_file_name);
private:
	Data* data_;
};