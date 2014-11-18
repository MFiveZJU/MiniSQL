#include "Buffer.h"
#include "Catalog.h"

int Buffer::getBlockAmount(string dbName,string name,int fileType){
	int blockAmount;

	if(fileType)
		fileName = dbName+"_"+name+".index";
	else
		fileName = dbName+"_"+name+".db";
	ifstream infile(fileName,ios::binary);
	if(!infile.is_open())							//这里应该抛出异常
		cout << "when read:file open failed" << endl;
	
	infile.seekg(0,ios::beg);
	infile.read((char*)(&blockAmount),sizeof(blockAmount));
	infile.close();

	return blockAmount;
}

BlockInfo* Buffer::getBlock(string dbName,string name,int blockNum,int type){
	string fileName = dbName+'_'+name;

	FileInfo* tempFile = findFile(dbName,fileName,type);

	BlockInfo* tempBlock = tempFile->firstBlock;
	BlockInfo* oldBlock = NULL;
	while(tempBlock){								//search the block
		if(tempBlock->blockNum == blockNum){		//move the block to the tail
			if(tempBlock->next){					//not tail
				if(oldBlock)						//not head
					oldBlock->next = tempBlock->next;
				else								//head
					tempFile->firstBlock = tempBlock->next;
				BlockInfo* tempBlock2 = tempBlock;
				while(tempBlock2->next)				
					tempBlock2 = tempBlock2->next;	
				tempBlock2->next = tempBlock;
				tempBlock->next = NULL;				
			}

			tempBlock->iTime++;						//tail
			tempBlock->lock = 1;
			goto end;
		}
		else{										//search the next block
			oldBlock = tempBlock;
			tempBlock = tempBlock->next;
		}
	}
															//do not find the block
	BlockInfo* tempBlock2 = findBlock(dbName);				//find an available block from buffer
															//load a block from file into buffer
	readBlock(dbName,name,fileType,blockNum,tempBlock2);
	if(tempBlock2 != NULL)
		addBlock(dbName,name,tempFile,tempBlock2);
		tempBlock2->iTime++;
		tempBlock2->lock = 1;
	}

	tempBlock = tempBlock2;
	end: return tempBlock;
}

BlockInfo* getAvaBlock(string dbName,string name){
	string fileName = dbName+"_"+name;
	FileInfo* tempFile = findFile(dbName,fileName,0);

	BlockInfo* tempBlock = tempFile->firstBlock;
	BlockInfo* oldBlock = NULL;
	while(tempBlock){								//search the block
		if(!tempBlock->isFull){						//move the block to the tail
			if(tempBlock->next){					//not tail
				if(oldBlock)						//not head
					oldBlock->next = tempBlock->next;
				else								//head
					tempFile->firstBlock = tempBlock->next;
				BlockInfo* tempBlock2 = tempBlock;
				while(tempBlock2->next)				
					tempBlock2 = tempBlock2->next;	
				tempBlock2->next = tempBlock;
				tempBlock->next = NULL;				
			}

			tempBlock->iTime++;						//tail
			tempBlock->lock = 1;
			goto end;
		}
		else{										//search the next block
			oldBlock = tempBlock;
			tempBlock = tempBlock->next;
		}
	}
															//do not find the block
	BlockInfo* tempBlock2 = findBlock(dbName);				//find an available block from buffer
															//load a block from file into buffer
	readEmptyBlock(dbName,name,0,tempBlock2);
	addBlock(dbName,name,tempFile,tempBlock2);
	tempBlock2->iTime++;
	tempBlock2->lock = 1;

	tempBlock = tempBlock2;
	end: return tempBlock;
}

