extern Buffer bufferManager;

int blockMore(int totalLength, int recordNum, int occupied)
{
	if((BLOCKSIZE - totalLength*recordNum - occupied) >= 0)
		return 0;
	else if(0 == (totalLength*recordNum + occupied)%BLOCKSIZE)
		return (totalLength*recordNum + occupied)/BLOCKSIZE;
	else
		return (totalLength*recordNum + occupied)/BLOCKSIZE + 1;
}

bool isFull(int totalLength, int occupied)
{
	if(BLOCKSIZE - occupied < totalLength)
		return true;
	else
		return false;
}

bool emptyRecord(char* block, int length)
{
	char empRec[length];
	memset(empRec, '0', length);
	if(0 == memcmp(block, empRec, length))
		return true;
	else
		return false;
}

int compareStr(string value,string str2)
{
	while(str2[0]=='0')
		str2=str2.substr(1,str2.size()-1);
	if(value<str2)
		return-1; 
	else if(value==str2)
		return 0;
	else
		return 1;
}

int compareInt(string int1,string int2)
{
	while(int2[0]=='0')
		int2=int2.substr(1,int2.size()-1);
	int n1=atoi(int1.c_str());
	int n2=atoi(int2.c_str());
	if(n1<n2)
		return -1;
	else if(n1==n2)
		return 0;
	else
		return 1;
}

int compareFloat(string float1,string float2)
{
	while(float2[0]=='0')
		float2=float2.substr(1,float2.size()-1);
	float a=atof(float1.c_str());
	float b=atof(float2.c_str());
	if(a<b)
		return -1;
	else if(a==b)
		return 0;
	else
		return 1;
}

bool compare(string s1, string s2, int type, int condType)
{
	int compare;
	switch(type)
	{
		case INT:
			compare = compareInt(s1, s2);
			break;
		case CHAR:
			compare = compareStr(s1, s2);
			break;
		case FLOAT:
			compare = compareFloat(s1, s2);
			break;
		default:
			compare = -100;
			break;
	}
	switch(condType)
	{
		case LT:
			return (compare == -1);
		case LE:
			return (compare != 1);
		case GT:
			return (compare == 1);
		case GE:
			return (compare != -1);
		case EQ:
			return (compare == 0);
		case NE:
			return (compare != 0);
	}
}

void getData(string DB_Name, TableInfo table, Data& data)
{
	int blockAmount = bufferManager.getBlockAmount();
	for (int i = 0; i < blockAmount; ++i)
	{
		BlockInfo* block = bufferManager.getBlock(DB_Name, table.name, i, DATAFILE);
		int recordAmount = block->charNum/table.totalLength;
		char* tmpBlock = block->cBlock;
		for (int j = 0; j < recordAmount; ++j)
		{
			if(!emptyRecord(tmpBlock, table.totalLength))
			{
				Record tmpRecord;
				tmpRecord.blockNum = i;
				tmpRecord.offset = j;
				for (int k = 0; k < table.attrNum; ++k)
				{
					int rLength = table.attributes[k].length;
					while(rLength > 0 && tmpBlock[table.attributes[k].length-rLength] == '0')
						rLength--;
					char tmpValue[rLength+1];
					memmove(tmpValue, tmpBlock+table.attributes[k].length-rLength, rLength);
					tmpValue[rLength] = '\0';
					string tmpColumn = tmpValue;
					tmpRecord.columns.push_back(tmpColumn);
					tmpBlock += table.attributes[k].length;
				}
				data.records.push_back(tmpRecord);
			}
			else
				tmpBlock += table.totalLength;
		}
	}	
}

void getIndexData(string DB_Name, TableInfo table,vector<Result> results, Data& data)
{
	for (int i = 0; i < results.size(); ++i)
	{
		BlockInfo* block = bufferManager.getBlock(DB_Name, table.name, results[i].blockNum, DATAFILE);
		for (int j = 0; j < results[i].offsets.size(); ++j)
		{
			char* tmpBlock = block->cBlock + results[i].offsets[j];
			Record tmpRecord;
			tmpRecord.blockNum = i;
			tmpRecord.offset = j;
			for (int k = 0; k < table.attrNum; ++k)
			{
				int rLength = table.attributes[k].length;
				while(rLength > 0 && tmpBlock[table.attributes[k].length-rLength] == '0')
					rLength--;
				char tmpValue[rLength+1];
				memmove(tmpValue, tmpBlock+table.attributes[k].length-rLength, rLength);
				tmpValue[rLength] = '\0';
				string tmpColumn = tmpValue;
				tmpRecord.columns.push_back(tmpColumn);
				tmpBlock += table.attributes[k].length;
			}
			data.records.push_back(tmpRecord);
		}
	}
}

