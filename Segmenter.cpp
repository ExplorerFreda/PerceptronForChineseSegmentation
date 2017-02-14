#include"stdafx.h"
#include"Segmenter.h"
#include<fstream>
#include<algorithm>
const int MAX_WORD_LENGTH = 10; //最长的词长度，在维特比算法中使用

void SPSegmenter::Train(Data* train_data) {
	int it_num;
	int n = train_data->size();
	for (it_num = 0; it_num < max_iteration_; it_num++) {
		cout << "Iteration: " << it_num << ": " << endl;
		bool train_done = Iterate(train_data);
		for (map<string, double>::iterator it = weight_.begin(); it != weight_.end(); it++) {
			global_weight_[it->first] += it->second;
		}
		if (train_done) {
			break;
		}
	}
	ofstream fout("data\\sp_model.txt");
	for (map<string, double>::iterator it = global_weight_.begin(); it != global_weight_.end(); it++) {
		fout << it->first << " " << global_weight_[it->first] << endl;
	}
	fout.close();
}

bool SPSegmenter::CheckPunc(const string& word) {
	if (word.length() < 2) return ispunct(word[0]) == 1;
	return (unsigned char)word[0] <= 175;
}

int SPSegmenter::WordType(const string& word) {
	if (word.length() == 1) {
		if (ispunct(word[0])) {
			return 0;
		}
		else {
			return 1;
		}
	}
	if (word.length() == 3) return -1;
	if ((unsigned char)word[0] <= 175) {
		return 2;
	}
	else {
		return 3;
	}
}

vector< vector<string> > SPSegmenter::GenerateFeatures(vector<string>* words) {
	vector< vector<string> > features;
	char temp_feature[100];
	for (int i = 2; i < (int)words->size() - 2; i++) {
		vector<string> curr_features;
		curr_features.clear();
		for (int j = -2; j < 3; j++) {
			sprintf(temp_feature, "C%d%s", j, words->at(i + j).c_str());
			curr_features.push_back(temp_feature); //C(n)
		}
		for (int j = -2; j < 2; j++) {
			sprintf(temp_feature, "P%d%s%s", j, words->at(i + j).c_str(), words->at(i + j + 1).c_str());
			curr_features.push_back(temp_feature); //C(n)C(n+1)
		}
		sprintf(temp_feature, "A%s%s", words->at(i - 1).c_str(), words->at(i + 1).c_str());
		curr_features.push_back(temp_feature); //C(idx-1)C(idx+1)
		sprintf(temp_feature, "p%d", CheckPunc(words->at(i)));
		curr_features.push_back(temp_feature); //Punc(C(idx))
		sprintf(temp_feature, "T%d%d%d%d%d", WordType(words->at(i - 2)),
			WordType(words->at(i - 1)), WordType(words->at(i)), WordType(words->at(i + 1)), WordType(words->at(i + 2)));
		curr_features.push_back(temp_feature); //Tags of idx-2,idx-1,...,idx+2
		features.push_back(curr_features);
	}
	return features;
}

bool SPSegmenter::Iterate(Data* train_data) {
	int n = train_data->size();
	vector<int> ord(n);
	for (int i = 0; i < n; i++) ord[i] = i;
	random_shuffle(ord.begin(), ord.end()); //每次迭代按随机顺序访问训练用例
	int error_words = 0; //错误标注的字数
	int error_instance = 0; //存在错误标注的训练用例数
	int total_words = 0;
	for (int i = 0; i < n; i++) {
		if (i % 1000 == 0) {
			cout << i << " " << weight_.size() << endl;
		}
		int idx = ord[i];
		vector<string> words = train_data->instance(idx);
		vector<int> tags = train_data->tag(idx);
		vector< vector<string> > features = GenerateFeatures(&words); //生成当前训练用例的特征
		vector<int> results = SegmentByFeature(&features);
		int error_num = Update(&tags, &results, &features);
		error_words += error_num;
		error_instance += (error_num != 0);
		total_words += words.size();
	}
	double instance_accurary = 1 - (double)error_instance / n;
	double word_accurary = 1 - (double)error_words / total_words;
	printf("Word Accurary: %.4lf%%, Instance Accurary: %.4lf%%\n", word_accurary * 100, instance_accurary * 100);
	return error_words == 0;
}