BlockInfo* Buffer::getEmptyBlock(string dbName,string name){
	string fileName = dbName+"_"+name;
	FileInfo* tempFile = findFile(dbName,fileName,1);

	BlockInfo* tempBlock = tempFile->firstBlock;
	BlockInfo* oldBlock = NULL;
	while(tempBlock){								//search the block
		if(tempBlock->charNum == 0){				//move the block to the tail
			if(tempBlock->next){					//not tail
				if(oldBlock)						//not head
					oldBlock->next = tempBlock->next;
				else								//head
					tempFile->firstBlock = tempBlock->next;
				BlockInfo* tempBlock2 = tempBlock;
				while(tempBlock2->next)				
					tempBlock2 = tempBlock2->next;	
				tempBlock2->next = tempBlock;
				tempBlock->next = NULL;				
			}

			tempBlock->iTime++;						//tail
			tempBlock->lock = 1;
			goto end;
		}
		else{										//search the next block
			oldBlock = tempBlock;
			tempBlock = tempBlock->next;
		}
	}

	BlockInfo* tempBlock2 = findBlock(dbName);				//find an available block from buffer
															//load a block from file into buffer
	readEmptyBlock(dbName,name,1,tempBlock2);
	addBlock(dbName,name,tempFile,tempBlock2);
	tempBlock2->iTime++;
	tempBlock2->lock = 1;

	tempBlock = tempBlock2;
	end: return tempBlock;
}

FileInfo* Buffer::findFile(string dbName,string fileName,int fileType){
	FileInfo* tempFile = fileHead->firstFile;
	FileInfo* oldFile = NULL;
	while(tempFile){										//search the file
		if((tempFile->fileName == fileName) && (tempFile->type == fileType)){
															//move the file to the tail
			if(tempFile->next){								//not tail
				if(oldFile)									//not head
					oldFile->next = tempFile->next;
				else										//head
					fileHead->firstFile = tempFile->next;
				FileInfo* tempFile2 = tempFile;
				while(tempFile2->next)
					tempFile2 = tempFile2->next;
				tempFile2->next = tempFile;
				tempFile->next = NULL;
			}
			goto end;
		}
		else{
			oldFile = tempFile;
			tempFile = tempFile->next;
		}
	}

	if(fileHead->fileNum >= MAX_FILE_ACTIVE)
		closeFile(dbName,fileHead->firstFile->fileName,fileHead->firstFile->type);
	FileInfo* tempFile2 = new FileInfo(fileType,fileName);

	fileHead->fileNum++;
	if(oldFile)
		oldFile->next = tempFile2;
	else
		fileHead->firstFile = tempFile2;
	tempFile2->next = NULL;
	tempFile = tempFile2;

	end: return tempFile;
}

BlockInfo* Buffer::findBlock(string dbName){
	if(fileHead->blockAmount < MAX_BLOCK ){
		BlockInfo* tempBlock = new BlockInfo();	
		fileHead->blockAmount++;
		goto end;
	}

	else{												//LRU algorithm
		FileInfo* tempFile = fileHead->firstFile;
		if(tempFile)
			BlockInfo* tempBlock = tempFile->firstBlock;
		minDirtyBlock = NULL;
		minBlock = NULL;
		
		while(tempFile){
			if(tempBlock){
				if(tempBlock->dirtyBit && !minDirtyBlock)
					minDirtyBlock = tempBlock;
				else if(tempBlock->dirtyBit && (minDirtyBlock->iTime > tempBlock->iTime))
					minDirtyBlock = tempBlock;
				else if(!tempBlock->dirtyBit && !minBlock)
					minBlock = tempBlock;
				else if(!tempBlock->dirtyBit && (minBlock->iTime > tempBlock->iTime))
					minBlock = tempBlock;
			}

			tempFile = tempFile->next;
			tempBlock = tempFile->firstBlock;
		}

		if(minBlock){
			minBlock->file->firstBlock = minBlock->next;
			tempBlock = minBlock;
		}
		else{
			minDirtyBlock->file->firstBlock = minDirtyBlock->next;
			tempBlock = minDirtyBlock;
			writeBlock(dbName,tempBlock);
		}
		tempBlock.clearBlock();
		end: return tempBlock;
	}
}

