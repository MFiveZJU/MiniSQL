
blockInfo* Buffer::getBlock(string dbName,string name,int fileType,int blockNum){
	string fileName = dbName+'_'+name;

	fileInfo* tempFile = findFile(dbName,fileName,fileType);

	blockInfo* tempBlock = tempFile->firstBlock;
	blockInfo* oldBlock = NULL;
	while(tempBlock){								//search the block
		if(tempBlock->blockNum == blockNum){
			//可变成setEnd函数，最好使用模板
													//move the block to the tail
			if(tempBlock->next){					//not tail
				if(oldBlock)						//not head
					oldBlock->next = tempBlock->next;
				else								//head
					tempFile->firstBlock = tempBlock->next;
				blockInfo* tempBlock2 = tempBlock;
				while(tempBlock2->next)				
					tempBlock2 = tempBlock2->next;	
				tempBlock2->next = tempBlock;
				tempBlock->next = NULL;				
			}

			tempBlock->iTime++;						//tail
			tempBlock->lock = 1;					//暂时不管锁
			goto end;
		}
		else{										//search the next block
			oldBlock = tempBlock;
			tempBlock = tempBlock->next;
		}
	}
															//do not find the block
	blockInfo* tempBlock2 = findBlock(dbName);				//find an available block from buffer
															//load a block from file into buffer
	readBlock(dbName,name,fileType,blockNum,tempBlock2);
	addBlock(dbName,name,tempFile,tempBlock2);

	tempBlock2->iTime++;
	tempBlock2->lock = 1;
	tempBlock = tempBlock2;

	end: return tempBlock;
}

