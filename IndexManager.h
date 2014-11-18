/*
 * IndexManager.h
 *
 *  Created on: 2014年10月31日
 *      Author: Chong Xiaoya
 */

#ifndef INDEX_MANAGER_H_
#define INDEX_MANAGER_H_

#include<iostream>
#include<CString>
#include<vector>

#include"Buffer.h"
#include"Record.h"

using namespace std;
#define LT 100
#define LE 101
#define GT 102
#define GE 103
#define EQ 104
#define INT 201
#define CHAR 202
#define FLOAT 203
struct BlockInfo;


struct FileInfo
{
	int type;					// 0-> data file
								// 1 -> index file
	string fileName;			// the name of the file
	int recordAmount;			// the number of record in the file
    int freeNum;                // the free block number which could be used for the file
	int recordLength;			// the length of the record in the file
	FileInfo *next;				// the pointer points to the next file
	BlockInfo *firstBlock;		// point to the first block within the file
};

struct BlockInfo
{
	int blockNum;				// the block number of the block, which indicate it when it be newed
	bool dirtyBit;				// 0 -> flase
								// 1 -> indicate dirty, write back
	BlockInfo *next;			// the pointer point to next block
	FileInfo *file;				// the pointer point to the file, which the block belongs to
	int charNum;				// the number of chars in the block
	char *cBlock;				// the array space for storing the records in the block in buffer
	int iTime;					// it indicate the age of the block in use
	int lock;					// prevent the block from replacing
};
// 所读取块的信息
class IndexInfo
{
public:
	string name;        //索引名
	string tableName;  //表名
	string attrName;  //属性名
	int length;                //the length of the value
	int type;                 //the type of the value
                               //0---int,1---float,2----char(n)
	long offset;               //the record offset in the table file
	int blockNum;
	string value;             //the value
};
class Condition
{
public:
	int op;
	int columNum;
	string columName;
	string value;
};
class Result
{
public:
	int blocknum;
	vector<int> offsets;
};
/*调用buffer的四个函数
BlockInfo* getEmptyBlock(string dbName,string name);
BlockInfo* getBlock(string dbName, string name,int blockNum, int type);
void deleteBlock(string dbName,string name,BlockInfo* tempBlock);
void writeRootBlock(string dbName,string name,BlockInfo* tempBlock);
*/

class BPlusTree
{
private:

	const int LENGTH_OF_INT;
	const int LENGTH_OF_FLOAT;
	int LENGTH_OF_CHAR;
	int N;//每个节点指针个数
    int leaf_least;//叶子节点最少的value值的个数
	int nonleaf_least;//非叶子节点最少的value值的个数
	const int VBIT;//每个节点value的个数用4位的string表示
	const int BBIT;//每个节点中孩子的块号用3位的string表示
	const int OBIT;//叶子节点的记录在块中的偏移量用4位的string表示
	const int LBIT;//叶子节点的块号+偏移量的位数
	int searchLeaf(string dbName, IndexInfo inform);
	void write(BlockInfo* b,string s);
	string findLinfo(string dbName,IndexInfo inform,int nodenum,int type);
	int compareInt(string int1,string int2);//将连个表示int的字符串进行大小比较
	int compareFloat(string float1,string float2);//将两个表示float的字符串进行大小比较
	int compareStr(string str1,string str2);
	int getValueNum(string snum);//将表示value个数的字符串转化成int
	int getBlockNum(string sblocknum);//将表示块号（中间节点中）的字符串转化成int
	int getOffset(string stableoffset);//将表示偏移量（叶子节点中）的字符串转化成int
	string intToStr(int value,int length);//将int转化成length长度的字符串
	string toLength(IndexInfo inform);//将字符串转化成标准长度
	void insertInLeaf(string database,IndexInfo &inform,int Node);
	void insertInParent(string dbName,IndexInfo inform,int N,string K1,int N1);
	void deleteEntry(string dbName,IndexInfo inform,int n,string K,int nod );
	int findPrevLeafSibling(string dbName, IndexInfo inform, int nodenum);//找前一个叶子的块号
	int findNextLeafSibling(string dbName, IndexInfo inform,int nodenum);//找后一个叶子的块号
	int findLeftestChild (string dbName,IndexInfo  inform);
	int findFather(string dbName,IndexInfo inform, int num);//找到节点的父节点
	void setN(int n);
	void getResult(string dbname,string name,int start,int end,IndexInfo inform,string Linfo,int type,vector<Result>& res);
	void searchOne(string dbName,IndexInfo inform,vector<Result>& res);
	void searchMany(string dbName,int type,IndexInfo inform,vector<Result>& res) ;
	BPlusTree();
public:
	/*inform需包含所有IndexInfo属性，con为一个条件（不包括不等于），res为一个空的容器*/
	void search(string dbName,IndexInfo inform,Condition con,vector<Result>& res);
	/*inform需包含IndexInfo所有属性，除了blocknum和offset，如果是char需告知length*/
	void insertOne(string dbName, IndexInfo inform);
	/*inform需包含所有IndexInfo属性，除了blocknum和offset，如果是char需告知length*/
	void deleteOne(string dbName,IndexInfo inform);
};
#endif /* INDEX_MANAGER_H_ */
