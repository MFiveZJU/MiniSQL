#ifndef __CATALOG_H__
#define __CATALOG_H__

#include "Definition.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

class CatalogManager
{
private:
	string dbName;
	vector<TableInfo> tables;	//全部表
	int tableNum;			//数据库内表的数量,等于tables.size()
	vector<Index> indexes;
	int indexNum;
public:
	CatalogManager()
	{
		cout << "Specify Database Name: ";
		string temp;
		cin >> temp;
		useDatabase(temp);
	};
	CatalogManager(string name):dbName(name)
	{
		initialCatalog();
	};
	~CatalogManager()
	{
		storeCatalog();
	};
	//在开始执行功能前将文件内信息读入内存,结束时写回硬盘
	bool initialCatalog();
	bool storeCatalog();
	bool createDatabase(string DB_Name);
	bool createTable(TableInfo newTable, string DB_Name);
	bool createIndex(Index index, string DB_Name);
	bool dropDatabase(string DB_Name);
	bool dropTable(string Table_Name,string DB_Name);
	bool dropIndex(string Index_Name,string DB_Name);
	bool existDB(string DB_Name)
	{
		const string filename = DB_Name + "_table.catalog";
		fstream file(filename.c_str(), ios::in);
		if(!file)
			return false;
		file.close();

		return true;
	}
	bool existTable(string Table_Name, string DB_Name)
	{
		if(DB_Name != dbName)
			useDatabase(DB_Name);
		for(int i = 0; i < tableNum; i++)
			if(tables[i].name == Table_Name)
				return true;

		return false;
	}
	bool existAttr(string Attr_Name, string Table_Name, string DB_Name)
	{
		if(DB_Name != dbName)
			useDatabase(DB_Name);
		for(int i = 0; i < tableNum; i++)
			if(tables[i].name == Table_Name)
				for (int j = 0; j < tables[i].attrNum; j++)
					if(tables[i].attributes[j].name == Attr_Name)
						return true;
	}
	bool existIndex(string Index_Name, string DB_Name)
	{
		if(DB_Name != dbName)
			useDatabase(DB_Name);
		for(int i = 0; i < indexNum; i++)
			if(indexes[i].name == Index_Name)
				return true;

		return false;
	}
	bool existIndex(string Table_Name, string Attr_Name, string DB_Name)
	{
		if(DB_Name != dbName)
			useDatabase(DB_Name);
		for(int i = 0; i < tableNum; i++)
			if(tables[i].name == Table_Name)
				for(int j = 0; j < tables[i].attrNum; j++)
					if(tables[i].attributes[j].name == Attr_Name)
						return true;

		return false;
	}
	TableInfo getTableInfo(string DB_Name, string Table_Name);
	Index getIndexInfo(string DB_Name, string Index_Name);
	Index getIndexInfo(string DB_Name, string Table_Name, string Attr_Name);
	////To be continued
	bool getAllIndex(string DB_Name, string Table_Name, vectror<Index>& allIndex);
	void useDatabase(string DB_Name)
	{
		if(dbName.empty())
			storeCatalog();
		dbName = DB_Name;
		initialCatalog();
	};
};

bool CatalogManager::initialCatalog()
{
	string filename = dbName + "_table.catalog";
	fstream  fin(filename.c_str(), ios::in);
	if(!fin)
		return false;
	fin >> tableNum;
	for(int i = 0; i < tableNum; i++)
	{//fill in the vector of tables
		TableInfo temp_table;
		fin >> temp_table.name;
		fin >> temp_table.attrNum;
		//fin >> temp_table.blockNum;
		for(int j = 0; j < temp_table.attrNum; j++)
		{//fill in the vector of temp_table.attributes
			AttrInfo temp_attr;
	 		fin >> temp_attr.name;
	 		fin >> temp_attr.type;
	 		fin >> temp_attr.length;
	 		fin >> temp_attr.isPrimeryKey;
	 		fin >> temp_attr.indexName;
	 		temp_table.attributes.push_back(temp_attr);
	 		temp_table.totalLength += temp_attr.length;
	 	}
	 	tables.push_back(temp_table);
	}
	fin.close();
	filename = dbName + "_index.catalog";
	fin.open(filename.c_str(), ios::in);
	if(!fin)
		return false;
	fin >> indexNum;
	for(int i = 0; i < indexNum; i++)
	{//fill in the vector of tables
		Index temp_index;
		fin >> temp_index.name;
		fin >> temp_index.tableName;
		fin >> temp_index.attrName;
		//fin >> temp_index.blockNum;
	 	indexes.push_back(temp_index);
	}
	fin.close();

	return true;
}

bool CatalogManager::storeCatalog()
{
	string filename = dbName + "_table.catalog";
	fstream  fout(filename.c_str(), ios::out);
	fout << tableNum << endl;
	for(int i = 0; i < tableNum; i++)
	{
		fout << tables[i].name << " ";
		fout << tables[i].attrNum <<" ";
		//fout << tables[i].blockNum << " ";
		fout << tables[i].totalLength << endl;
		for(int j = 0; j < tables[i].attrNum; j++)
		{
			fout << tables[i].attributes[j].name << " ";
			fout << tables[i].attributes[j].type << " ";
			fout << tables[i].attributes[j].length << " ";
			fout << tables[i].attributes[j].isPrimeryKey  << " ";
			fout << tables[i].attributes[j].indexName << endl;
	 	}
	}
	fout.close();
	filename = dbName + "_index.catalog";
	fout.open(filename.c_str(), ios::out);
	fout << indexNum << endl;
	for(int i = 0; i < indexNum; i++)
	{
		fout << indexes[i].name << " ";
		fout << indexes[i].tableName <<" ";
		//fout << indexes[i].attrName << " ";
		//fout << indexes[i].blockNum << endl;
		fout << indexes[i].attrName << endl;
	}
	fout.close();

	return true;
}

