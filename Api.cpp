#include "Api.h"

bool Api::createDatabase(string dbName){
	return CatalogManager::createDatabase(dbName);
}

bool dropDatabase(string dbName){
	return CatalogManager::dropDatabase(dbName);
}

bool useDatabase(string dbName){
	// DB_Name = dbName;	//DB_Name是一个全局变量
	return CatalogManager::useDatabase(dbName)
}

bool Api::createTable(Table table){
	return CatalogManager::createTable(table);
}

bool Api::dropTabel(Table table){
	return CatalogManager::dropTable(table);
}

bool Api::createIndex(Index index){
	return CatalogManager::createIndex(index);
}

bool Api::dropIndex(Index index){
	return CatalogManager::dropIndex(index);
}

bool Api::selectRecord(string tableName,vector<string>& columns,vector<Condition>& conds){
	return RecordManager::selectRecord(tableName,columns,conds);
}

bool Api::insertRecord(string tableName,vector<string>& values){
	return RecordManager::insertRecord(tableName,values);
}

bool Api::deleteRecord(string tableName,vector<Condition>& conds){
	return RecordManager::deleteRecord(tableName,conds);
}

bool Api::quit(){
	return quitProc()
}