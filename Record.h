#ifndef __RECORD_H__
#define __RECORD_H__

#include "Definition.h"
#include "Buffer.h"		//根据buffer模块修改
#include <stdio.h>

extern Buffer bufferManager;

//计算插入所需block数量
int blockMore(int totalLength, int recordNum, int occupied);
//判断是否插满
bool isFull(int totalLength, int occupied);
//判断是否为空记录
bool emptyRecord(char* block, int length);
//各种类型数据的比较
int compareStr(string value,string str2);
int compareInt(string int1,string int2);
int compareFloat(string float1,string float2);
bool compare(string s1, string s2, int type, int condType);
//获取数据记录
void getData(string DB_Name, TableInfo table, Data& data);
void getIndexData(string DB_Name, TableInfo table,vector<Result> results, Data& data);
//包含生成index模块所需info容器
void insertRecord(string DB_Name, TableInfo table, Data insertedValues, int& record_Num, vector<IndexInfo>& indexValues);
//初始化
void initialIndex(string DB_Name, TableInfo table, string Attr_Name, vector<IndexInfo>& indexValues);
//没索引的select
void selectRecord(string DB_Name, vector<string>& columns, TableInfo table, vector<Condition>& conds, Data& dataOut);
//没索引的delete
void deletRecord(string DB_Name, TableInfo table, vector<Condition>& conds, int& affectedNum);
//有索引的select
void printSelectedRecord(string DB_Name, vector<string>& columns, TableInfo table, vector<Condition>& conds, Data& dataOut, vector<Result> results);
//有索引的delete
void deleteIndexRecord(string DB_Name, TableInfo table, vector<Condition>& conds,  vector<Result> results, int& affectedNum);
#endif
