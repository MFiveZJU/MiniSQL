#ifndef __DEFINITION_H__
#define __DEFINITION_H__

#include <string.h>
#include <vector>
#include <iostream>

using namespace std;

class attr_info
{
public:
    string name;
	int type;
	int length;
	bool isPrimeryKey;
	bool isUnique;
	string indexName;
	attr_info():isPrimeryKey(false),isUnique(false){};
	attr_info(string a_name, int a_type, int a_length, bool isP, bool isU):name(a_name), type(a_type), length(a_length), isPrimeryKey(isP), isUnique(isU){};
};

class table_info
{
public:
	string name;
	int attrNum;	//表中含有的属性数
	int blockNum;
	int totalLength;	//值为sum(attributes[i].length)
	vector<attr_info> attributes;
	table_info():attrNum(0), blockNum(0), totalLength(0){};
};

class index_info
{
public:
	string name;
	string tableName;
	string attrName;
	int blockNum;
	index_info():blockNum(0){};
	~index_info(){};
};

class blockInfo
{
public:
	int blockNum;
	bool dirtyBit;
	blockInfo* next;
	fileInfo* file;
	int charNum;
	char* cBlock;
	int iTime;
	int lock;
	int father;
	blockInfo(){};
	~blockInfo(){};
};

#endif