bool CatalogManager::createDatabase(string DB_Name)
{
	if(existDB(DB_Name))
		return false;
	string filename = DB_Name + "_table.catalog";
	ofstream fout(filename.c_str(), ios::out);
	fout << 0 << endl;
	fout.close();
	filename = DB_Name + "_index.catalog";
	fout.open(filename.c_str(), ios::out);
	fout << 0 << endl;
	fout.close();

	return true;
}
//emptyAmount/emptyBlock[100]:-1。
bool CatalogManager::createTable(TableInfo newTable, string DB_Name)
{
	if(DB_Name != dbName)
		useDatabase(DB_Name);
	const string filename = DB_Name + "_" + newTable.name + ".db";
	fstream file;
	file.open(filename.c_str(), ios::in);
	if(file)
	{
		file.close();
		return false;
	}
	file.open(filename.c_str(), ios::out);
	file << 0;          //blockAmount
	file << tableNum;   //recordLength
	file << 0;          //recordAmount
	file << 0;			//emptyAmount
	int emptyBlock[100];
	memset(emptyBlock, -1, sizeof(emptyBlock));
	for (int i = 0; i < 100; ++i)
		file << emptyBlock[i];
	file.close();
	tableNum++;
 	tables.push_back(newTable);

 	return true;
}

bool CatalogManager::createIndex(Index index, string DB_Name)
{
	if(DB_Name != dbName)
		useDatabase(DB_Name);
	const string filename = DB_Name + "_" + index.tableName + "_" + index.attrName + "_" + index.name + ".index";
	fstream file;
	file.open(filename.c_str(), ios::in);
	if(file)
	{
		file.close();
		return false;
	}
	file.open(filename.c_str(), ios::out);
	file << 0;//blockAmount
	file.close();
	indexes.push_back(index);
	for (int i = 0; i < tableNum; ++i)
		if(tables[i].name == index.tableName)
			for(int j = 0; j < tables[i].attrNum; j++)
				if(tables[i].attributes[j].name == index.attrName)
					tables[i].attributes[j].indexName = index.name;

	return true;
}

bool CatalogManager::dropDatabase(string DB_Name)
{
	string filename = DB_Name + "_table.catalog";
	if(!remove(filename.c_str()))
		return false;
	filename = DB_Name + "_index.catalog";
	if(!remove(filename.c_str()))
		return false;
	for (int i = 0; i < tableNum; ++i)
	{
		filename = DB_Name + "_" +tables[i].name + ".db";
		if(!remove(filename.c_str()))
			return false;
	}
	for (int i = 0; i < indexNum; ++i)
	{
		filename = DB_Name + "_" + indexes[i].tableName + "_" + indexes[i].attrName + "_" + indexes[i].name + ".index";
		if(!remove(filename.c_str()))
			return false;
	}
}

bool CatalogManager::dropTable(string Table_Name, string DB_Name)
{
	if(DB_Name != dbName)
		useDatabase(DB_Name);
	const string filename = DB_Name + "_" + Table_Name + ".db";
	if(!remove(filename.c_str()))
		return false;
	for(int i = tableNum -1; i >= 0; i--)
	{
		if(tables[i].name == Table_Name)
		{
			tables.erase (tables.begin()+i);
			tableNum--;
			return true;
		}
	}

	return false;
}

bool CatalogManager::dropIndex(string Index_Name,string DB_Name)
{
	if(DB_Name != dbName)
		useDatabase(DB_Name);
	for(int i = indexNum -1; i >= 0; i--)
	{
		if(indexes[i].name == Index_Name)
		{
			for (int j = 0; j < tableNum; j++)
				if(tables[j].name == indexes[i].tableName)
					for(int k = 0; k < tables[i].attrNum; k++)
						if(tables[j].attributes[k].name == indexes[i].attrName)
							tables[j].attributes[k].indexName = "";
			const string filename = DB_Name + "_" + indexes[i].tableName + "_" + indexes[i].attrName + "_" + Index_Name + ".index";
			if(!remove(filename.c_str()))
				return false;
			indexes.erase(indexes.begin()+i);
			indexNum--;
			return true;
		}
	}

	return false;
}

TableInfo CatalogManager::getTableInfo(string DB_Name, string Table_Name)
{
	if(DB_Name != dbName)
		useDatabase(DB_Name);
	for(int i = 0; i < tableNum; i++)
	{
		if(tables[i].name == Table_Name)
			return tables[i];
	}
}

Index CatalogManager::getIndexInfo(string DB_Name, string Index_Name)
{
	if(DB_Name != dbName)
		useDatabase(DB_Name);
	for(int i = 0; i < indexNum; i++)
	{
		if(indexes[i].name == Index_Name)
			return indexes[i];
	}
}

Index CatalogManager::getIndexInfo(string DB_Name, string Table_Name, string Attr_Name)
{
	if(DB_Name != dbName)
		useDatabase(DB_Name);
	for(int i = 0; i < indexNum; i++)
	{
		if(indexes[i].tableName == Table_Name && indexes[i].attrName == Attr_Name)
			return indexes[i];
	}
}

#endif