int SPSegmenter::Update(vector<int>* tags, vector<int>* results, vector< vector<string> >* features) {
	int error_num = 0;
	int num = tags->size();
	for (int i = 2; i < num - 2; i++) {
		if (tags->at(i) == results->at(i)) {
			continue;	
		}
		error_num++;
		vector<string> pos_feature = features->at(i - 2);
		int correct = tags->at(i);
		int current = results->at(i);
		for (int j = 0; j < (int)pos_feature.size(); j++) {
			weight_[pos_feature.at(j) + ">" + (char)(correct + '0')] += 1.0;
			weight_[pos_feature.at(j) + ">" + (char)(current + '0')] -= 1.0;
		}
	}
	return error_num;
}

vector<int> SPSegmenter::SegmentByFeature(vector< vector<string> >* features) {
	int num = features->size() + 4;
	vector<double*> value(num);
	for (int i = 0; i < num; i++) {
		value[i] = new double[4];
	}
	for (int i = 2; i < num - 2; i++) {
		value[i] = new double[4];
		for (int j = 0; j < 4; j++) {
			value[i][j] = ComputeScore(i-2, j, features);
		}
	}
	vector<int> tags(num);

	//使用维特比算法得到最优路径
	double *max_score = new double[num];
	int *length = new int[num];
	for (int i = 0; i < num; i++) {
		max_score[i] = 0;
		length[i] = -1;
	}
	for (int i = num - 2 - 1; i >= 2; i--) {
		for (int j = 1; j < MAX_WORD_LENGTH; j++) { //尝试每种长度的词，假设词长度最多为10
			if (i + j > num - 2) {
				break;
			}
			double current_score = max_score[i + j];
			MakeTag(i, j, &tags);
			for (int k = i; k < i + j; k++) {
				current_score += value[k][tags[k]];
			}
			if (length[i] == -1 || current_score > max_score[i]) {
				max_score[i] = current_score;
				length[i] = j;
			}
		}
	}
	for (int i = 2; i < num - 2; i+=length[i]) { //最后找到的最优结果
		MakeTag(i, length[i], &tags);
	}

	for (int i = 0; i < num; i++) {
		delete[] value[i];
	}
	delete[] length;
	delete[] max_score;
	return tags;
}

double SPSegmenter::ComputeScore(int idx, int type, vector< vector<string> >* features) {
	vector<string> pos_feature = features->at(idx);
	double score = 0;
	for (int i = 0; i < (int)pos_feature.size(); i++) {
		if (weight_.find(pos_feature.at(i) + ">" + (char)(type + '0')) != weight_.end()) {
			score += weight_[pos_feature.at(i) + ">" + (char)(type + '0')];
		}
	}
	return score;
}

void SPSegmenter::MakeTag(int pos, int length, vector<int>* tags) {
	(*tags)[pos] = START;
	for (int i = 1; i < length; i++) {
		(*tags)[pos + i] = NORMAL;
	}
	(*tags)[pos + length - 1] |= END;
}

void SPSegmenter::LoadModel(const char* file_name) {
	ifstream fin(file_name);
	weight_.clear();
	string str; 
	double score;
	while (fin >> str >> score) {
		weight_[str] = score;
	}
	fin.close();
}

