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
#define LT			100			//less than <
#define LE			101			//less or equal <=
#define	GT			102			//great than >
#define GE			103			//great or equal >=
#define EQ			104			//equal =
#define NE			105			//not equal <>

#define INT			201
#define CHAR		202
#define FLOAT		203

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

class Attribute  //表的属性类
{
public:
    string name;
    int type;
    bool isPrimeryKey;
    bool isUnique;
    int length;
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
        cout<<"name="<<name<<"  type="<<type<<"  length=" << length << "  isPrimeryKey="<<isPrimeryKey<<"  isUnique="<<isUnique<<endl;
    }
};

class Table //Table类，用来表示一个关系
{
public:
    string name;   //all the datas is store in file name.table
    string primarykey_name;
    int attriNum;	//the number of attributes in the tables
    vector<Attribute> attributes;
    Table(): attriNum(0){}
    void debug(){
        cout<<name<<" "<<primarykey_name<<" "<<attriNum<<endl;
        for(int i=0;i<attributes.size();i++)
            attributes.at(i).debug();
    }
};

class Index //Index类，用来表示一个索引
{
public:
    string index_name;	//all the datas is store in file index_name.index
    string table_name;  //the name of the table on which the index is create
//    int column;			//on which column the index is created
    string column_name;
    Index(): index_name(""), table_name(""){}
    void debug(){
        cout<< index_name << " " << table_name << " "<< column_name << endl;
    }
};

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


#endif /* defined(__miniSQL2__Interpreter__) */
