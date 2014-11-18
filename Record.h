#ifndef __RECORD_H__
#define __RECORD_H__

#include "Definition.h"
#include "Buffer.h"		//����bufferģ���޸�
#include <stdio.h>

extern Buffer bufferManager;

//�����������block����
int blockMore(int totalLength, int recordNum, int occupied);
//�ж��Ƿ����
bool isFull(int totalLength, int occupied);
//�ж��Ƿ�Ϊ�ռ�¼
bool emptyRecord(char* block, int length);
//�����������ݵıȽ�
int compareStr(string value,string str2);
int compareInt(string int1,string int2);
int compareFloat(string float1,string float2);
bool compare(string s1, string s2, int type, int condType);
//��ȡ���ݼ�¼
void getData(string DB_Name, TableInfo table, Data& data);
void getIndexData(string DB_Name, TableInfo table,vector<Result> results, Data& data);
//��������indexģ������info����
void insertRecord(string DB_Name, TableInfo table, Data insertedValues, int& record_Num, vector<IndexInfo>& indexValues);
//��ʼ��
void initialIndex(string DB_Name, TableInfo table, string Attr_Name, vector<IndexInfo>& indexValues);
//û������select
void selectRecord(string DB_Name, vector<string>& columns, TableInfo table, vector<Condition>& conds, Data& dataOut);
//û������delete
void deletRecord(string DB_Name, TableInfo table, vector<Condition>& conds, int& affectedNum);
//��������select
void printSelectedRecord(string DB_Name, vector<string>& columns, TableInfo table, vector<Condition>& conds, Data& dataOut, vector<Result> results);
//��������delete
void deleteIndexRecord(string DB_Name, TableInfo table, vector<Condition>& conds,  vector<Result> results, int& affectedNum);
#endif
