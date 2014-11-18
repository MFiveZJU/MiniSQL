//
//  main.cpp
//  miniSQL2
//
//  Created by Leon on 14/11/8.
//  Copyright (c) 2014年 ZJU. All rights reserved.
//

#include <iostream>
#include "Interpreter.h"
Interpreter myinterpreter;
using namespace std;
int main(int argc, const char * argv[]) {
    string filename;
    int i;
    while(cin){
        cout<<"miniSQL>>";
        string s=myinterpreter.read_input();
        i = myinterpreter.run(s,filename);
        if(i == -1){
            ifstream fin;
            //cout<<"filename="<<filename<<endl;
            fin.open(filename.c_str());
            int num = 0;
            while(fin){
                s = myinterpreter.read_input(fin);
                if(s=="")
                    break;
                i = myinterpreter.run(s,filename);
                if(i==-1){
                    cout<<"文件里面包含了execfile 语句"<<endl;
                    return 0;
                }
                printf("%d sentences executed \n", ++num);
            }
        }
    }
}