void Buffer::readBlock(string dbName,string name,int fileType,int blockNum,BlockInfo* tempBlock){
//从磁盘中读取该块，返回该块
//考虑重用代码
	string fileName = dbName+'_'+name;
	int blockAmount,emptyAmount;
	int emptyBlock[100];

	if(fileType)										//index file
		fileName +=".index";
	else
		fileName +=".db";

	ifstream infile(fileName,ios::binary);
	if(!infile.is_open())							//需要改成抛出错误
		cout << "when read:file open failed" << endl;

	infile.seekg(0,ios::beg);
	infile.read((char*)(&blockAmount),sizeof(blockAmount));
	if(!fileType)
		infile.seekg(2*sizeof(int),ios::cur);
	infile.read((char*)(&emptyAmount),sizeof(emptyAmount));

	if((blockAmount+emptyAmount)<blockNum+1){
		delete tempBlock;
		tempBlock = NULL;
	}

	else{
		infile.read((char*)(emptyBlock),100*sizeof(int));
		int i=0;
		for(i=0;i<emptyAmount;i++){					//search the empty block
			if(blockNum == emptyBlock[i]){
				delete tempBlock;
				tempBlock = NULL;
				break;
			}
		}
		if(i == emptyAmount){						//not empty
			infile.seekg(sizeof(BlockInfo)*(blockNum),ios::cur);
			infile.read((char*)tempBlock,sizeof(BlockInfo));
		}
	}

	infile.close();
}

void Buffer::readEmptyBlock(string dbName,string name,int fileType,BlockInfo* tempBlock){
	string fileName = dbName+'_'+name;
	int blockAmount,emptyAmount;
	int emptyBlock[100];

	if(fileType)										//index file
		fileName +=".index";
	else
		fileName +=".db";

	ifstream infile(fileName,ios::binary);
	if(!infile.is_open())							//需要改成抛出错误
		cout << "when read:file open failed" << endl;

	infile.seekg(0,ios::beg);
	infile.read((char*)(&blockAmount),sizeof(blockAmount));
	if(!fileType)
		infile.seekg(2*sizeof(int),ios::cur);
	infile.read((char*)(&emptyAmount),sizeof(emptyAmount));
	if(emptyAmount == 0){							//expand the space of the file
		blockAmount++;
		tempBlock->blockNum = blockNum-1;
		// tempBlock->dirtyBit = 1;					//record应该会自己改
	}
	else{
		infile.read((char*)(emptyBlock),100*sizeof(int));
		tempBlock->blockNum = emptyBlock[0];
		// tempBlock->dirtyBit = 1;
		emptyBlock[0] = emptyBlock[emptyAmount-1];
		emptyBlock[emptyAmount-1] = -1;
		emptyAmount--;
		blockAmount++;
	}
	infile.close();

	ofstream outfile(fileName,ios::binary);
	if(!outfile.is_open())							//需要改成抛出错误
		cout << "when write read:file open failed" << endl;

	outfile.seekp(0,ios::beg);
	outfile.write((char*)(&blockAmount),sizeof(blockAmount));
	outfile.write((char*)(&emptyAmount),sizeof(emptyAmount));
	if(!fileType){
		outfile.seekp(2*sizeof(int),ios::cur);
	}
	outfile.write((char*)(emptyBlock),100*sizeof(int));
	outfile.seekp(sizeof(BlockInfo)*(tempBlock->blockNum),ios::cur);
	outfile.write((char*)tempBlock,sizeof(BlockInfo));

	outfile.close();
}