//需要修改File中的文件头属性，因为后面没有相应的修改操作
fileInfo* Buffer::findFile(string dbName,string fileName,int fileType){
	fileInfo* tempFile = fileHead->firstFile;
	fileInfo* oldFile = NULL;
	while(tempFile){										//search the file
		if((tempFile->fileName == fileName) && (tempFile->type == fileType)){
															//move the file to the tail
			if(tempFile->next){								//not tail
				if(oldFile)									//not head
					oldFile->next = tempFile->next;
				else										//head
					fileHead->firstFile = tempFile->next;
				fileInfo* tempFile2 = tempFile;
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

	if(fileHead->fileNum >= MAX_FILE_ACTIVE){
		closeFile(dbName,fileHead->firstFile->fileName,fileHead->firstFile->type);
		tempFile = fileHead->firstFile;
		fileHead->firstFile = tempFile->next;
		free(tempFile);
		fileHead->fileNum--;
	}
	fileInfo* tempFile2 = (fileInfo* )malloc(sizeof(fileInfo));
	if(fileType)										//1 for index
		getIndexInfo(dbName,fileName,tempFile2);
	else												//0 for data
		getTableInfo(dbName,fileName,tempFile2);
	fileHead->fileNum++;
	if(oldFile)
		oldFile->next = tempFile2;
	else
		fileHead->firstFile = tempFile2;
	tempFile2->next = NULL;
	tempFile = tempFile2;

	end: return tempFile;
}

blockInfo* Buffer::findBlock(string dbName){
	if(fileHead->blockAmount < MAX_BLOCK ){
		blockInfo* tempBlock = new blockInfo;	
		fileHead->blockAmount++;
		goto end;
	}

	else{												//LRU algorithm
		fileInfo* tempFile = fileHead->firstFile;
		if(tempFile)
			blockInfo* tempBlock = tempFile->firstBlock;
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

void Buffer::readBlock(string dbName,string name,int fileType,int blockNum,blockInfo* tempBlock){
//从磁盘中读取该块，返回该块
	string fileName = dbName+'_'+name;

	int blockAmount;

	if(fileType){										//index file
		fileName +=".index";
		cout << "read from an index file" << endl;
	}
	else{												//data file
		fileName +=".db";
		ifstream infile(fileName,ios::binary);
		if(!infile.is_open()){							//需要改成抛出错误
			cout << "when read:file open failed" << endl;
		}
		infile.seekg(0,ios::beg);
		infile.read((char*)(&blockAmount),sizeof(blockAmount));
		if(blockAmount<blockNum){						//expand the space of the file
			infile.close();
			ofstream outfile(fileName,ios::binary);
			if(!outfile.is_open()){						//需要改成抛出错误
				cout << "when read&write:file open failed" << endl;
			}
			oufile.seekp(0,ios::end);
			//这里有个假设：block在文件中是连续的
			outfile.write((char*)tempBlock,sizeof(blcokInfo));
			outfile.close();
		}
		else{
			infile.seekg(sizeof(blockInfo)*(blockNum-1),ios::cur);
			infile.read((char*)tempBlock,sizeof(blockInfo));
			infile.close();
		}
	}
}

void Buffer::writeBlock(string dbName,blockInfo* tempBlock){
	//无需初始化，findBlock会进行初始化的操作。
	string fileName = tempBlock->file->fileName;
	int fileType = tempBlock->file->type;
	tempBlock->dirtyBit = 0;
	tempBlock->next = NULL;
	tempBlock->iTime = 0;
	tempBlock->lock = 0;	

	int blockAmount,recordLength,recordAmount;

	if(fileType){										//index file
		fileName +=".index";
		cout << "write to an index file" << endl;
	}
	else{												//data file
		fileName +=".db";
		ifstream infile(fileName,ios::binary);
		if(infile.is_open()){
			infile.seekg(0,ios::beg);
			infile.read((char*)(&blockAmount),sizeof(blockAmount));
			infile.read((char*)(&recordLength),sizeof(recordLength));
			infile.read((char*)(&recordAmount),sizeof(recordAmount));
			infile.seekg(sizeof(blockInfo)*(tempBlock->blockNum-1),ios::cur);
			
			blockInfo* oldBlock = (blockInfo*)malloc(sizeof(blockInfo));
			infile.read((char*)oldBlock,sizeof(blockInfo));

			infile.close();

			int oldAmount = oldBlock->charNum/recordLength;
			int newAmount = tempBlock->charNum/recordLength;

			recordAmount = recordAmount - oldAmount + newAmount;
			free(oldBlock);

			ofstream outfile(fileName,ios::binary);
			if(outfile.is_open()){
				outfile.seekp(0,ios::beg);
				outfile.write((char*)(&blockAmount),sizeof(blockAmount));
				outfile.write((char*)(&recordLength),sizeof(recordLength));
				outfile.write((char*)(&recordAmount),sizeof(recordAmount));
				outfile.seekp(sizeof(blockInfo)*(tempBlock->blockNum-1),ios::cur)；
				outfile.write((char*)tempBlock,sizeof(blockInfo));

				outfile.close();
			}
			else{
				cout<< "when write:file open failed" << endl;
			}
		}
		else{
			cout << "when write read:file open failed" << endl;
		}

	}
}

void Buffer::addBlock(string dbName,string name,fileInfo* tempFile,blockInfo* tempBlock){
	tempBlock->next = NULL;
	tempBlock->file = tempFile;

	int count;
	attr_info print[32];
	Get_Attr_Info_All(dbName,name,print[],count);	//get the info of the table

	int recordLength = print[count-1].offset+print[count-1].length;
	int recordNum = tempBlock->charNum/recordLength;
	
	tempFile->recordAmount +=recordNum;

	blockInfo* tempBlock2 = tempFile->firstBlock;
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

void Buffer::getIndexInfo(string dbName,string name,fileInfo* tempFile){
	tempFile->type = 1;
	tempFile->fileName = name;
	tempFile->recordAmount = 0;
	tempFile->freeNum = 0;
	tempFile->next = NULL;
	tempFile->firstBlock = NULL;
}

//关于Index的结构还需要好好地问一下
void Buffer::getTableInfo(string dbName,string name,fileInfo* tempFile){
	tempFile->type = 0;
	tempFile->fileName = name;
	tempFile->recordAmount = 0;
	tempFile->freeNum = 0;
	tempFile->next = NULL;
	tempFile->firstBlock = NULL;
}

void Buffer::closeFile(string dbName,string fileName,int fileType){
	fileInfo* tempFile = fileHead->firstFile;

	while(tempFile){
		if((tempFile->fileName == fileName) && (tempFile->type == fileType)){
			blockInfo* tempBlock = tempFile->firstBlock;
			blockInfo* oldBlock = NULL;
			while(tempBlock){
				if(tempBlock->dirtyBit)
					writeBlock(dbName,tempBlock)
				oldBlock = tempBlock;
				tempBlock = tempBlock->next;
				free(oldBlock);
			}
			break;
		}
		else
			tempFile = tempFile->next;
	}
}

void Buffer::closeDatabase(string dbName){
	fileInfo* tempFile = fileHead->firstFile;
	fileInfo* oldFile = NULL;

	while(tempFile){
		closeFile(dbName,tempFile->fileName,tempFile->type);
		oldFile = tempFile;
		tempFile = tempFile->next;
		free(oldFile);
	}
}

bool Buffer::quitpProc(string dbName){
	fileInfo* tempFile = fileHead->firstFile;
	fileInfo* oldFile = NULL;

	while(tempFile){
		closeFile(dbName,tempFile->fileName,tempFile->type);
		oldFile = tempFile;
		tempFile = tempFile->next;
		free(oldFile);
	}

	free(fileHead);

	return true;
}


//得到catalog属性使用 void getTableInfo(tableInfo& table);