//包含生成index模块所需info容器
void insertRecord(string DB_Name, TableInfo table, Data insertedValues, int& record_Num, vector<IndexInfo>& indexValues)
{
	record_Num = insertedValues.records.size();
	char* stringValues;
	for (int i = 0; i < insertedValues.records.size(); ++i)
	{
		for(int j =0; j < insertedValues.records[i].columns.size(); ++j)
		if(insertedValues.records[i].columns[j].length() < table.attributes[j].length)
			for(int k = 0; k < table.attributes[j].length - insertedValues.records[i].columns[j].length(); ++k)
				insertedValues.records[i].columns[j] = FILLEMPTY + insertedValues.records[i].columns[j];
		stringValues = strcat(stringValues, insertedValues.records[i].columns[j].c_str());//stringValues += insertedValues[i];			
	}
	BlockInfo* newBlock = bufferManager.getAvaBlock(DB_Name, table.name);
	int blockAmount = blockMore(table.totalLength, record_Num, newBlock->charNum);
	if(blockAmount == 0)
	{
		int tableLength = table.totalLength*record_Num;
		memmove(newBlock->cBlock+newBlock->charNum, stringValues, table.totalLength*record_Num) + newBlock->charNum;
		int oriOffset = newBlock->charNum / table.totalLength;	//indexInfo
		newBlock->charNum += tableLength;
		newBlock->isFull = isFull(table.totalLength, charNum);
		//indexInfo容器
		for (int i = 0; i < record_Num; ++i)
		{
			for (int j = 0; j < table.attrNum; ++j)
			{
				if()
			}
		}
	}
	else
	{
		int tableLength = BLOCKSIZE- BLOCKSIZE%totalLength;
		int initSize = tableLength - newBlock->charNum;
		for (i = 0; i < blockAmount; ++i)
		{
			if (i == 0)
				memmove(newBlock->cBlock+newBlock->charNum, stringValues, initSize) + newBlock->charNum;
			else
				memmove(newBlock->cBlock, stringValues+initSize+(i-1)*tableLength, tableLength);
			newBlock->charNum = tableLength;
			newBlock->isFull = true;
			newBlock = bufferManager.getAvaBlock(DB_Name, table.name);
		}
		newBlock->charNum = table.totalLength*record_Num - initSize - tableLength*(i-1);
		memmove(newBlock->cBlock, stringValues+initSize+(i-1)*tableLength, newBlock->charNum);
		newBlock->isFull = isFull(table.totalLength, charNum);
	}
}

//初始化.SQL语句:CREATE INDEX Index_Name ON Table_Name (Attr_Name)
void initialIndex(string DB_Name, TableInfo table, string Attr_Name, vector<IndexInfo>& indexValues)
{
	int offInRecord = 0;
	for (int attrNo = 0; attrNo < table.attrNum; ++attrNo)
		if(table.attributes[attrNo].name == Attr_Name)
			break;
		else
			offInRecord += table.attributes[attrNo].length;	
	int blockAmount = bufferManager.getBlockAmount();
	for (int i = 0; i < blockAmount; ++i)
	{
		BlockInfo* block = bufferManager.getBlock(DB_Name, table.name, i, DATAFILE);
		int recordAmount = block->charNum / table.totalLength;
		char* tmpBlock = block->cBlock;
		for (int j = 0; j < recordAmount; ++j)
		{
			if(!emptyRecord(tmpBlock, table.attributes[attrNo].length))
			{
				IndexInfo tmpIndex;
				tmpIndex.name = table.attributes[attrNo].indexName;
				tmpIndex.tableName = table.name;
				tmpIndex.attrName = Attr_Name;
				tmpIndex.blockNum = i;
				tmpIndex.offset = j;
				tmpIndex.type = table.attributes[attrNo].type;
				tmpIndex.length = table.attributes[attrNo].length;
				char tmpValue[table.attributes[attrNo].length];
				tmpValue = memcpy(tmpValue, tmpBlock+offInRecord, table.attributes[attrNo].length);
				tmpIndex.value = tmpValue;
				indexValues.push_back(tmpIndex);
			}
			tmpBlock += table.totalLength;
		}
	}	
}