void Buffer::writeBlock(string dbName,BlockInfo* tempBlock){
//无需初始化，findBlock会进行初始化的操作。
//考虑重用代码
	string fileName = tempBlock->file->fileName;
	int fileType = tempBlock->file->type;
	tempBlock->dirtyBit = 0;
	tempBlock->next = NULL;
	tempBlock->iTime = 0;
	tempBlock->lock = 0;	

	int blockAmount,recordLength,recordAmount,emptyAmount;
	int emptyBlock[100];
	
	if(fileType)
		fileName +=".index";
	else
		fileName +=".db";

	ifstream infile(fileName,ios::binary);
	if(!infile.is_open())							//需要改成抛出错误
		cout<< "when write:file open failed" << endl;

	infile.seekg(0,ios::beg);
	infile.read((char*)(&blockAmount),sizeof(blockAmount));
	if(!fileType){
		infile.read((char*)(&recordLength),sizeof(recordLength));
		infile.read((char*)(&recordAmount),sizeof(recordAmount));
	}
	infile.read((char*)(&emptyAmount),sizeof(emptyAmount));
	infile.read((char*)(emptyBlock),100*sizeof(int));

	if(!fileType){									//update the recordAmount
		infile.seekg(sizeof(BlockInfo)*(tempBlock->blockNum),ios::cur);
		BlockInfo* oldBlock = new BlockInfo();
		infile.read((char*)oldBlock,sizeof(BlockInfo));

		recordAmount = recordAmount - oldBlock->charNum/recordLength + tempBlock->charNum/recordLength;
		delete oldBlock;
	}
	infile.close();

	if(tempBlock->charNum == 0){					//remember the empty block
		emptyBlock[blockAmount] = tempBlock->blockNum;
		blockAmount--;
		emptyAmount++;
	}

	ofstream outfile(fileName,ios::binary);
	if(!outfile.is_open())							//需要改成抛出错误
		cout << "when write read:file open failed" << endl;

	outfile.seekp(0,ios::beg);
	outfile.write((char*)(&blockAmount),sizeof(blockAmount));
	outfile.write((char*)(&emptyAmount),sizeof(emptyAmount));
	if(!fileType){
		outfile.write((char*)(&recordLength),sizeof(recordLength));
		outfile.write((char*)(&recordAmount),sizeof(recordAmount));
	}
	outfile.write((char*)(emptyBlock),100*sizeof(int));
	outfile.seekp(sizeof(BlockInfo)*(tempBlock->blockNum),ios::cur);
	outfile.write((char*)tempBlock,sizeof(BlockInfo));

	outfile.close();
}
	
void Buffer::addBlock(string dbName,string name,FileInfo* tempFile,BlockInfo* tempBlock){

	int count;
	TableInfo table;
	table.name = dbName + "_" + name; 
	getTableInfo(table);							//get the info of the table

	int recordLength = table.totalLength;
	int recordNum = tempBlock->charNum/recordLength;
	
	tempFile->recordAmount +=recordNum;

	BlockInfo* tempBlock2 = tempFile->firstBlock;
	if(tempBlock2){
		while(tempBlock2->next)
			tempBlock2 = tempBlock2->next;
		tempBlock2->next = tempBlock;
	}
	else
		tempFile->firstBlock = tempBlock;
	tempBlock->next = NULL;
	tempBlock->file = tempFile;
}

void Buffer::deleteBlock(string dbName,string name,BlockInfo* tempBlock){
	tempBlock->dirtyBit = 1;
	tempBlock->charNum = 0;
	delete[] tempBlock->cBlock;
	tempBlock->cBlock = new char[BLOCKSIZE];
	father = -1;
	iTime = 0;
	lock = 0;
	isFull = 0;
}

void Buffer::closeFile(string dbName,string fileName,int fileType){
	FileInfo* tempFile = fileHead->firstFile;
	FileInfo* oldFile = NULL;

	while(tempFile){
		if((tempFile->fileName == fileName) && (tempFile->type == fileType)){
			BlockInfo* tempBlock = tempFile->firstBlock;
			BlockInfo* oldBlock = NULL;
			while(tempBlock){
				if(tempBlock->dirtyBit)
					writeBlock(dbName,tempBlock)
				oldBlock = tempBlock;
				tempBlock = tempBlock->next;
				delete oldBlock;
			}
			if(oldFile)
				oldFile->next = tempFile->next;
			else
				fileHead->firstFile = tempFile->next;
			delete tempFile;
			fileHead->fileNum--;
			break;
		}
		else{
			oldFile = tempFile;
			tempFile = tempFile->next;
		}
	}
}

void Buffer::closeDatabase(string dbName){

	FileInfo* tempFile = fileHead->firstFile;
	while(tempFile){
		closeFile(dbName,tempFile->fileName,tempFile->type);
		tempFile = fileHead->firstFile;
	}
}

bool Buffer::quitpProc(string dbName){
	closeDatabase(dbName);

	delete fileHead;

	return true;
}
