#ifndef __CATALOG_H__
#define __CATALOG_H__

#include "Definition.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

class catalogManager
{
private:
	string dbName;
	vector<table_info> tables;	//全部表
	int tableNum;			//数据库内表的数量,等于tables.size()
	vector<index_info> indexes;
	int indexNum;
public:
	catalogManager()
	{
		cout << "Specify Database Name: ";
		string temp;
		cin >> temp;
		Use_Database(temp);
	};
	catalogManager(string name):dbName(name)
	{
		cout << "catalogManager(string name);" << endl;//test
		initialCatalog();
	};
	~catalogManager()
	{
		storeCatalog();
	};
	//在开始执行功能前将文件内信息读入内存,结束时写回硬盘
	bool initialCatalog();
	bool storeCatalog();
	bool Create_Database(string DB_Name);
	bool Create_Table(table_info newTable, string DB_Name);
	bool Create_Index(index_info index, string DB_Name);
	bool Drop_Database(string DB_Name);
	bool Drop_Table(string Table_Name,string DB_Name);
	bool Drop_Index(string Index_Name,string DB_Name);
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
			Use_Database(DB_Name);
		for(int i = 0; i < tableNum; i++)
			if(tables[i].name == Table_Name)
				return true;

		return false;
	}
	bool existAttr(string Attr_Name, string Table_Name, string DB_Name)
	{
		if(DB_Name != dbName)
			Use_Database(DB_Name);
		for(int i = 0; i < tableNum; i++)
			if(tables[i].name == Table_Name)
				for (int j = 0; j < tables[i].attrNum; j++)
					if(tables[i].attributes[j].name == Attr_Name)
						return true;
	}
	bool existIndex(string Index_Name, string DB_Name)
	{
		if(DB_Name != dbName)
			Use_Database(DB_Name);
		for(int i = 0; i < indexNum; i++)
			if(indexes[i].name == Index_Name)
				return true;

		return false;
	}
	bool existIndex(string Table_Name, string Attr_Name, string DB_Name)
	{
		if(DB_Name != dbName)
			Use_Database(DB_Name);
		for(int i = 0; i < tableNum; i++)
			if(tables[i].name == Table_Name)
				for(int j = 0; j < tables[i].attrNum; j++)
					if(tables[i].attributes[j].name == Attr_Name)
						return true;

		return false;
	}
	table_info Get_Table_Info(string DB_Name, string Table_Name);
	index_info Get_Index_Info(string DB_Name, string Index_Name);
	index_info Get_Index_Info(string DB_Name, string Table_Name, string Attr_Name);
	void Use_Database(string DB_Name)
	{
		if(dbName.empty())
			storeCatalog();
		dbName = DB_Name;
		initialCatalog();
	};
};

bool catalogManager::initialCatalog()
{
	string filename = dbName + "_table.catalog";
	fstream  fin(filename.c_str(), ios::in);
	if(!fin)
		return false;
	fin >> tableNum;
	for(int i = 0; i < tableNum; i++)
	{//fill in the vector of tables
		table_info temp_table;
		fin >> temp_table.name;
		fin >> temp_table.attrNum;
		fin >> temp_table.blockNo;
		for(int j = 0; j < temp_table.attrNum; j++)
		{//fill in the vector of temp_table.attributes
			attr_info temp_attr;
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
		index_info temp_index;
		fin >> temp_index.name;
		fin >> temp_index.tableName;
		fin >> temp_index.attrName;
		fin >> temp_index.blockNo;
	 	indexes.push_back(temp_index);
	}
	fin.close();

	return true;
}

bool catalogManager::storeCatalog()
{
	string filename = dbName + "_table.catalog";
	fstream  fout(filename.c_str(), ios::out);
	fout << tableNum << endl;
	for(int i = 0; i < tableNum; i++)
	{
		fout << tables[i].name << " ";
		fout << tables[i].attrNum <<" ";
		fout << tables[i].blockNo << " ";
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
		fout << indexes[i].attrName << " ";
		fout << indexes[i].blockNo << endl;
	}
	fout.close();

	return true;
}

bool catalogManager::Create_Database(string DB_Name)
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

bool catalogManager::Create_Table(table_info newTable, string DB_Name)
{
	if(DB_Name != dbName)
		Use_Database(DB_Name);
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
	file.close();
	tableNum++;
 	tables.push_back(newTable);

 	return true;
}

bool catalogManager::Create_Index(index_info index, string DB_Name)
{
	if(DB_Name != dbName)
		Use_Database(DB_Name);
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

bool catalogManager::Drop_Database(string DB_Name)
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

bool catalogManager::Drop_Table(string Table_Name, string DB_Name)
{
	if(DB_Name != dbName)
		Use_Database(DB_Name);
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

bool catalogManager::Drop_Index(string Index_Name,string DB_Name)
{
	if(DB_Name != dbName)
		Use_Database(DB_Name);
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

table_info catalogManager::Get_Table_Info(string DB_Name, string Table_Name)
{
	if(DB_Name != dbName)
		Use_Database(DB_Name);
	for(int i = 0; i < tableNum; i++)
	{
		if(tables[i].name == Table_Name)
			return tables[i];
	}
}

index_info catalogManager::Get_Index_Info(string DB_Name, string Index_Name)
{
	if(DB_Name != dbName)
		Use_Database(DB_Name);
	for(int i = 0; i < indexNum; i++)
	{
		if(indexes[i].name == Index_Name)
			return indexes[i];
	}
}

index_info catalogManager::Get_Index_Info(string DB_Name, string Table_Name, string Attr_Name)
{
	if(DB_Name != dbName)
		Use_Database(DB_Name);
	for(int i = 0; i < indexNum; i++)
	{
		if(indexes[i].tableName == Table_Name && indexes[i].attrName == Attr_Name)
			return indexes[i];
	}
}

#endif
