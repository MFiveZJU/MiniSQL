/*
 * IndexManager.h
 *
 *  Created on: 2014��10��31��
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
// ����ȡ�����Ϣ
class IndexInfo
{
public:
	string name;        //������
	string tableName;  //����
	string attrName;  //������
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
/*����buffer���ĸ�����
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
	int N;//ÿ���ڵ�ָ�����
    int leaf_least;//Ҷ�ӽڵ����ٵ�valueֵ�ĸ���
	int nonleaf_least;//��Ҷ�ӽڵ����ٵ�valueֵ�ĸ���
	const int VBIT;//ÿ���ڵ�value�ĸ�����4λ��string��ʾ
	const int BBIT;//ÿ���ڵ��к��ӵĿ����3λ��string��ʾ
	const int OBIT;//Ҷ�ӽڵ�ļ�¼�ڿ��е�ƫ������4λ��string��ʾ
	const int LBIT;//Ҷ�ӽڵ�Ŀ��+ƫ������λ��
	int searchLeaf(string dbName, IndexInfo inform);
	void write(BlockInfo* b,string s);
	string findLinfo(string dbName,IndexInfo inform,int nodenum,int type);
	int compareInt(string int1,string int2);//��������ʾint���ַ������д�С�Ƚ�
	int compareFloat(string float1,string float2);//��������ʾfloat���ַ������д�С�Ƚ�
	int compareStr(string str1,string str2);
	int getValueNum(string snum);//����ʾvalue�������ַ���ת����int
	int getBlockNum(string sblocknum);//����ʾ��ţ��м�ڵ��У����ַ���ת����int
	int getOffset(string stableoffset);//����ʾƫ������Ҷ�ӽڵ��У����ַ���ת����int
	string intToStr(int value,int length);//��intת����length���ȵ��ַ���
	string toLength(IndexInfo inform);//���ַ���ת���ɱ�׼����
	void insertInLeaf(string database,IndexInfo &inform,int Node);
	void insertInParent(string dbName,IndexInfo inform,int N,string K1,int N1);
	void deleteEntry(string dbName,IndexInfo inform,int n,string K,int nod );
	int findPrevLeafSibling(string dbName, IndexInfo inform, int nodenum);//��ǰһ��Ҷ�ӵĿ��
	int findNextLeafSibling(string dbName, IndexInfo inform,int nodenum);//�Һ�һ��Ҷ�ӵĿ��
	int findLeftestChild (string dbName,IndexInfo  inform);
	int findFather(string dbName,IndexInfo inform, int num);//�ҵ��ڵ�ĸ��ڵ�
	void setN(int n);
	void getResult(string dbname,string name,int start,int end,IndexInfo inform,string Linfo,int type,vector<Result>& res);
	void searchOne(string dbName,IndexInfo inform,vector<Result>& res);
	void searchMany(string dbName,int type,IndexInfo inform,vector<Result>& res) ;
	BPlusTree();
public:
	/*inform���������IndexInfo���ԣ�conΪһ�������������������ڣ���resΪһ���յ�����*/
	void search(string dbName,IndexInfo inform,Condition con,vector<Result>& res);
	/*inform�����IndexInfo�������ԣ�����blocknum��offset�������char���֪length*/
	void insertOne(string dbName, IndexInfo inform);
	/*inform���������IndexInfo���ԣ�����blocknum��offset�������char���֪length*/
	void deleteOne(string dbName,IndexInfo inform);
};
#endif /* INDEX_MANAGER_H_ */