//没索引的select
void selectRecord(string DB_Name, vector<string>& columns, TableInfo table, vector<Condition>& conds, Data& dataOut)
{
	Data dataIn;
	getData(DB_Name, table, &dataIn);
	if(columns.size() == 0)
	{//显示记录所有属性
		for (int eveCond = 0; eveCond < conds.size(); ++eveCond)
			for (i = 0; i < dataIn.records.size(); ++i)
			{
				bool isSatisfy = compare(conds[eveCond].value, dataIn.records[i].columns[conds[eveCond].columnNum], table.attributes[conds[eveCond].columnNum].tpye, conds[eveCond].op);
				if(!isSatisfy)
					dataIn.records.erase(dataIn.records.begin()+i);
			}
		dataOut = dataIn;		
	}
	else
	{
		for (int eveCond = 0; eveCond < conds.size(); ++eveCond)
		{
			for (i = 0; i < dataIn.records.size(); ++i)
			{
				bool isSatisfy = compare(conds[eveCond].value, dataIn.records[i].columns[conds[eveCond].columnNum], table.attributes[conds[eveCond].columnNum].tpye, conds[eveCond].op);
				if(!isSatisfy)
					dataIn.records.erase(dataIn.records.begin()+i);
			}
		}
		for (int i = 0; i < dataIn.records.size(); ++i)
		{
			Record tmpRecord;
			tmpRecord.blockNum = dataIn.records[i].blockNum;
			tmpRecord.offset = dataIn.records[i].offset;
			int colNum = 0;
			for (int k = 0; k < table.attrNum; ++k)
			{
				if(table.attributes[k].name == columns[colNum])
				{
					colNum++;
					tmpRecord.columns.push_back(dataIn.records[i].columns[k]);
				}
			}
			dataOut.records.push_back(tmpRecord);
		}
	}		
}

//没索引的delete
void deletRecord(string DB_Name, TableInfo table, vector<Condition>& conds, int& affectedNum)
{
	Data selectedData;
	vector<string> noColumns;
	selectRecord(DB_Name, &noColumns, table, conds, &selectedData);
	affectedNum = selectedData.records.size();
	for (int i = 0; i < affectedNum; ++i)
	{
		BlockInfo* block = bufferManager.getBlock(DB_Name, table.name, selectedData.records[i].blockNum, DATAFILE);
		char empRec[table.totalLength];
		memset(empRec, '0', table.totalLength);
		memmove(cBlock+table.totalLength*selectedData.records[i].offset, empRec, table.totalLength);
	}
}

//有索引的select
void printSelectedRecord(string DB_Name, vector<string>& columns, TableInfo table, Condition& conds, Data& dataOut, vector<Result> results)
{
	if(columns.size() == 0)
	{//显示记录所有属性
		for (int i = 0; i < table.attributes.size(); ++i)
			cout << table.attributes[i].name << '\t' ;
		cout << endl;
		for (i = 0; i < results.size(); ++i)
		{
			BlockInfo* block = bufferManager.getBlock(DB_Name, table.name, results[i].blockNum,DATAFILE);
			char* tmpBlock = block->cBlock;
			for (int j = 0; j < results[i].offsets.size(); ++j)
			{
				tmpBlock += table.totalLength*results[i].offsets[j];
				for (int k = 0; k < table.attributes.size(); ++k)
				{
					int rLength = table.attributes[k].length;
					char printValue[rLength+1];
					while(rLength > 0 && tmpBlock[table.attributes[k].length-rLength] == '0')
						rLength--;
					memmove(printValue, tmpBlock+table.attributes[k].length-rLength, rLength);
					printValue[rLength] = '\0';
					cout << printValue << '\t';
					tmpBlock += table.attributes[k].length;
				}
				cout << endl;
			}
		}
	}
	else
	{
		for (int i = 0; i < columns.size(); ++i)
			cout << columns[i] << '\t' ;
		cout << endl;
		for (i = 0; i < results.size(); ++i)
		{
			BlockInfo* block = bufferManager.getBlock(DB_Name, table.name, results[i].blockNum,DATAFILE);
			char* tmpBlock = block->cBlock;
			for (int j = 0; j < results[i].offsets.size(); ++j)
			{
				int m = 0;
				tmpBlock += table.totalLength*results[i].offsets[j];
				for (int k = 0; k < table.attributes.size(); ++k)
				{
					if(table.attributes[k].name == columns[m])
					{
						m++;
						int rLength = table.attributes[k].length;
						char printValue[rLength+1];
						while(rLength > 0 && tmpBlock[table.attributes[k].length-rLength] == '0')
							rLength--;
						memmove(printValue, tmpBlock+table.attributes[k].length-rLength, rLength);
						printValue[rLength] = '\0';
						cout << printValue << '\t';
					}
					tmpBlock += table.attributes[k].length;
				}
				cout << endl;
			}
		}
	}
}