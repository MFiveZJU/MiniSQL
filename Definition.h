#ifndef __DEFINITION_H__
#define __DEFINITION_H__

#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>

#define DATAFILE 0
#define INT 1
#define FLOAT 2
#define CHAR 3

#define FILLEMPTY "0"
#define BLOCKSIZE 4096			//the size of the block
	
#define MAX_FILE_ACTIVE	5		//limit the active files in the buffer
#define MAX_BLOCK		50 		//the max number of blocks

using namespace std;

class Result
{
public:
	int blockNum;
	vector<int> offsets;
	result():blockNum(-1);
	~result();
};

class Record   //Record类，用来存储一条记录
{
public:
    vector<string> columns;
    int blockNum;
    int offset;    
};

class Data
{
public:
    vector<Record> records;
    vector<int> location;//???
};

class AttrInfo
{
public:
    string name;
	int type;
	int length;
	bool isPrimeryKey;
	bool isUnique;
	string indexName;
	AttrInfo():isPrimeryKey(false),isUnique(false){};
	AttrInfo(string a_name, int a_type, int a_length, bool isP, bool isU):name(a_name), type(a_type), length(a_length), isPrimeryKey(isP), isUnique(isU){};
};

class TableInfo
{
public:
	string name;
	int attrNum;	//表中含有的属性数
	//int blockNum;
	int totalLength;	//值为sum(attributes[i].length)
	vector<AttrInfo> attributes;
	//TableInfo():attrNum(0), blockNum(-1), totalLength(0){};
	TableInfo():attrNum(0),totalLength(0){};
	~TableInfo(){};
};

class Index //存在catalog里面
{
public:
	string name;
	string tableName;
	string attrName;
	//int blockNum;
	//IndexInfo():blockNum(-1){};
	Index(){};
	~Index(){};
};

class IndexInfo: class Index //B+TreeNode
{
public:
	int blockNum;
	int offset;
	int type;
	int length;
	string value;
	IndexInfo():blockNum(-1),offset(-1){};
	~IndexInfo(){};
};

class BlockInfo
{
public:
	int blockNum;
	bool dirtyBit;
	BlockInfo* next;
	fileInfo* file;
	int charNum;	//initial to be 0
	char* cBlock;
	int iTime;
	int lock;
	int father;
	bool isFull;	//initial to be false
	BlockInfo();
	~BlockInfo();
	void clearBlock();
};

class fileInfo{
public:
	int type;					//0 for Data File
								//1 for Index File
	string fileName;			//the name of the file
	int recordAmount;			//the number of the record in th file
	int freeNum;				//the free block number which could be used for the file
	fileInfo* next;				//the pointer points to the next file
	BlockInfo* firstBlock;		//point to the first blcok within the file
	fileInfo();
	~fileInfo();
};

//stants for less than, less equal, greater than, greater equal, equal, not equal respectivly
class Condition
{
public:
    int op;
    int columnNum;
    string columname;
    string value;
};

#endif