void SPSegmenter::Test(const char* test_file_name, const char* answer_file_name) {
	ifstream testin(test_file_name);
	ifstream answerin(answer_file_name);
	ofstream fout("result\\sp_segment.txt");
	string str;
	double precision_correct = 0, precision_total = 0;
	double recall_correct = 0, recall_total = 0;
	int test_case = 0;
	while (testin >> str) {
		vector<string> words;
		words.clear();
		words.push_back("xxx");
		words.push_back("xxx");
		for (int i = 0; i < str.length(); i++) {
			if (str[i] < 0) {
				words.push_back(str.substr(i, 2));
				i++;
			}
			else {
				words.push_back(str.substr(i, 1));
			}
		}
		words.push_back("xxx");
		words.push_back("xxx");
		vector< vector<string> > features = GenerateFeatures(&words);
		vector<int> tags = SegmentByFeature(&features);
		
		multiset<string> result;
		result.clear();
		string temp_word="";
		for (int i = 2; i < tags.size() - 2; i++) {
			if (tags[i] == 1) {
				temp_word = words[i];
			}
			else if (tags[i] == 0) {
				temp_word = temp_word + words[i];
			}
			else if (tags[i] == 2) {
				temp_word = temp_word + words[i];
				result.insert(temp_word);
				fout << temp_word << "  ";
			} 
			else if (tags[i] == 3) {
				temp_word = words[i];
				result.insert(temp_word);
				fout << temp_word << "  ";
			}
		}
		fout << endl;
		multiset<string> answer;
		answer.clear();
		string buffer;
		getline(answerin, buffer);
		stringstream ss;
		ss << buffer;
		while (ss >> str) {
			answer.insert(str);
		}
		
		//Precision
		int correct = 0, total = 0;;
		for (multiset<string>::iterator it = result.begin(); it != result.end(); it++) {
			if (answer.find(*it) != answer.end()) {
				correct++;
			}
			total++;
		}
		precision_correct += correct;
		precision_total += total;

		//Recall
		correct = 0, total = 0;
		for (multiset<string>::iterator it = answer.begin(); it != answer.end(); it++) {
			if (result.find(*it) != result.end()) {
				correct++;
			}
			total++;
		}
		recall_correct += correct;
		recall_total += total;

		cout << "Test Case: " << ++ test_case << " done." << endl;
	}

	double precision = precision_correct / precision_total;
	double recall = recall_correct / recall_total;
	double Fscore = 2 * precision * recall / (precision + recall);
	printf("Precision: %.4lf\nRecall: %.4lf\nFScore: %.4lf\n", precision, recall, Fscore);
	testin.close();
	answerin.close();
	fout.close();
}
//=======================以上是SPSegmenter的实现=======================
//====================================================================

void NSPSegmenter::Train(Data* train_data) {
	int it_num;
	int n = train_data->size();
	for (it_num = 0; it_num < max_iteration_; it_num++) {
		cout << "Iteration: " << it_num << ": " << endl;
		bool train_done = Iterate(train_data);
		for (map<string, double>::iterator it = weight_.begin(); it != weight_.end(); it++) {
			global_weight_[it->first] += it->second;
		}
		if (train_done) {
			break;
		}
	}
	ofstream fout("data\\nsp_model.txt");
	for (map<string, double>::iterator it = global_weight_.begin(); it != global_weight_.end(); it++) {
		fout << it->first << " " << global_weight_[it->first] << endl;
	}
	fout.close();
}

bool NSPSegmenter::CheckPunc(const string& word) {
	if (word.length() < 2) return ispunct(word[0]) == 1;
	return (unsigned char)word[0] <= 175;
}

int NSPSegmenter::WordType(const string& word) {
	if (word.length() == 1) {
		if (ispunct(word[0])) {
			return 0;
		}
		else {
			return 1;
		}
	}
	if (word.length() == 3) return -1;
	if ((unsigned char)word[0] <= 175) {
		return 2;
	}
	else {
		return 3;
	}
}

vector< vector<string> > NSPSegmenter::GenerateFeatures(vector<string>* words) {
	vector< vector<string> > features;
	char temp_feature[100];
	for (int i = 2; i < (int)words->size() - 2; i++) {
		vector<string> curr_features;
		curr_features.clear();
		for (int j = -2; j < 3; j++) {
			sprintf(temp_feature, "C%d%s", j, words->at(i + j).c_str());
			curr_features.push_back(temp_feature); //C(n)
		}
		for (int j = -2; j < 2; j++) {
			sprintf(temp_feature, "P%d%s%s", j, words->at(i + j).c_str(), words->at(i + j + 1).c_str());
			curr_features.push_back(temp_feature); //C(n)C(n+1)
		}
		sprintf(temp_feature, "A%s%s", words->at(i - 1).c_str(), words->at(i + 1).c_str());
		curr_features.push_back(temp_feature); //C(idx-1)C(idx+1)
		sprintf(temp_feature, "p%d", CheckPunc(words->at(i)));
		curr_features.push_back(temp_feature); //Punc(C(idx))
		sprintf(temp_feature, "T%d%d%d%d%d", WordType(words->at(i - 2)),
			WordType(words->at(i - 1)), WordType(words->at(i)), WordType(words->at(i + 1)), WordType(words->at(i + 2)));
		curr_features.push_back(temp_feature); //Tags of idx-2,idx-1,...,idx+2
		features.push_back(curr_features);
	}
	return features;
}

