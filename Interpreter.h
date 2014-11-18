//
//  Interpreter.h
//  miniSQL2
//
//  Created by Leon on 14/11/8.
//  Copyright (c) 2014年 ZJU. All rights reserved.
//

#ifndef __miniSQL2__Interpreter__
#define __miniSQL2__Interpreter__

#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdio.h>
#define BLOCK_LEN 4096
#define MAX_BLOCK 500
#define MAX_RECORD 10100

#define LT			100			//less than
#define LE			101			//less or equal
#define	GT			102			//great than
#define GE			103			//great or equal
#define EQ			104			//equal
#define NE			105			//not equal

#define INT			201
#define CHAR		202
#define FLOAT		203

//#include "API.h"
//#include "CatalogManager.h"
//extern API myAPI;
//extern CatalogManager mycatalog;
using namespace std;

class Interpreter
{
public:
    Interpreter(void);
    ~Interpreter(void);
    int run(string SQL,string& execfilename);
    string read_input();//读取用户一条记录（以；结束）
    string read_input(ifstream& fin);
    string process(string& SQL);
    string getword(string& sql );
    
};


using namespace std;


class Attribute  //表的属性类
{
public:
    string name;
    int type;
    int length;
    bool isPrimeryKey;
    bool isUnique;
    Attribute()
    {
        isPrimeryKey=false;
        isUnique=false;
    }
    Attribute(string n, int t, int l, bool isP, bool isU)
    :name(n), type(t), length(l), isPrimeryKey(isP), isUnique(isU){}
    void init(){
        name ="";
        type = 0;
        length = 0;
        isPrimeryKey = false;
        isUnique = false;
    }
    void debug(){ //调试时使用
        cout<<"name="<<name<<"  type="<<type<<"  length="<<length<<"  isPrimeryKey="<<isPrimeryKey<<"  isUnique="<<isUnique<<endl;
    }
};

class Table //Table类，用来表示一个关系
{
public:
    string name;   //all the datas is store in file name.table
    string primarykey_name;
    int blockNum;	//number of block the datas of the table occupied in the file name.table
    int recordNum;	//number of record in name.table(include delete record)
    int attriNum;	//the number of attributes in the tables
    int totalLength;	//total length of one record, should be equal to sum(attributes[i].length)
    bool mark[MAX_RECORD];  //用来标记惰性删除的数组
    vector<Attribute> attributes;
    Table(): blockNum(0), attriNum(0), totalLength(0),recordNum(0){
        for(int i=0;i<MAX_RECORD;i++)
            mark[i] = 0;
    }
    void debug(){
        cout<<name<<" "<<primarykey_name<<" "<<blockNum<<" "<<attriNum<<endl;
        for(int i=0;i<attributes.size();i++)
            attributes.at(i).debug();
    }
};

class Index //Index类，用来表示一个索引
{
public:
    string index_name;	//all the datas is store in file index_name.index
    string table_name;	//the name of the table on which the index is create
    int column;			//on which column the index is created
    int columnLength;
    string column_name;
    int blockNum;		//number of block the datas of the index occupied in the file index_name.table
    Index(): column(0), blockNum(0),index_name(""),table_name(""){}
    void debug(){
        cout<<index_name<<" "<<table_name<<" "<<columnLength<<blockNum<<endl;
    }
};

class Record   //Record类，用来存储一条记录
{
public:
    vector<string> columns;
    
};
class Data//这样就把Data搞成了一个二维数组
{
public:
    vector<Record> records;
    vector<int> location;
};

//enum Comparison{LT, LE, GT, GE, EQ, NE};//stants for less than, less equal, greater than, greater equal, equal, not equal respectivly
class Condition{
public:
    int op;
    int columnNum;
    string columname;
    string value;
    void debug(){
        cout<<columname<<" "<<op<<" "<<value<<endl;
    }
};

class insertPos{
public:
    int bufferNUM;
    int position;
};

#endif /* defined(__miniSQL2__Interpreter__) */
