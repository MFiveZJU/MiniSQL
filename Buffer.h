/***********************************************
**Buffer Module
************************************************/

#if !defined(_BUFFER_H_)
#define _BUFFER_H_

#include <fstream>

class fileHandle{
public:
	FileInfo* firstFile;		//the first file in the list of files
	int fileNum;				//the number of files in the buffer
	int blockAmout;				//the number of blocks in the buffer

	fileHandle():firstFile(NULL),fileNum(0),blockAmount(0){};
	~fileHandle(){};
};

class Buffer{
public:
	fileHandle* fileHead;

	Buffer(){
		fileHead = new fileHandle;
	}

	~Buffer(){
		delete fileHead;
	}
	//get the block amount
	int getBlockAmount(string dbName,string name,int fileType);

	//find the data block required
	BlockInfo* getBlock(string dbName,string name,int blockNum,int type);

	//find an available block required
	//只有DATA会调用，所以不用类型
	BlockInfo* getAvaBlock(string dbName,string name); 

	//find an empty Block for index
	//只有Index会调用！所以不用类型
	BlockInfo* getEmptyBlock(string dbName,string name);

	//find the file required or available
	FileInfo* findFile(string dbName,string fileName,int fileType);

	//find an available block
	BlockInfo* findBlock(string dbName);

	//find the required block from the Files
	void readBlock(string dbName,string name,int fileType,int blockNum,BlockInfo* tempBlock);

	//find an empty block form the Files
	void readEmptyBlock(string dbName,string name,int fileType,BlockInfo* tempBlock);
	
	//write back the block to the Files
	//可能可以把这个和writeBack合并！
	void writeBlock(string dbName,BlockInfo* tempBlock);

	//add the block to the file
	void addBlock(string DB_Name,string name,FileInfo* tempFile,BlockInfo* tempBlock);

	//delete a block for index
	//只有index会调用，所以不用类型
	void deleteBlock(string dbName,string name,BlockInfo* tempBlock);

	//close the file
	void closeFile(string dbName,string fileName,int fileType);

	//close the database
	void closeDatabase(string dbName);

	//quit the process
	void quitProc(string dbName);

};

#endif