bool NSPSegmenter::Iterate(Data* train_data) {
	int n = train_data->size();
	vector<int> ord(n);
	for (int i = 0; i < n; i++) ord[i] = i;
	random_shuffle(ord.begin(), ord.end()); //每次迭代按随机顺序访问训练用例
	int error_words = 0; //错误标注的字数
	int error_instance = 0; //存在错误标注的训练用例数
	int total_words = 0;
	for (int i = 0; i < n; i++) {
		if (i % 1000 == 0) {
			cout << i << " " << weight_.size() << endl;
		}
		int idx = ord[i];
		vector<string> words = train_data->instance(idx);
		vector<int> tags = train_data->tag(idx);
		vector< vector<string> > features = GenerateFeatures(&words); //生成当前训练用例的特征
		vector<int> results = SegmentByFeature(&features);
		int error_num = Update(&tags, &results, &features);
		error_words += error_num;
		error_instance += (error_num != 0);
		total_words += words.size();
	}
	double instance_accurary = 1 - (double)error_instance / n;
	double word_accurary = 1 - (double)error_words / total_words;
	printf("Word Accurary: %.4lf%%, Instance Accurary: %.4lf%%\n", word_accurary * 100, instance_accurary * 100);
	return error_words == 0;
}

int NSPSegmenter::Update(vector<int>* tags, vector<int>* results, vector< vector<string> >* features) {
	int error_num = 0;
	int num = tags->size();
	for (int i = 2; i < num - 2; i++) {
		if (tags->at(i) == results->at(i)) {
			continue;
		}
		error_num++;
		vector<string> pos_feature = features->at(i - 2);
		int correct = tags->at(i);
		int current = results->at(i);
		for (int j = 0; j < (int)pos_feature.size(); j++) {
			weight_[pos_feature.at(j) + ">" + (char)(correct + '0')] += 1.0;
			weight_[pos_feature.at(j) + ">" + (char)(current + '0')] -= 1.0;
		}
	}
	return error_num;
}

vector<int> NSPSegmenter::SegmentByFeature(vector< vector<string> >* features) {
	int num = features->size() + 4;
	vector<int> tags(num);
	for (int i = 2; i < num - 2; i++) {
		double value[4];
		int pos = -1;
		for (int j = 0; j < 4; j++) {
			value[j] = ComputeScore(i - 2, j, features);
			if (pos == -1 || value[j] > value[pos]) {
				pos = j;
			}
		}
		tags[i] = pos;
	}
	return tags;
}

double NSPSegmenter::ComputeScore(int idx, int type, vector< vector<string> >* features) {
	vector<string> pos_feature = features->at(idx);
	double score = 0;
	for (int i = 0; i < (int)pos_feature.size(); i++) {
		if (weight_.find(pos_feature.at(i) + ">" + (char)(type + '0')) != weight_.end()) {
			score += weight_[pos_feature.at(i) + ">" + (char)(type + '0')];
		}
	}
	return score;
}

void NSPSegmenter::LoadModel(const char* file_name) {
	ifstream fin(file_name);
	weight_.clear();
	string str;
	double score;
	while (fin >> str >> score) {
		weight_[str] = score;
	}
	fin.close();
}

