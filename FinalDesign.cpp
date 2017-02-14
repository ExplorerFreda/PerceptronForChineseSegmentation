// FinalDesign.cpp : 定义控制台应用程序的入口点。
//

#include"stdafx.h"
#include"Data.h"
#include"Segmenter.h"
#include<iostream>
#include<cstdio>

using namespace std;

int main()
{
	RawDataReader* raw_data_reader = new RawDataReader("data\\train.txt");
	Data* data = new Data();
	data->Input(raw_data_reader);
	SPSegmenter* sp_segmenter = new SPSegmenter(10);
//	sp_segmenter->Train(data);
//  如果需要加载模型，则注释上一行，取消下一行的注释
	sp_segmenter->LoadModel("data\\sp_model.txt");
	sp_segmenter->Test("data\\test.txt", "data\\test.answer.txt");
	delete sp_segmenter;

	NSPSegmenter *nsp_segmenter = new NSPSegmenter(10);
//	nsp_segmenter->Train(data);
//  如果需要加载模型，则注释上一行，取消下一行的注释
	nsp_segmenter->LoadModel("data\\nsp_model.txt");
	nsp_segmenter->Test("data\\test.txt", "data\\test.answer.txt");
	delete nsp_segmenter;

	FMMSegmenter* fmm_segmenter = new FMMSegmenter(data);
	fmm_segmenter->Test("data\\test.txt", "data\\test.answer.txt");
	delete fmm_segmenter;

	
	delete data;
	delete raw_data_reader;
    return 0;
}

