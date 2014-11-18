/***********************************************
**Buffer Module
************************************************/

#if !defined(_BUFFER_H_)
#define _BUFFER_H_

#include <fstream>


//最好能够在BlockInfo里面加上recordAmount!

class fileHandle{
public:
	fileInfo* firstFile;		//the first file in the list of files
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
	//find the data block required
	blockInfo* getBlock(string dbName,string name,int blockNum,int type);

	//find an available block required
	blockInfo* getAvaBlock(string dbName,string name); 

	//find the file required or available
	fileInfo* findFile(string dbName,string fileName,int fileType);

	//find an available block
	blockInfo* findBlock(string dbName);

	//find the required block from the Files
	void readBlock(string dbName,string name,int fileType,int blockNum,blockInfo* tempBlock);

	//write back the block to the Files
	//可能可以把这个和writeBack合并！
	void writeBlock(string dbName,blockInfo* tempBlock);

	//add the block to the file
	void addBlock(string DB_Name,string name,fileInfo* tempFile,blockInfo* tempBlock);

	//find an empty Block for index
	//只有Index会调用！所以不用类型
	blockInfo* getEmptyBlock(string dbName,string name);

	//write the root back
	//写回之前先判断是删除根还是写回新根，通过charNum
	void writeRootBlock(string dbName,string name,blockInfo* tempBlock); 

	//delete a block for index
	//只有index会调用，所以不用类型
	void deleteBlock(string dbName,string name,blockInfo* tempBlock);

	//initial an indexFile
	void getIndexInfo(string dbName,string Index_Name,fileInfo* tempFile);

	//initial an dataFile
	void getTableInfo(string dbName,string name,fileInfo* tempFile);

	//close the file
	void closeFile(string dbName,string fileName,int fileType);

	//close the database
	void closeDatabase(string dbName);

	//quit the process
	void quitProc(string dbName);

};

#endif