void NSPSegmenter::Test(const char* test_file_name, const char* answer_file_name) {
	ifstream testin(test_file_name);
	ifstream answerin(answer_file_name);
	string str;
	double precision_correct = 0, precision_total = 0;
	double recall_correct = 0, recall_total = 0;
	int test_case = 0;
	ofstream fout("result\\nsp_result.txt");
	while (testin >> str) {
		vector<string> words;
		words.clear();
		words.push_back("xxx");
		words.push_back("xxx");
		for (int i = 0; i < str.length(); i++) {
			if (str[i] < 0) {
				words.push_back(str.substr(i, 2));
				i++;
			}
			else {
				words.push_back(str.substr(i, 1));
			}
		}
		words.push_back("xxx");
		words.push_back("xxx");
		vector< vector<string> > features = GenerateFeatures(&words);
		vector<int> tags = SegmentByFeature(&features);

		multiset<string> result;
		result.clear();
		string temp_word = "";
		for (int i = 2; i < tags.size() - 2; i++) {
			if (tags[i] == 1) {
				temp_word = words[i];
			}
			else if (tags[i] == 0) {
				temp_word = temp_word + words[i];
			}
			else if (tags[i] == 2) {
				temp_word = temp_word + words[i];
				result.insert(temp_word);
				fout << temp_word << "  ";
			}
			else if (tags[i] == 3) {
				temp_word = words[i];
				result.insert(temp_word);
				fout << temp_word << "  ";
			}
		}
		fout << endl;
		multiset<string> answer;
		answer.clear();
		string buffer;
		getline(answerin, buffer);
		stringstream ss;
		ss << buffer;
		while (ss >> str) {
			answer.insert(str);
		}

		//Precision
		int correct = 0, total = 0;;
		for (multiset<string>::iterator it = result.begin(); it != result.end(); it++) {
			if (answer.find(*it) != answer.end()) {
				correct++;
			}
			total++;
		}
		precision_correct += correct;
		precision_total += total;

		//Recall
		correct = 0, total = 0;
		for (multiset<string>::iterator it = answer.begin(); it != answer.end(); it++) {
			if (result.find(*it) != result.end()) {
				correct++;
			}
			total++;
		}
		recall_correct += correct;
		recall_total += total;

		cout << "Test Case: " << ++test_case << " done." << endl;
	}

	double precision = precision_correct / precision_total;
	double recall = recall_correct / recall_total;
	double Fscore = 2 * precision * recall / (precision + recall);
	printf("Precision: %.4lf\nRecall: %.4lf\nFScore: %.4lf\n", precision, recall, Fscore);
	testin.close();
	answerin.close();
	fout.close();
}

//=======================以上是NSPSegmenter的实现======================
//====================================================================

void FMMSegmenter :: Test(const char* test_file_name, const char* answer_file_name) {
	ifstream testin(test_file_name);
	ifstream answerin(answer_file_name);
	ofstream fout("result\\fmm_segment.txt");
	string str;
	double precision_correct = 0, precision_total = 0;
	double recall_correct = 0, recall_total = 0;
	int test_case = 0;
	int max_len = data_->ComputeMaxWordLength();
	while (testin >> str) {
		vector<string> words;
		words.clear();
		for (int i = 0; i < str.length(); i++) {
			if (str[i] < 0) {
				words.push_back(str.substr(i, 2));
				i++;
			}
			else {
				words.push_back(str.substr(i, 1));
			}
		}
		multiset<string> result;
		result.clear();
		for (int i = 0; i < words.size();) {
			bool flag = false;
			for (int j = min(max_len, (int)words.size() - i); j >= 0; j--) {
				string temp_word = "";
				for (int k = 0; k < j; k++) {
					temp_word += words[i + k];
				}
				if (data_->FindInDict(temp_word)) {
					result.insert(temp_word);
					fout << temp_word << "  ";
					flag = true;
					i += j;
					break;
				}
			}
			if (!flag) {
				fout << words[i] << "  ";
				result.insert(words[i++]);
			}
		}
		fout << endl;

		multiset<string> answer;
		answer.clear();
		string buffer;
		getline(answerin, buffer);
		stringstream ss;
		ss << buffer;
		while (ss >> str) {
			answer.insert(str);
		}

		//Precision
		int correct = 0, total = 0;;
		for (multiset<string>::iterator it = result.begin(); it != result.end(); it++) {
			if (answer.find(*it) != answer.end()) {
				correct++;
			}
			total++;
		}
		precision_correct += correct;
		precision_total += total;

		//Recall
		correct = 0, total = 0;
		for (multiset<string>::iterator it = answer.begin(); it != answer.end(); it++) {
			if (result.find(*it) != result.end()) {
				correct++;
			}
			total++;
		}
		recall_correct += correct;
		recall_total += total;

		cout << "Test Case: " << ++test_case << " done." << endl;
	}

	double precision = precision_correct / precision_total * 100;
	double recall = recall_correct / recall_total * 100;
	double Fscore = 2 * precision * recall / (precision + recall);
	printf("Precision: %.4lf\nRecall: %.4lf\nFScore: %.4lf\n", precision, recall, Fscore);
	testin.close();
	answerin.close();
}