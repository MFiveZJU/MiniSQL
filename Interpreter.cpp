//
//  Interpreter.cpp
//  miniSQL2
//
//  Created by Leon on 14/11/8.
//  Copyright (c) 2014年 ZJU. All rights reserved.
//
//  怎么识别'和"？insert语句有问题
#include "Interpreter.h"



Interpreter::Interpreter(void)
{
}
Interpreter::~Interpreter(void)
{
}

//从控制台读取用户的一条SQL语句，假设新的一条语句从新的一行开始
string Interpreter::read_input(){
    string temp;
    string result;
    bool mark =true;
    cin>>temp;
    result +=temp;
    result +=" ";
    while(temp.find(";")==temp.npos){
        cin >> temp;
        result +=temp;
        result +=" ";
    }
    cout << "命令行输入: " << result << endl;
    return result;
    
}
//从文件读取用户的SQL语句
string Interpreter::read_input(ifstream& fin){
    string temp;
    string result;
    bool mark =true;
    while(temp.find(";")==temp.npos){
        fin >> temp;
        if(temp.compare("")==0)
            return "";
        result +=temp;
        result +=" ";
    }
    cout << "文件输入：" << result <<endl;
    return result;
}
//把 （ ） ， ；* " ' 等都用空格分开
string Interpreter::process(string& SQL){
    int i;
    string result;
    for(i=0;i<SQL.length();i++){
        if(SQL[i]==',')
            result +=" , ";
        else if(SQL[i]=='(')
            result +=" ( ";
        else if(SQL[i]==')')
            result +=" ) ";
        else if(SQL[i]==';')
            result +=" ; ";
        else if(SQL[i]=='*')
            result +=" * ";
        else if(SQL[i]=='=')
            result +=" = ";
        else if(SQL[i]=='>'){
            if (SQL[i+1]=='=') {
                result += " >= ";
                i++;
            }
            else{
                result += " > ";
            }
        }
        else if(SQL[i]=='<'){
            if (SQL[i+1]=='=') {
                result += " <= ";
                i++;
            }
            else if(SQL[i+1]=='>'){
                result += " <> ";
                i++;
            }
            else{
                result += " < ";
            }
        }
        else
            result +=SQL[i];
    }
    cout << "加完空格：" << result << endl;
    return result;
}
//获取用空格分开的word
string Interpreter::getword(string& sql ){
    string word;
    for (int i=0;i<sql.length();i++){
        if(sql[i]!=' ')
            word +=sql[i];
        else{
            while(sql[i]==' '&&i+1<sql.length()){
                i++;
            }
            sql.erase(0,i);
            break;
        }
    }
    if(word =="")
        word = "XXX";
    //cout <<word<<endl;
    return word;
}
//处理一条语句，为了简化假设语句的书写格式都较规范 语法错误主要是大错误
int Interpreter::run(string SQL,string& execfilename){
    string opcode;
    string temp, s1,s2,s3;
    SQL = process(SQL);
    opcode = getword(SQL);
    if(opcode.compare("create")==0){
        temp = getword(SQL);
        if (temp.compare("database") == 0){
            s1 = getword(SQL);
            if (s1.compare(";") == 0) {
                cout << "Expected database name" << endl;
                return 1;
            }
            temp = getword(SQL);
            if (temp.compare(";") != 0) {
                cout << "Expected ';'" << endl;
                return 1;
            }
            cout << "create database! database name: " << s1 << endl;//test
            return 0;
        }
        if(temp.compare("table")==0){
            Table table;
            Attribute attr;
            string temp = getword(SQL);    //table name
            table.name = temp;
            temp = getword(SQL);
            if(temp.compare("(")!=0){
                cout<<"Expected '(' after 'table'"<<endl;
                return 1; //语法错误
            }
            temp = getword(SQL);
            while(temp.compare(")")!=0){  //还没有处理完属性
                if(temp.compare("primary")==0){  //primary语句
                    temp = getword(SQL);
                    if(temp.compare("key")!=0){
                        cout<<"Expected 'key'"<<endl;
                        return 1; //primary 后面没有跟key
                    }
                    temp = getword(SQL);
                    if(temp.compare("(")!=0){
                        cout<<"Expected '(' after 'primary key'"<<endl;
                        return 1;
                    }
                    temp = getword(SQL);
                    table.primarykey_name=temp;
                    for(int i=0;i<table.attributes.size();i++)
                        if(table.attributes.at(i).name.compare(temp)==0)
                            table.attributes.at(i).isPrimeryKey = true;
                    temp = getword(SQL);
                    if(temp.compare(")")!=0){
                        cout<<"Expected ')'"<<endl;
                        return 1;
                    }
                    temp = getword(SQL);
                    break;
                }
                attr.init();
                attr.name = temp;
                temp = getword(SQL);
                if(temp.compare("int")==0){
                    attr.length=4;
                    attr.type = INT;
                }
                else if(temp.compare("float")==0){
                    attr.length=4;
                    attr.type = FLOAT;
                }
                else if(temp.compare("char")==0){ //SQL经过预先处理("("前面加空格)， char(12)会转变成 char (12);
                    temp = getword(SQL);
                    if(temp.compare("(")!=0){
                        cout<<"Do you input the number of char?"<<endl;
                        return 1;
                    }
                    temp = getword(SQL);
                    attr.length = atoi(temp.c_str());
                    attr.type = CHAR;
                    temp = getword(SQL);
                    if(temp.compare(")")!=0){
                        cout<<"Expected')'"<<endl;
                        return 1;
                    }
                }
                else
                    return 2; //不支持该数据类型
                s1 = getword(SQL);
                if(s1.compare("unique")==0){
                    attr.isUnique = true;
                    table.attriNum++;
                    table.attributes.push_back(attr);
                    temp = getword(SQL);
                    if(temp.compare(",")!=0){
                        cout<<"Expected','"<<endl;
                        return 1;
                    }
                    temp = getword(SQL);
                    continue;
                }
                else{  //没有unique
                    if(s1.compare(",")!=0&&s1.compare(")")!=0){
                        cout<<"Expected','"<<endl;
                        return 1;
                    }
                    table.attriNum++;
                    table.attributes.push_back(attr);
                    temp = getword(SQL);
                    continue;
                }
            }
            temp = getword(SQL);
            if(temp.compare(";")!=0){
                cout<<"Expected';'"<<endl;
                return 1;
            }
//            myAPI.createtable(table);
            table.debug(); //test
            return 0; //正确执行
        }
        else if(temp.compare("index")==0){
            Index index;
            s1 = getword(SQL);
            index.index_name = s1;
            s1 = getword(SQL);
            if(s1.compare("on")!=0){
                cout<<"Expected'on'"<<endl;
                return 1;//语法错误
            }
            s1 = getword(SQL);
            index.table_name = s1;
            s1 = getword(SQL);
            if(s1.compare("(")!=0){
                cout<<"Expected'('"<<endl;
                return 1;
            }
            s1 = getword(SQL);
            index.column_name = s1;
            s1 = getword(SQL);
            if(s1.compare(")")!=0){
                cout<<"Expected')'"<<endl;
                return 1;
            }
            s1 = getword(SQL);
            if(s1.compare(";")!=0){
                cout<<"Expected';'"<<endl;
                return 1;
            }
//            myAPI.createindex(index);
            index.debug();//test
            return 0; //执行成功
            
        }
        else{
            cout<<"We can only create database, table or index!"<<endl;
            return 1;
        }
    }
    else if(opcode.compare("drop")==0){
        temp = getword(SQL);
        if(temp.compare("database")==0){
            s1= getword(SQL);
            cout << "drop database! database name: " << s1 << endl;//test
            //            myAPI.dropdatabase(s1);
            return 0;
        }
        if(temp.compare("table")==0){
            s1= getword(SQL);
            cout << "drop table! table name: " << s1 << endl;//test
//            myAPI.droptable(s1);
            return 0;
        }
        else if(temp.compare("index")==0){
            s1 = getword(SQL);
//            myAPI.dropindex(s1);
            cout << "drop index! index name: " << s1 << endl;//test
            return 0;
        }
        else{
            cout<<"We can only drop database, table or index!"<<endl;
            return 1;
        }
    }
    
    else if(opcode.compare("select")==0){
        vector<Condition> conds;
        vector<string> colomns;
        Condition cond;
        string tablename;
        temp = getword(SQL);
        if (temp.compare("*") == 0) {
            colomns.push_back(temp);
            temp = getword(SQL);
        }
        else{
            colomns.push_back(temp);
            temp = getword(SQL);
            
            while (temp.compare(",") == 0) {
                temp = getword(SQL);
                colomns.push_back(temp);
                temp = getword(SQL);
            }
        }
//        if(temp.compare("*")!=0){
//            cout<<"We can only support select * "<<endl;
//            return 1;
//        }
//        temp = getword(SQL);
        if(temp.compare("from")!=0){
            cout<<"Expect 'from'"<<endl;
            return 1;
        }
        tablename = getword(SQL);
        temp = getword(SQL);
        if(temp.compare("where")==0){//有where语句
            s1 = getword(SQL);
            while(s1.compare(";")!=0){   //处理多个条件
                cond.columname = s1;
                s2 = getword(SQL);  //条件
                if(s2.compare("=")==0){
                    cond.op = EQ;
                }
                else if(s2.compare("<>")==0){
                    cond.op = NE;
                }
                else if(s2.compare("<")==0){
                    cond.op = LT;
                }
                else if(s2.compare(">")==0){
                    cond.op = GT;
                }
                else if(s2.compare("<=")==0){
                    cond.op = LE;
                }
                else if(s2.compare(">=")==0){
                    cond.op = GE;
                }
                else{
                    cout<<"Not support this kind of op"<<endl;
                    return 1;
                }
                s3 = getword(SQL);
                /*if(s3[0]=='\''){
                 cond.value = "";
                 for(int i=1;i<s3.length()-1;i++){
                 cond.value +=s3[i];
                 }
                 }
                 else{ */
                cond.value = s3;
                //}
                conds.push_back(cond);
                s1 = getword(SQL);
                if(s1.compare(";")==0)
                    break;
                s1 = getword(SQL);
            }
            
        }
        cout << "select ";
        for (int i=0; i < colomns.size(); i++) {
            cout << colomns[i] << " ";
        }
        cout << "from " << tablename << " where" << endl;
        for (int i=0; i < conds.size(); i++) {
            cout << conds[i].columname << " " << conds[i].op << " " << conds[i].value << endl;
        }
        //cout <<"==" <<  conds[0].op << " " << conds.size() << endl;
//        myAPI.select(tablename, colomns, conds);//colomns, conds可能为空 也可能不为空
        return 0;
    }	
    else if(opcode.compare("insert")==0){
        temp = getword(SQL);
        string tablename;
        vector<string> values;
        if(temp.compare("into")!=0){
            cout<<"Expected 'into'"<<endl;
            return 1;
        }
        tablename = getword(SQL);
        temp = getword(SQL);
        if(temp.compare("values")!=0){
            cout<<"Expected 'values'"<<endl;
            return 1;
        }
        temp = getword(SQL);
        if(temp.compare("(")!=0){
            cout<<"Expected '('"<<endl;
            return 1;
        }
        temp = getword(SQL);
//        cout << temp <<endl;
        while(temp.compare(";")!=0){
//            if ((temp.compare("\'") == 0) || (temp.compare("\"") == 0)) {
//                temp = getword(SQL);
//            }
            values.push_back(temp);
            temp = getword(SQL);
//            if ((temp.compare("\'") == 0) || (temp.compare("\"") == 0)) {
//                temp = getword(SQL);
//            }
            temp = getword(SQL);
        }
        cout << "insert into values(";
        for (int i = 0; i <=values.size(); i++) {
            cout << " " << values[i];
        }
        cout << ")" << endl;
//        myAPI.insert(tablename,values);
        return 0;
    }
    else if(opcode.compare("delete")==0){
        temp = getword(SQL);
        Condition cond;
        string tablename;
        if(temp.compare("from")!=0){
            cout<<"Expected 'from'"<<endl;
            return 1;
        }
        tablename = getword(SQL);
        temp = getword(SQL);
        if(temp.compare("where")==0){
            s1 = getword(SQL);
            s2 = getword(SQL);
            s3 = getword(SQL);
            cond.columname = s1;
            cond.value = s3;
            if(s2.compare("=")==0){
                cond.op = EQ;
            }
            else if(s2.compare("<>")==0){
                cond.op = NE;
            }
            else if(s2.compare("<")==0){
                cond.op = LT;
            }
            else if(s2.compare(">")==0){
                cond.op = GT;
            }
            else if(s2.compare("<=")==0){
                cond.op = LE;
            }
            else if(s2.compare(">=")==0){
                cond.op = GE;
            }
            else{
                cout<<"Not support this kind of op!"<<endl;
                return 1;
            }
            s1 = getword(SQL);
        }
        else{
            cout << "Expected 'where' after table name" << endl;
            return 1;
        }
    
        cout << "delete from " << tablename << " where " << cond.columname << cond.op << cond.value << endl;
//        myAPI.deletetable(tablename,conds);
        return 0;
    }

    else if(opcode.compare("quit")==0){
//        myAPI.quit();
        cout << "quit!" << endl;
        exit(1);
    }
    else if(opcode.compare("execfile")==0){
        temp = getword(SQL);
        execfilename = temp;
        temp = getword(SQL);
        if(temp.compare(";")!=0){
            cout<<"Expected ';'"<<endl;
            return 1;
        }
        return -1; //表示是execfile;
    }
    else {
        cout<<"Not support this kind of  SQL sentence ! "<<endl;
        return 0;
    }	
}
