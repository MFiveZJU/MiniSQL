/***********************************
**Tips
**1.功能里面还应该有根据判断语义是否正确的操作
**
***********************************/

#if !defined(_API_H_)
#define _API_H_

#include "record_manager.h"
#include "buffer_manager.h"
#include "index_manager.h"
#include "catalog_manager.h"
#include "interpreter.h"

class Api{
public:
	Api(){}
	virtual ~Api()
	static bool createDatabase(string dbName);
	static bool dropDatabase(string dbName);
	static bool useDatabase(string dbName);
	static bool createTable(Table table);
	static bool dropTable(Table table);
	static bool createIndex(Index index);
	static bool dropIndex(Index index);
	static bool selectRecord(string tableName,vector<string>& columns,vector<Condition>& conds);
	static bool insertRecord(string tableName,vector<string>& values);
	static bool deleteRecord(string tableName);
	static bool quit();
	// static void execfile(string fileName);
};

#endif