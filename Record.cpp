#include "Record.h"
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
	int blockAmount = bufferManager.getBlockAmount(DB_Name, table.name, DATAFILE);
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
			char* tmpBlock = block->cBlock + results[i].offsets[j]*table.totalLength;
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

void select(TableInfo table, Data dataIn, Data &dataOut, vector<string>& columns, vector<Condition>& conds)
{
	if(columns.size() == 0)
	{//显示记录所有属性
		for (int eveCond = 0; eveCond < conds.size(); ++eveCond)
			for (i = 0; i < dataIn.records.size(); ++i)
			{
				int k = 0;
				while(table.attributes[k].name != conds[eveCond].columnname)
					k++;
				bool isSatisfy = compare(conds[eveCond].value, dataIn.records[i].columns[k], table.attributes[k].tpye, conds[eveCond].op);
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
				int k = 0;
				while(table.attributes[k].name != conds[eveCond].columnname)
					k++;
				bool isSatisfy = compare(conds[eveCond].value, dataIn.records[i].columns[k], table.attributes[k].tpye, conds[eveCond].op);
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

void coutPrint(vector<string>& columns, Data& dataOut)
{
	for (int i = 0; i < columns.size(); ++i)
		cout << columns[i] << '\t' ;
	cout << endl;
	for (i = 0; i < dataOut.records.size(); ++i)
	{
		for (int j = 0; j < dataOut.records[i].columns.size(); ++j)
			cout << dataOut.records[i].columns[j] << '\t' ;
		cout << endl;
	}
}

//包含生成index模块所需info容器
void insertRecord(string DB_Name, TableInfo table, Data insertedValues, vector<Index> existIndex, vector<IndexInfo>& indexValues)
{
	int record_Num = insertedValues.records.size();
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
		char* tmpBlock = newBlock->cBlock+newBlock->charNum;
		memmove(newBlock->cBlock+newBlock->charNum, stringValues, table.totalLength*record_Num);
		newBlock->charNum += tableLength;
		newBlock->isFull = isFull(table.totalLength, charNum);
		newBlock->dirtyBit = true;
		//indexInfo容器
		int oriOffset = newBlock->charNum / table.totalLength;	//indexInfo
		for (int i = 0; i < record_Num; ++i)
		{
			int avaIndexNum = 0;
			for (int j = 0; j < table.attrNum; ++j)
			{
				if(table.attributes[j].name == existIndex[avaIndexNum].attrName)
				{
					avaIndexNum++;
					IndexInfo tmpIndex;
					tmpIndex.name = table.attributes[j].indexName;
					tmpIndex.tableName = table.name;
					tmpIndex.attrName = table.attributes[j].name;
					tmpIndex.blockNum = newBlock->blockNum;
					tmpIndex.offset = oriOffset+i;
					tmpIndex.type = table.attributes[j].type;
					tmpIndex.length = table.attributes[j].length;
					char tmpValue[table.attributes[j].length];
					memcpy(tmpValue, tmpBlock, table.attributes[j].length);
					tmpIndex.value = tmpValue;
					indexValues.push_back(tmpIndex);
				}
				tmpBlock += table.attributes[j].length;
			}
		}
	}
	else
	{
		int tableLength = BLOCKSIZE- BLOCKSIZE%totalLength;
		int initSize = tableLength - newBlock->charNum;
		char* tmpBlock;
		int oriOffset, insertNum;
		for (int i = 0; i < blockAmount; ++i)
		{
			if (i == 0)
			{
				memmove(newBlock->cBlock+newBlock->charNum, stringValues, initSize);
				tmpBlock = newBlock->cBlock+newBlock->charNum;
				oriOffset = newBlock->charNum / table.totalLength;
				insertNum = initSize / table.totalLength;
			}				
			else
			{
				memmove(newBlock->cBlock, stringValues+initSize+(i-1)*tableLength, tableLength);
				tmpBlock = newBlock->cBlock;
				oriOffset = 0;
				insertNum = BLOCKSIZE%totalLength;
			}	
			newBlock->charNum = tableLength;
			newBlock->isFull = true;
			newBlock->dirtyBit = true;
			//Index容器
			for (int i = 0; i < insertNum; ++i)
			{
				int avaIndexNum = 0;
				for (int j = 0; j < table.attrNum; ++j)
				{
					if(table.attributes[j].name == existIndex[avaIndexNum].attrName)
					{
						avaIndexNum++;
						IndexInfo tmpIndex;
						tmpIndex.name = table.attributes[j].indexName;
						tmpIndex.tableName = table.name;
						tmpIndex.attrName = table.attributes[j].name;
						tmpIndex.blockNum = newBlock->blockNum;
						tmpIndex.offset = oriOffset+i;
						tmpIndex.type = table.attributes[j].type;
						tmpIndex.length = table.attributes[j].length;
						char tmpValue[table.attributes[j].length];
						memcpy(tmpValue, tmpBlock, table.attributes[j].length);
						tmpIndex.value = tmpValue;
						indexValues.push_back(tmpIndex);
					}
					tmpBlock += table.attributes[j].length;
				}
			}
			newBlock = bufferManager.getAvaBlock(DB_Name, table.name);
		}
		//最后一块
		tmpBlock = newBlock->cBlock;
		oriOffset = 0;
		newBlock->charNum = table.totalLength*record_Num - initSize - tableLength*(i-1);
		insertNum = newBlock->charNum / table.totalLength;
		memmove(newBlock->cBlock, stringValues+initSize+(i-1)*tableLength, newBlock->charNum);
		newBlock->isFull = isFull(table.totalLength, charNum);
		newBlock->dirtyBit = true;
		//indexInfo容器
		for (int i = 0; i < insertNum; ++i)
		{
			int avaIndexNum = 0;
			for (int j = 0; j < table.attrNum; ++j)
			{
				if(table.attributes[j].name == existIndex[avaIndexNum].attrName)
				{
					avaIndexNum++;
					IndexInfo tmpIndex;
					tmpIndex.name = table.attributes[j].indexName;
					tmpIndex.tableName = table.name;
					tmpIndex.attrName = table.attributes[j].name;
					tmpIndex.blockNum = newBlock->blockNum;
					tmpIndex.offset = oriOffset+i;
					tmpIndex.type = table.attributes[j].type;
					tmpIndex.length = table.attributes[j].length;
					char tmpValue[table.attributes[j].length];
					memcpy(tmpValue, tmpBlock, table.attributes[j].length);
					tmpIndex.value = tmpValue;
					indexValues.push_back(tmpIndex);
				}
				tmpBlock += table.attributes[j].length;
			}
		}
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
	int blockAmount = bufferManager.getBlockAmount(DB_Name, table.name, DATAFILE);
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
void selectRecord(string DB_Name, vector<string>& columns, TableInfo table, vector<Condition>& conds)
{
	Data dataIn, dataOut;
	getData(DB_Name, table, &dataIn);
	select(table, dataIn, &dataOut, columns, conds);
	coutPrint(columns, &dataOut);	
}

//没索引的delete
void deletRecord(string DB_Name, TableInfo table, vector<Condition>& conds)
{
	Data dataIn, selectedData;
	vector<string> noColumns;
	getData(DB_Name, table, &dataIn);
	select(table, dataIn, &selectedData, &noColumns, conds);
	int affectedNum = selectedData.records.size();
	for (int i = 0; i < affectedNum; ++i)
	{
		BlockInfo* block = bufferManager.getBlock(DB_Name, table.name, selectedData.records[i].blockNum, DATAFILE);
		char empRec[table.totalLength];
		memset(empRec, '0', table.totalLength);
		memmove(cBlock+table.totalLength*selectedData.records[i].offset, empRec, table.totalLength);
		block->dirtyBit = true;
	}
}

//有索引的select
void printSelectedRecord(string DB_Name, vector<string>& columns, TableInfo table, vector<Condition>& conds, vector<Result> results)
{
	Data dataIn, dataOut;
	getIndexData(DB_Name, table, results, &dataIn);
	select(table, dataIn, &dataOut, columns, conds);
	coutPrint(columns, &dataOut);
}

//有索引的delete
void deleteIndexRecord(string DB_Name, TableInfo table, vector<Condition>& conds, vector<Result> results, vector<Index> existIndex, vector<IndexInfo>& indexValues)
{
	Data dataIn, selectedData;
	vector<string> noColumns;
	getIndexData(DB_Name, table, results, &dataIn);
	select(table, dataIn, &selectedData, &noColumns, conds);
	int affectedNum = selectedData.records.size();
	for (int i = 0; i < affectedNum; ++i)
	{
		BlockInfo* block = bufferManager.getBlock(DB_Name, table.name, selectedData.records[i].blockNum, DATAFILE);
		char empRec[table.totalLength];
		memset(empRec, '0', table.totalLength);
		memmove(cBlock+table.totalLength*selectedData.records[i].offset, empRec, table.totalLength);
		block->dirtyBit = true;
	}
	for (int i = 0; i < selectedData.records.size(); ++i)
	{
		int avaIndex = 0;
		for (int j = 0; j < table.attrNum; ++j)
		{
			if (table.attributes[j].indexName == existIndex[avaIndex])
			{
				IndexInfo tmpIndex;
				tmpIndex.name = existIndex[avaIndex];
				tmpIndex.tableName = table.name;
				tmpIndex.attrName = table.attributes[j].name;
				tmpIndex.type = table.attributes[j].type;
				tmpIndex.length = table.attributes[j].length;
				tmpIndex.value = selectedData.records[i].columns[j];
				indexValues.push_back(tmpIndex);
			}
		}
	}
}