/*
 * IndexManager.cpp
 *
 *  Created on: 2014年10月31日
 *      Author: Chong Xiaoya
 */
#include"IndexManager.h"
#include<iostream>
#include<stdio.h>
#include<string>
#include<sstream>
#include <cstdlib>
#include<cmath>
#include<vector>
#include <algorithm>
using namespace std;

void BPlusTree::search(string dbName,IndexInfo inform,Condition con,vector<Result>& res)
{
	int type;
	if(inform.type==CHAR)
		LENGTH_OF_CHAR=inform.length;
	if(con.op==EQ){
		searchOne(dbName,inform,res);
	}
	else if(con.op==GT){
		type=1;
		searchMany(dbName,type,inform,res);
	}
	else if(con.op==GE){
		type=2;
		searchMany(dbName,type,inform,res);
	}
	else if(con.op==LT)
	{
		type=3;
		searchMany(dbName,type,inform,res);
	}
	else if(con.op==LE){
		type=4;
		searchMany(dbName,type,inform,res);
	}

}
void BPlusTree::searchOne(string dbName,IndexInfo inform,vector<Result>& res){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	BlockInfo* root=getBlock(dbName,name,0,1);
	BlockInfo* Node;
	Node=getBlock(dbName,name,root->blockNum,1);
	string info=Node->cBlock;
	int num=0;//每个节点的索引值个数
	int start;
	int length;
	if(inform.type==INT)
		length=LENGTH_OF_INT;
	else if(inform.type==FLOAT)
		length=LENGTH_OF_FLOAT;
	else if(inform.type==CHAR)
		length=LENGTH_OF_CHAR;
	/*不断地找下一个节点，直到到达叶子节点*/
	while(Node->cBlock[0]!='!'){
		/*得到当前Node的value值的个数num*/
		num=getValueNum(info.substr(1,VBIT));
		int blocknum=0;//下一个节点的块号
		info=Node->cBlock;
		int end;
		/*要查找的数据类型为int型，此时Node中的索引值为5个字节*/
		for(int i=0;i<num;i++){
			start=(length+BBIT)*i+(1+VBIT+BBIT);
			end=start+length-1;
			int compare;
			if(inform.type==INT)
				compare=compareInt(inform.value,info.substr(start,length));
			else if(inform.type==FLOAT)
				compare=compareFloat(inform.value,info.substr(start,length));
			else if(inform.type==CHAR)
				compare=compareStr(inform.value,info.substr(start,length));
			if(compare>=0){
				if(end+BBIT==Node->charNum-1){
					blocknum=getBlockNum(info.substr(end+1,BBIT));
					break;
				}
				else
					continue;//如果当前Node值小于要查找的值
			}
			else{
				blocknum=getBlockNum(info.substr(start-BBIT,BBIT));
				break;
			}
		}
		Node=getBlock(dbName,name,blocknum,1);//调用buffermanager的函数找到下一个节点
		info=Node->cBlock;
	}//end of while
	/*找到叶子节点*/
	info=Node->cBlock;
	if(info[0]=='!'){
		/*得到当前Node的value值的个数num*/
		num=getValueNum(info.substr(1,VBIT));
		for(int i=0;i<num;i++){
			start=(length+LBIT)*i+(1+VBIT+LBIT);
			int compare;
			if(inform.type==INT)
				compare=compareInt(inform.value,info.substr(start,length));
			else if(inform.type==FLOAT)
				compare=compareFloat(inform.value,info.substr(start,length));
			else if(inform.type==CHAR)
				compare=compareStr(inform.value,info.substr(start,length));
			/*找到了包含value的叶子节点，设置记录号，返回叶子块号*/
			if(compare==0){
				string Linfo=info.substr(start-LBIT,LBIT);
				Result a;
				a.blocknum=getBlockNum(Linfo.substr(0,BBIT));
				a.offsets.push_back(getOffset(Linfo.substr(BBIT,VBIT)));
				res.push_back(a);
				return;
			}
		}
	}
}
void BPlusTree::searchMany(string dbName,int type,IndexInfo inform,vector<Result>& res){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	int length;
	int start,end;
	if(inform.type==INT)
		length=LENGTH_OF_INT;
	else if(inform.type==FLOAT)
		length=LENGTH_OF_FLOAT;
	else if(inform.type==CHAR)
		length=LENGTH_OF_CHAR;
	/*调用search_leaf找到应该包含value值的叶子块*/
	int blocknum;
	blocknum=searchLeaf(dbName,inform);
	BlockInfo *leaf=getBlock(dbName,name,blocknum,1);
	int next;
	string info=leaf->cBlock;
	int compare;
	if(type==1||type==2){
		if(info[info.size()-1]=='#'){
			if(inform.type==INT)
				compare=compareInt(inform.value,info.substr(info.size()-length-1,length));
			else if(inform.type==FLOAT)
				compare=compareFloat(inform.value,info.substr(info.size()-length-1,length));
			else if(inform.type==CHAR)
				compare=compareStr(inform.value,info.substr(info.size()-length-1,length));

		}
		else{
			if(inform.type==INT)
				compare=compareInt(inform.value,info.substr(info.size()-length-BBIT,length));
			else if(inform.type==FLOAT)
				compare=compareFloat(inform.value,info.substr(info.size()-length-BBIT,length));
			else if(inform.type==CHAR)
				compare=compareStr(inform.value,info.substr(info.size()-length-BBIT,length));
		}
	}
	else if(type==3||type==4){
		if(inform.type==INT)
			compare=compareInt(inform.value,info.substr((1+VBIT+LBIT),length));
		else if(inform.type==FLOAT)
			compare=compareFloat(inform.value,info.substr((1+VBIT+LBIT),length));
		else if(inform.type==CHAR)
			compare=compareStr(inform.value,info.substr((1+VBIT+LBIT),length));
	}
	string Linfo;
	/*找出所有 >inform.value的记录*/
	if(type==1){
		/*如果是最后一个叶子节点*/
		if(info[info.size()-1]=='#'){
			/*最后一个值都<=value*/
			if(compare>=0){
				start=0;
				return;
			}
			else{
				start=leaf->blockNum;
				Linfo=findLinfo(dbName,inform,leaf->blockNum,type);
			}
		}
		/*不是最后一个叶子*/
		else{
			/*该节点最后一个值都<=value*/
			if(compare>=0){
				/*start为next兄弟节点*/
				start=findNextLeafSibling(dbName,inform,leaf->blockNum);
				/*offset为兄弟节点的第一个偏移量*/
				BlockInfo* next=getBlock(dbName,name,start,1);
				string a=next->cBlock;
				Linfo=a.substr(1+VBIT,LBIT);
			}
			else{
				start=leaf->blockNum;
				/*在本节点中，找到第一个大于value的值的偏移量*/
			    Linfo=findLinfo(dbName,inform,leaf->blockNum,type);
			}
		}
		blocknum=start;
		while((next=findNextLeafSibling(dbName,inform,blocknum))!=0)
			blocknum=next;
		end=blocknum;
	}
	/*找出所有>=inform.value的记录*/
	else if(type==2){
		/*如果是最后一个叶子节点*/
		if(info[info.size()-1]=='#'){
			/*最后一个值都小于value*/
			if(compare>0){
				start=0;
				return;
			}
			else{
				start=leaf->blockNum;
				Linfo=findLinfo(dbName,inform,leaf->blockNum,type);
			}
		}
		/*不是最后一个叶子*/
		else{

			/*最后一个值都小于value*/
			if(compare>0){
				/*start为next兄弟节点*/
				start=findNextLeafSibling(dbName,inform,leaf->blockNum);
				/*offset为兄弟节点的第一个偏移量*/
				BlockInfo* next=getBlock(dbName,name,start,1);
				string a=next->cBlock;
				Linfo=a.substr(1+VBIT,LBIT);
			}
			else{
				start=leaf->blockNum;
				/*在本节点中，找到第一个大于value的值的偏移量*/
				Linfo=findLinfo(dbName,inform,leaf->blockNum,type);
			}
		}
		while((next=findNextLeafSibling(dbName,inform,blocknum))!=0)
			blocknum=next;
		end=blocknum;
	}
	/*找出所有 <inform.value的记录*/
	else if(type==3){
		start=findLeftestChild(dbName,inform);
		/*如果是第一个叶子节点*/
		if(leaf->blockNum==start){
			/*第一个值都>=value*/
			if(compare<=0){
				start=0;
				return;
			}
			else{
				end=leaf->blockNum;
				Linfo=findLinfo(dbName,inform,leaf->blockNum,type);
			}
		}
		/*不是第一个叶子*/
		else{
			/*value<=第一个值*/
			if(compare<=0){
				/*end为pre兄弟节点*/
				end=findPrevLeafSibling(dbName,inform,leaf->blockNum);
				/*offset为兄弟节点的最后一个偏移量*/
				BlockInfo* prev=getBlock(dbName,name,end,1);
				string a=prev->cBlock;
				Linfo=a.substr(1+VBIT,LBIT);
			}
			else{
				end=leaf->blockNum;
				/*在本节点中，找到第一个大于value的值的偏移量*/
				Linfo=findLinfo(dbName,inform,leaf->blockNum,type);
			}
		}
	}
	/*找出所有 <= inform.value的记录*/
	else if(type==4){
		start=findLeftestChild(dbName,inform);
		/*如果是第一个叶子节点*/
		if(leaf->blockNum==start){
			/*第一个值都>value*/
			if(compare<0){
				start=0;
				return;
			}
			else{
				end=leaf->blockNum;
				Linfo=findLinfo(dbName,inform,leaf->blockNum,type);
			}
		}
		/*不是第一个叶子*/
		else{
			/*第一个值都>value*/
			if(compare<0){
				/*end为pre兄弟节点*/
				end=findPrevLeafSibling(dbName,inform,leaf->blockNum);
				/*offset为兄弟节点的最后一个偏移量*/
				BlockInfo* prev=getBlock(dbName,name,end,1);
				string a=prev->cBlock;
				Linfo=a.substr(1+VBIT,LBIT);
			}
			else{
				end=leaf->blockNum;
				/*在本节点中，找到第一个大于value的值的偏移量*/
				Linfo=findLinfo(dbName,inform,leaf->blockNum,type);
			}
		}
	}
	if(start!=0)
		getResult(dbName,name,start,end,inform,Linfo,type,res);

}
void BPlusTree::getResult(string dbName,string name,int start,int end,IndexInfo inform,string Linfo,int type,vector<Result>& res){
	BlockInfo*Node=getBlock(dbName,name,start,1);;
	string info=Node->cBlock;
	int length;
	if(inform.type==INT)
		length=LENGTH_OF_INT;
	else if(inform.type==FLOAT)
		length=LENGTH_OF_FLOAT;
	else if(inform.type==CHAR)
		length=LENGTH_OF_CHAR;
	int s,e;
	int num;
	for(int i=start;;i=getBlockNum(info.substr(info.size()-3,3))){
		Node=getBlock(dbName,name,i,1);
		info=Node->cBlock;
		num=getValueNum(info.substr(1,VBIT));
		/*找大于*/
		if(type==1||type==2){
			/*如果是start块*/
			if(Node->blockNum==start){
				for(int j=0;j<num;j++){
					s=(LBIT+length)*j+1+VBIT;
					e=s+LBIT+length-1;
					string newInfo=info.substr(s,LBIT);
					/*找到offset*/
					if(newInfo==Linfo){
						int m=j;
						/*从offse开始所有的加入容器中*/
						while(e<=1+VBIT+(LBIT+length)*num-1){
							newInfo=info.substr(s,LBIT);
							/*查找容器中是否存在块号*/
							int find=0;
							for(unsigned int k=0;k<res.size();k++){
								if(getBlockNum(newInfo.substr(0,BBIT))==res[k].blocknum){
									int offset=getOffset(newInfo.substr(BBIT,OBIT));
									res[k].offsets.push_back(offset);
									find=1;
									break;
								}
							}
							/*如果不存在，则再容器最后加入一个result，它有块号和一个偏移量*/
							if(find==0){
								Result a;
								a.blocknum=getBlockNum(newInfo.substr(0,BBIT));
								int offset=getOffset(newInfo.substr(BBIT,OBIT));
								a.offsets.push_back(offset);
								res.push_back(a);
							}
							m++;
							s=(LBIT+length)*m+1+VBIT;
							e=s+LBIT+length-1;
						}
					}
					else continue;
				}
			}
			/*不是start块，把块中所有块号和偏移量加入容器*/
			else{
				for(int j=0;j<num;j++){
					s=(LBIT+length)*j+1+VBIT;
					e=s+LBIT+length-1;
					string newInfo=info.substr(s,LBIT);
					/*块号是否在容器中*/
					int find=0;
					for(unsigned int k=0;k<res.size();k++){
						if(getBlockNum(newInfo.substr(0,BBIT))==res[k].blocknum){
							int offset=getOffset(newInfo.substr(BBIT,OBIT));
							res[k].offsets.push_back(offset);
							find=1;
							break;
						}
					}
					if(find==0){
						Result a;
						a.blocknum=getBlockNum(newInfo.substr(0,BBIT));
						int offset=getOffset(newInfo.substr(BBIT,OBIT));
						a.offsets.push_back(offset);
						res.push_back(a);
					}
				}
			}
			if(info[info.size()-1]=='#'){
				for(unsigned int i=0;i<res.size();i++){
					sort(res[i].offsets.begin(),res[i].offsets.end());
				}
				return;
			}
		}
		/*找小于*/
		else{
			/*如果是最后一块,从头到offset加入到容器*/
			if(Node->blockNum==end){
				for(int j=0;j<num;j++){
					s=(LBIT+length)*j+1+VBIT;
					e=s+LBIT+length-1;
					string newInfo=info.substr(s,LBIT);
					/*加入容器中*/
					if(newInfo!=Linfo){
						/*查找容器中是否存在块号*/
						int find=0;
						for(unsigned int k=0;k<res.size();k++){
							if(getBlockNum(newInfo.substr(0,BBIT))==res[k].blocknum){
								int offset=getOffset(newInfo.substr(BBIT,OBIT));
								res[k].offsets.push_back(offset);
								find=1;
								break;
							}
						}
						/*如果不存在，则再容器最后加入一个result，它有块号和一个偏移量*/
						if(find==0){
							Result a;
							a.blocknum=getBlockNum(newInfo.substr(0,BBIT));
							int offset=getOffset(newInfo.substr(BBIT,OBIT));
							a.offsets.push_back(offset);
							res.push_back(a);
						}
					}
					/*Linfo加入容器，之后的舍弃*/
					else{
						int find=0;
						for(unsigned int k=0;k<res.size();k++){
							if(getBlockNum(newInfo.substr(0,BBIT))==res[k].blocknum){
								int offset=getOffset(newInfo.substr(BBIT,OBIT));
								res[k].offsets.push_back(offset);
								find=1;
								break;
							}
						}
						/*如果不存在，则再容器最后加入一个result，它有块号和一个偏移量*/
						if(find==0){
							Result a;
							a.blocknum=getBlockNum(newInfo.substr(0,BBIT));
							int offset=getOffset(newInfo.substr(BBIT,OBIT));
							a.offsets.push_back(offset);
							res.push_back(a);
						}
						for(unsigned int i=0;i<res.size();i++){
							sort(res[i].offsets.begin(),res[i].offsets.end());
						}
						return;
					}
				}
			}
			/*如果不是最后一块，全都加入到容器中*/
			else{
				for(int j=0;j<num;j++){
					s=(LBIT+length)*j+1+VBIT;
					e=s+LBIT+length-1;
					string newInfo=info.substr(s,LBIT);
					/*块号是否在容器中*/
					int find=0;
					for(unsigned int k=0;k<res.size();k++){
						if(getBlockNum(newInfo.substr(0,BBIT))==res[k].blocknum){
							int offset=getOffset(newInfo.substr(BBIT,OBIT));
							res[k].offsets.push_back(offset);
							find=1;
							break;
						}
					}
					if(find==0){
						Result a;
						a.blocknum=getBlockNum(newInfo.substr(0,BBIT));
						int offset=getOffset(newInfo.substr(BBIT,OBIT));
						a.offsets.push_back(offset);
						res.push_back(a);
					}
				}
			}
		}//end of type34

	}//end of for

}
int BPlusTree::searchLeaf(string dbName, IndexInfo  inform){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	BlockInfo* root=getBlock(dbName,name,0,1);
	BlockInfo* Node;
	Node=getBlock(dbName,name,root->blockNum,1);
	string info=Node->cBlock;
	int num=0;//每个节点的索引值个数
	int start;
	int length;
	if(inform.type==INT)
		length=LENGTH_OF_INT;
	else if(inform.type==FLOAT)
		length=LENGTH_OF_FLOAT;
	else if(inform.type==CHAR)
		length=LENGTH_OF_CHAR;
	/*不断地找下一个节点，直到到达叶子节点*/
	while(Node->cBlock[0]!='!'){
		/*得到当前Node的value值的个数num*/
		num=getValueNum(info.substr(1,VBIT));
		int blocknum=0;//下一个节点的块号
		info=Node->cBlock;
		int end;
		/*要查找的数据类型为int型，此时Node中的索引值为5个字节*/
		for(int i=0;i<num;i++){
			start=(length+BBIT)*i+(1+VBIT+BBIT);
			end=start+length-1;
			int compare;
			if(inform.type==INT)
				compare=compareInt(inform.value,info.substr(start,length));
			else if(inform.type==FLOAT)
				compare=compareFloat(inform.value,info.substr(start,length));
			else if(inform.type==CHAR)
				compare=compareStr(inform.value,info.substr(start,length));
			if(compare>=0){
				if(end+BBIT==Node->charNum-1){
					blocknum=getBlockNum(info.substr(end+1,BBIT));
					break;
				}
				else
					continue;//如果当前Node值小于要查找的值
			}
			else{
				blocknum=getBlockNum(info.substr(start-BBIT,BBIT));
				break;
			}
		}
		Node=getBlock(dbName,name,blocknum,1);//调用buffermanager的函数找到下一个节点
		info=Node->cBlock;
		}//end of while
		/*找到叶子节点*/
		info=Node->cBlock;
		if(info[0]=='!')
			return Node->blockNum;
		return -1;
}
void BPlusTree::insertOne(string dbName,IndexInfo  inform){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	BlockInfo* root=getBlock(dbName,name,0,1);
	int length;
	if(inform.type==INT){
		length=LENGTH_OF_INT;
		setN(340);
	}
	else if(inform.type==FLOAT){
		length=LENGTH_OF_FLOAT;
		setN(240);
	}
	else if(inform.type==CHAR){
		LENGTH_OF_CHAR=inform.length;
		length=LENGTH_OF_CHAR;
		setN(4088/(length+LBIT));
	}
	int Node;
	/*索引文件为空，建立一个根节点*/
	if(root->charNum==0)
	{
		/*将value存入根节点*/
		string temp="!0001";
		temp+=intToStr(inform.blockNum,BBIT);
		temp+=intToStr(inform.offset,OBIT);
		temp+=toLength(inform);
		temp+="#";
		write(root,temp);
		return;
	}
	else{
		Node=searchLeaf(dbName,inform);//find leaf Node L that should contain value K
		BlockInfo* node=getBlock(dbName,name,Node,1);
		string info=node->cBlock;
		if(getValueNum(info.substr(1,VBIT))<N-1)
			insertInLeaf(dbName,inform,Node);
		else{
			/*create node L'(=L1 here)*/
			BlockInfo* L1=getEmptyBlock(dbName,name);
			int l1=L1->blockNum;
			/*Copy L (=node here) to T*/
			BlockInfo* T=getEmptyBlock(dbName,name);
			int t=T->blockNum;
			T->cBlock=(char*)info.c_str();
			/*store node.pn*/
			string pn;
			if(info[node->charNum-1]=='#')
				pn=info.substr(node->charNum-1,1);
			else
				pn=info.substr(node->charNum-BBIT,BBIT);
			insertInLeaf(dbName,inform,t);
			/*erase all values and pointers from node*/
			/*copy 1 to n/2 from T to L(node)*/
			string tempT=T->cBlock;
			int halfn=ceil((getValueNum(info.substr(1,VBIT))+1)/2.0);
			string tempL;
			tempL="!"+intToStr(halfn,VBIT);
			tempL+=tempT.substr(1+VBIT,(LBIT+length)*halfn);
			/*set L(node).pn=L1*/
			tempL+=intToStr(l1,BBIT);
			write(node,tempL);
			int n=getValueNum(tempT.substr(1,VBIT));
			/*update L1节点value的个数*/
			string tempL1;
			tempL1="!"+intToStr(n-halfn,VBIT);
			/*copy n/2 +1 to n from T to L1*/
			tempL1+=tempT.substr(1+VBIT+(length+LBIT)*halfn,(n-halfn)*(length+LBIT));
			/*set L1(node).pn=L.pn*/
			tempL1+=pn;
			write(L1,tempL1);
			/*let K1 be the smallest value in L'(=L1 here)*/
			string K1=tempL1.substr(1+VBIT+LBIT,length);
			insertInParent(dbName,inform,Node,K1,l1);
			//delete T
			deleteBlock(dbName,name,T);
		}
	}
}
void BPlusTree::insertInLeaf(string dbName,IndexInfo &inform,int Node){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	BlockInfo* leaf;
	leaf=getBlock(dbName,name,Node,1);//调用buffermanager的函数找到叶子节点
	string info;
	int num;
	info=leaf->cBlock;
	num=getValueNum(info.substr(1,VBIT));
	int start;
	int length;
	if(inform.type==INT)
		length=LENGTH_OF_INT;
	else if(inform.type==FLOAT)
		length=LENGTH_OF_FLOAT;
	else if(inform.type==CHAR)
		length=LENGTH_OF_CHAR;
	int compare;
	string temp=leaf->cBlock;
	string bnum=intToStr(inform.blockNum,BBIT);
	string offset=intToStr(inform.offset,OBIT);
	string insert=bnum+offset+toLength(inform);
	for(int i=0;i<num;i++){
		start=(length+LBIT)*i+(1+VBIT+LBIT);
		if(inform.type==INT)
			compare=compareInt(inform.value,info.substr(start,length));
		else if(inform.type==FLOAT)
			compare=compareFloat(inform.value,info.substr(start,length));
		else if(inform.type==CHAR)
			compare=compareStr(inform.value,info.substr(start,length));
		/*将叶子节点value值的个数加1*/
		temp.replace(1,VBIT,intToStr(getValueNum(info.substr(1,VBIT))+1,VBIT));
		/*找到了最小的大于value的值*/
		if(compare<0)
		{
			/*将value值存入节点*/
			temp.insert(start-LBIT,insert);
			write(leaf,temp);
			return;
		}
	}
	/*插到最后*/
	if(temp[temp.size()-1]=='#')
		temp.insert(temp.size()-1,insert);
	else
		temp.insert(temp.size()-BBIT,insert);
	write(leaf,temp);
	return;
}
void BPlusTree::insertInParent(string dbName,IndexInfo inform,int Node,string K1,int N1){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	BlockInfo* root=getBlock(dbName,name,0,1);
	int length;
	if(inform.type==INT)
			length=LENGTH_OF_INT;
	else if(inform.type==FLOAT)
			length=LENGTH_OF_FLOAT;
	else if(inform.type==CHAR)
			length=LENGTH_OF_CHAR;
	/*if N is the root of the tree*/
	if(root->blockNum==Node){
		/*create new node R containing N,K1,N1*/
		BlockInfo* R=getEmptyBlock(dbName,name);
		int r=R->blockNum;
		BlockInfo* node=getBlock(dbName,name,Node,1);
		string info="?0001"+intToStr(r,BBIT)+K1+intToStr(N1,BBIT);
		R->blockNum=Node;
		node->blockNum=r;
		write(R,info);
		writeRootBlock(dbName,name,R);
		return;
	}
	else{
		/*let P=parent(N)*/
		BlockInfo* N0=getBlock(dbName,name,Node,1);
		int p=findFather(dbName,inform,N0->blockNum);
		BlockInfo* P=getBlock(dbName,name,p,1);
		string info=P->cBlock;
		/*P has less than n-1 values*/
		if(getValueNum(info.substr(1,4))<N-1){
			/*insert (K1,N1)in P just after N*/
			string temp=P->cBlock;
			int num=getValueNum(temp.substr(1,VBIT));
			int start,end;
			temp.replace(1,VBIT,intToStr(num+1,VBIT));
			for(int i=0;i<=num;i++){
				start=(length+BBIT)*i+1+VBIT;
				end=start+BBIT-1;
				/*找到了匹配Node的块号*/
				if(temp.substr(start,BBIT)==intToStr(Node,BBIT)){
					string insert=K1+intToStr(N1,BBIT);
					temp.insert(end+1,insert);
					write(P,temp);
					return;
				}
			}
		}
		/*split P*/
		else
		{
			/*create T that can hold P,(K1,N1)*/
			BlockInfo* T=getEmptyBlock(dbName,name);
			/*insert (K1,N1)in T just after N*/
			T->cBlock=P->cBlock;
			string tempT=T->cBlock;
			int num=getValueNum(tempT.substr(1,VBIT));
			int start,end;
			/*value个数加1*/
			string a=P->cBlock;
			int n=getValueNum(a.substr(1,VBIT))+1;
			tempT.replace(1,VBIT,intToStr(n,VBIT));
			for(int i=0;i<num;i++){
				start=(length+BBIT)*i+1+VBIT;
				end=start+BBIT-1;
				string insert=K1+intToStr(N1,BBIT);
				/*找到了匹配的块号*/
				if(tempT.substr(start,BBIT)==intToStr(Node,BBIT)){
					tempT.insert(end+1,insert);
					break;
				}
			}
			/*erase all form P*/
			P->cBlock=NULL;
			/*create node P1*/
			BlockInfo* P1=getEmptyBlock(dbName,name);
			int p1=P1->blockNum;
			/*copy 1 to n/2 from T to P*/
			int halfn=ceil((n-1)/2.0);
			string tempP="?"+intToStr(halfn,4);
			tempP+=tempT.substr(5,(length+3)*halfn+3);
			write(P,tempP);
			/*let K11=T.K n/2*/
			string K11=tempT.substr(5+(length+3)*halfn+3,length);
			/*copy n/2 +1 to n from T to P1*/
			string tempP1="?"+intToStr(n-halfn-1,4);
			tempP1+=tempT.substr(5+(length+3)*(halfn+1),(length+3)*(n-halfn-1)+3);
			write(P1,tempP1);
			insertInParent(dbName,inform,p,K11,p1);
			deleteBlock(dbName,name,T);
			return;
		}
	}
}
void BPlusTree::deleteOne(string database,IndexInfo  inform){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	int L=searchLeaf(database,inform);
	deleteEntry(database,inform,L,inform.value,L);
}
void BPlusTree::deleteEntry(string dbName,IndexInfo inform,int n,string K,int nod){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	int length;
	if(inform.type==INT){
		length=LENGTH_OF_INT;
		setN(340);
	}
	else if(inform.type==FLOAT){
		length=LENGTH_OF_FLOAT;
		setN(240);
	}
	else if(inform.type==CHAR){
		LENGTH_OF_CHAR=inform.length;
		length=LENGTH_OF_CHAR;
		setN(4088/(length+LBIT));
	}
	BlockInfo *Node=getBlock(dbName,name,n,1);
	string info=Node->cBlock;
	string originN=info;
	int num=getValueNum(info.substr(1,VBIT));
	string tempN=Node->cBlock;
	while(K[0]=='0'){
		K=K.substr(1,K.size()-1);
	}
	/*delete K,P from N*/
	tempN.replace(1,VBIT,intToStr(num-1,VBIT));
	int start;
	for(int i =0;i<num;i++){
		int compare;
		if(tempN[0]=='?')
			start=(length+BBIT)*i+1+VBIT+BBIT;
		else if(tempN[0]=='!')
			start=(length+LBIT)*i+1+VBIT+LBIT;
		if(inform.type==INT)
			compare=compareInt(K,info.substr(start,length));
		else if(inform.type==FLOAT)
			compare=compareFloat(K,info.substr(start,length));
		else if(inform.type==CHAR)
			compare=compareStr(K,info.substr(start,length));
		if(compare==0){
			if(tempN[0]=='?'){
				if(tempN.substr(start-3,3)==intToStr(nod,3)){
					tempN.replace(start-3,length+3,"");
					break;
				}
				else if(tempN.substr(start+length,3)==intToStr(nod,3)){
					tempN.replace(start,length+3,"");
					break;
				}
			}
			else if(tempN[0]=='!'){
				tempN.replace(start-LBIT,length+LBIT,"");
				break;
			}
		}

	}
	int father=findFather(dbName,inform, Node->blockNum);
	write(Node,tempN);
	/*if N is root and has one value*/
	if(Node->blockNum==0&&originN.substr(1,4)=="0001"){
		/*make the child of N the root*/
		int child=getBlockNum(tempN.substr(5,3));
		BlockInfo* Child=getBlock(dbName,name,child,1);
		/*交换块号*/
		Node->blockNum=Child->blockNum;
		Child->blockNum=0;
		writeRootBlock(dbName,name,Child);
		/*delete N*/
		deleteBlock(dbName,name,Node);
	}
	else{
		string info=Node->cBlock;
		tempN=Node->cBlock;
		/*如果是叶子*/
		if(info[0]=='!'){
			/*if has too few values*/
			if(getValueNum(info.substr(1,4))<leaf_least){
				/*let N1 be the previous child of parent(N)*/
				int n1=findPrevLeafSibling(dbName,inform, n);
				BlockInfo* N1=getBlock(dbName,name,n1,1);
				int father1=findFather(dbName,inform,N1->blockNum);
				if(father==father1){
					/*let K1 be the value between N1 and N in parent(N)*/
					string K1=originN.substr(1+VBIT+LBIT,length);
					string tempN1=N1->cBlock;
					/*if can fit in one block*/
					if(getValueNum(info.substr(1,4))+getValueNum(tempN1.substr(1,4))<=N-1){
						tempN1.replace(1,4,intToStr(getValueNum(info.substr(1,4))+getValueNum(tempN1.substr(1,4)),4));
						/*append all in N into N1,let N1.pn=N.pn*/
						tempN1.replace(tempN1.size()-3,3,"");
						string n0=info.substr(5,strlen(Node->cBlock)-5);
						tempN1+=n0;
						write(N1,tempN1);
						deleteEntry(dbName,inform,father,K1,n);
						deleteBlock(dbName,name,Node);
					}
					/*redistribution*/
					else{
						/*find the last value and its offset in N1*/
						int numN1=getValueNum(tempN1.substr(1,4));
						string last=tempN1.substr(5+(length+LBIT)*(numN1-1),length);
						/*remove the last from N1*/
						tempN1.replace(1,4,intToStr(getValueNum(tempN1.substr(1,4))-1,4));
						tempN1.replace(5+(length+LBIT)*(numN1-1),length,"");
						write(N1,tempN1);
						/*insert the last in N1 as the first in N*/
						tempN.replace(1,4,intToStr(getValueNum(tempN.substr(1,4))+1,4));
						tempN.insert(5,last);
						write(Node,tempN);
						/*replace K1 in parent N by N1.Km*/
						string Km=last.substr(LBIT,length);
						BlockInfo* Father=getBlock(dbName,name,father,1);
						string tempF=Father->cBlock;
						num=getValueNum(tempF.substr(1,4));
						for(int i=0;i<num;i++){
							start=(3+length)*i+8;
							/*找到后替换*/
							if(tempF.substr(start,length)==K1){
								tempF.replace(start,K1.size(),Km);
								write(Father,tempF);
								break;
							}
						}
					}
				}
				/*or next child of parent(N)*/
				else{
					n1=findNextLeafSibling( dbName,inform, n);
					BlockInfo* N1=getBlock(dbName,name,n1,1);
					string tempN1=N1->cBlock;
					string K1=tempN1.substr(1+VBIT+LBIT,length);
					/*if can fit in one block*/
					if(getValueNum(tempN.substr(1,4))+getValueNum(tempN1.substr(1,4))<=N-1){
						tempN1.replace(1,4,intToStr(getValueNum(info.substr(1,4))+getValueNum(tempN1.substr(1,4)),4));
						tempN.replace(tempN.size()-3,3,"");
						/*append all in N into N1*/
						string part1=tempN.substr(5,Node->charNum-5);
						tempN1.insert(5,part1);
						write(N1,tempN1);
						deleteEntry(dbName,inform,father,K1,n);
						/*delete N*/
						deleteBlock(dbName,name,Node);
					}
					/*redistribution*/
					else{
						/*find the first in N1*/
						string first=tempN1.substr(1+VBIT+LBIT,length);
						/*remove the first from N1*/
						tempN1.replace(1,4,intToStr(getValueNum(tempN1.substr(1,4))-1,4));
						tempN1.replace(5,LBIT+length,"");
						write(N1,tempN1);
						/*insert the first in N1 as the last in N*/
						tempN.replace(1,4,intToStr(getValueNum(tempN.substr(1,4))+1,4));
						tempN.insert(tempN.size()-3,first);
						write(Node,tempN);
						/*replace K1 in parent N by N1's first value K1*/
						string N1K1=tempN1.substr(1+VBIT+LBIT,length);
						BlockInfo* Father=getBlock(dbName,name,father,1);
						string tempF=Father->cBlock;
						num=getValueNum(tempF.substr(1,4));
						for(int i=0;i<num;i++){
							start=(3+length)*i+8;
							/*找到后替换*/
							if(tempF.substr(start,length)==K1){
								tempF.replace(start,length,N1K1);
								write(Father,tempF);
								break;
							}
						}
					}
				}
			}
		}
		/*如果不是叶子*/
		else if(info[0]=='?'){
			/*if has too few values*/
			if(getValueNum(info.substr(1,4))<nonleaf_least){
				/*找到N的相邻节点*/
				BlockInfo* Father=getBlock(dbName,name,father,1);
				string tempF=Father->cBlock;
				int num=getValueNum(tempF.substr(1,4));
				int start,end;
				int n1;
				int pre=1;
				string K1;
				for(int i=0;i<=num;i++){
					start=(3+length)*i+5;
					end=start+2;
					if(tempF.substr(start,3)==intToStr(Node->blockNum,3)){
						/*N1是N的prev节点*/
						if(end!=7){
							n1=getBlockNum(tempF.substr(start-length-3,3));
							K1=tempF.substr(start-length,length);
							pre=1;
						}
						/*N1是N的next节点*/
						else{
						n1=getBlockNum(tempF.substr(end+length+1,3));
							K1=tempF.substr(end+1,length);
							pre=0;
						}
						break;
					}
				}
				BlockInfo* N1=getBlock(dbName,name,n1,1);
				string tempN1=N1->cBlock;
				/*let N1 be the previous child of parent(N)*/
				if(pre==1){
					/*if can fit in one block*/
					if(getValueNum(tempN.substr(1,4))+getValueNum(tempN1.substr(1,4))<N-1){
						/*append K1 and all in N to N1 */
						tempN1.replace(1,4,intToStr(getValueNum(tempN1.substr(1,4))+getValueNum(tempN.substr(1,4))+1,4));
						string tail=tempN.substr(5,Node->charNum-5);
						string head=tempN1.substr(0,5);
						tempN1=head+K1+tail;
						write(N1,tempN1);
						deleteEntry(dbName,inform,father,K1,n);
						deleteBlock(dbName,name,Node);
					}
					/*redistribution*/
					else{
						/*pm is the last pointer in N1*/
						string N1Pm=tempN1.substr(N1->charNum-3,3);
						string N1K=tempN1.substr(N1->charNum-3-length,length);
						/*remove N1.Km-1,N1.pm from N1*/
						tempN1.replace(1,4,intToStr(getValueNum(tempN1.substr(1,4))-1,4));
						tempN1=tempN1.substr(0,N1->charNum-3-length);
						write(N1,tempN1);
						/*insert N1.pm,K1 as the first in N*/
						tempN.replace(1,4,intToStr(getValueNum(tempN.substr(1,4))+1,4));
						string head=tempN.substr(0,5);
						string tail=tempN.substr(5,Node->charNum-5);
						tempN=head+N1Pm+K1+tail;
						write(Node,tempN);
						/*replace K1 in parent(N) by N1.Km-1*/
						tempF.replace(start-length,length,N1K);
						write(Father,tempF);
					}
				}
				/*or next child of parent(N)*/
				else if(pre==0){
					/*if can fit in one block*/
					if(getValueNum(tempN.substr(1,4))+getValueNum(tempN1.substr(1,4))<N-1){
						/*append K1 and all in N to N1 */
						tempN1.replace(1,4,intToStr(getValueNum(tempN1.substr(1,4))+getValueNum(tempN.substr(1,4))+1,4));
						string head=tempN1.substr(0,5);
						string tail=tempN1.substr(5,N1->charNum-5);
						string part=tempN.substr(5,tempN.size()-5);
						tempN1=head+part+K1+tail;
						write(N1,tempN1);
						deleteEntry(dbName,inform,father,K1,n);
						deleteBlock(dbName,name,Node);
					}
					/*redistribution*/
					else{
						/*P1 is the first pointer in N1*/
						string N1P1=tempN1.substr(5,3);
						string N1K1=tempN1.substr(8,length);
						/*remove N1.K1,N1.P1 from N1*/
						tempN1.substr(1,4)=intToStr(getValueNum(tempN1.substr(1,4))-1,4);
						string headN1=tempN1.substr(0,5);
						string tailN1=tempN1.substr(8+length,N1->charNum-8-length);
						tempN1=headN1+tailN1;
						write(N1,tempN1);
						/*insert N1.P1,K1 as the last in N*/
						tempN.replace(1,4,intToStr(getValueNum(tempN.substr(1,4))+1,4));
						tempN=tempN+K1+N1P1;
						write(Node,tempN);
						tempF.replace(end+1,length,N1K1);
						write(Father,tempF);
					}
				}
			}
		}
	}
}
int BPlusTree::getBlockNum(string sblocknum){
	int blocknum=0;
	for(int i=0;i<BBIT;i++)
		blocknum=blocknum+((int)sblocknum[i]-48)*pow(10,2-i);
	return blocknum;
}
int BPlusTree::getOffset(string stableoffset){
	int tableoffset=0;
	for(int i=0;i<OBIT;i++)
		tableoffset=tableoffset+((int)stableoffset[i]-48)*pow(10,3-i);
	return tableoffset;
}
int BPlusTree::getValueNum(string snum){//将节点value值的个数转化成int
	int num=0;
	for(int i=0;i<VBIT;i++)
		num=num+((int)snum[i]-48)*pow(10,3-i);
	return num;

}
int BPlusTree::compareStr(string value,string str2)
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
int BPlusTree::compareInt(string int1,string int2)
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
int BPlusTree::compareFloat(string float1,string float2)
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
string BPlusTree::findLinfo(string dbName,IndexInfo inform,int nodenum,int type){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	string offset;
	BlockInfo* leaf=getBlock(dbName,name,nodenum,1);//调用buffermanager的函数找到叶子节点
	string info=leaf->cBlock;
	int num=getValueNum(info.substr(1,VBIT));
	int start;
	for(int i=0;i<num;i++){
		int length;
		int compare;
		if(inform.type==INT){
			length=LENGTH_OF_INT;
			compare=compareInt(inform.value,info.substr((length+LBIT)*i+(1+VBIT+LBIT),length));
		}
		else if(inform.type==FLOAT){
			length=LENGTH_OF_FLOAT;
			compare=compareFloat(inform.value,info.substr((length+LBIT)*i+(1+VBIT+LBIT),length));
		}
		else if(inform.type==CHAR){
			length=LENGTH_OF_CHAR;
			compare=compareStr(inform.value,info.substr((length+LBIT)*i+(1+VBIT+LBIT),length));
		}
		start=(length+LBIT)*i+(1+VBIT+LBIT);
		if(type==1){
			if(compare>=0) continue;
			else{
				offset=info.substr(start-LBIT,LBIT);
				break;
			}
		}
		else if(type==2){
			if(compare>0) continue;
			else{
				offset=info.substr(start-LBIT,LBIT);
				break;
			}
		}
		else if(type==3){
			if(compare>0)
				if(i==num-1){
					offset=info.substr(start-LBIT,LBIT);
					break;
				}
				else
					continue;
			else{
				offset=info.substr(start-LBIT-length-LBIT,LBIT);
				break;
			}
		}
		else if(type==4){
			if(compare>=0)
				if(i==num-1){
					offset=info.substr(start-LBIT,LBIT);
					break;
				}
				else
					continue;
			else{
				offset=info.substr(start-LBIT-length-LBIT,LBIT);
				break;
			}
		}
	}
	return offset;
}
string BPlusTree::intToStr(int value,int length){//int 型数值转化成length字节的string类型
	stringstream s1;
	s1<<value;
	string svalue;
	s1>>svalue;
	string res;
	if(length==5){
	if(value<10)
		res="0000"+svalue;
	else if(value<100)
		res="000"+svalue;
	else if(value<1000)
		res="00"+svalue;
	else if(value<10000)
		res="0"+svalue;
	else
		res=svalue;
	}
	else if(length==4){
		if(value<10)
			res="000"+svalue;
		else if(value<100)
			res="00"+svalue;
		else if(value<1000)
			res="0"+svalue;
		else
			res=svalue;
	}
	else if(length==3){
		if(value<10)
			res="00"+svalue;
		else if(value<100)
			res="0"+svalue;
		else
			res=svalue;
		}
	return res;
}
string BPlusTree::toLength(IndexInfo inform)/*将inform中的value位拓展到节点中相应的LENGTH的长度*/
{
	string value=inform.value;
	int l=value.size();
	if(inform.type==INT)
	{
		if(l<LENGTH_OF_INT)
			for(int i=0;i<LENGTH_OF_INT-l;i++)
				value="0"+value;
	}
	else if(inform.type==FLOAT)
	{
		if(l<LENGTH_OF_FLOAT)
			for(int i=0;i<LENGTH_OF_FLOAT-l;i++)
				value="0"+value;
	}
	else if(inform.type==CHAR){
		if(l<LENGTH_OF_CHAR)
			for(int i=0;i<LENGTH_OF_CHAR-l;i++)
				value="0"+value;

	}


	return value;
}
int BPlusTree::findPrevLeafSibling(string dbName,IndexInfo inform, int nodenum){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	int left=findLeftestChild(dbName,inform);
	/*没有前一个叶子节点*/
	if(left==nodenum)
		return -1;
	BlockInfo* Left=getBlock(dbName,name,left,1);
	/*读到异常块*/
	if(Left==NULL)
		return -1;
	string info=Left->cBlock;
	while(info.substr(Left->charNum-3,3)!=intToStr(nodenum,3)){
		left=getBlockNum(info.substr(Left->charNum-3,3));
		Left=getBlock(dbName,name,left,1);
		if(Left==NULL)
			return -1;
		info=Left->cBlock;
	}
	return left;
}
int BPlusTree::findNextLeafSibling(string dbName, IndexInfo inform,int nodenum){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	BlockInfo* node=getBlock(dbName,name,nodenum,1);
	/*读到异常块*/
	if(node==NULL)
		return -1;
	string info=node->cBlock;
	/*没有下一个节点*/
	if(info[node->charNum-1]=='#')
		return 0;
	int start=node->charNum-3;
	string next=info.substr(start,3);
	int Next=getBlockNum(next);
	return Next;
}
int BPlusTree::findLeftestChild (string dbName,IndexInfo inform){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	BlockInfo* root=getBlock(dbName,name,0,1);
	string left;
	BlockInfo* node=root;
	string info=node->cBlock;
	while(info[0]=='?')
	{
		left=info.substr(5,3);
		node=getBlock(dbName,name,getBlockNum(left),1);
		info=node->cBlock;
	}
	return getBlockNum(left);
}

void BPlusTree::write(BlockInfo* b,string s){
	b->cBlock=new char[s.size()+1];
	strcpy(b->cBlock,s.c_str());
	b->charNum=strlen(b->cBlock);
	b->dirtyBit=1;
}

BPlusTree::BPlusTree():LENGTH_OF_INT(5),LENGTH_OF_FLOAT(10),LENGTH_OF_CHAR(10),N(3),leaf_least(ceil((N-1)/2.0)),nonleaf_least(ceil(N/2.0)-1),VBIT(4),BBIT(3),OBIT(4),LBIT(7){

}
void BPlusTree::setN(int n){
	N=n;
	leaf_least=ceil((N-1)/2.0);
	nonleaf_least=ceil(N/2.0)-1;
}

int BPlusTree::findFather(string dbName,IndexInfo inform,int num){
	const string name=inform.tableName+"_"+inform.attrName+"_"+inform.name;
	int length;
	if(inform.type==INT)
			length=LENGTH_OF_INT;
	else if(inform.type==FLOAT)
			length=LENGTH_OF_FLOAT;
	else if(inform.type==CHAR)
			length=LENGTH_OF_CHAR;
	int father;
	BlockInfo* node=getBlock(dbName,name,num,1);
	string info=node->cBlock;
	string value;
	if(info[0]=='?')
		value=info.substr(8,length);
	else if(info[0]=='!')
		value=info.substr(1+VBIT+LBIT,length);
	while(value[0]=='0')
		value=value.substr(1,value.size()-1);
	BlockInfo* root=getBlock(dbName,name,0,1);
	BlockInfo*Node=root;
	if(root->blockNum==num)
		return -1;
	int start;
	info=Node->cBlock;
	while(Node->blockNum!=num){
		/*得到当前Node的value值的个数num*/
		int vnum=getValueNum(info.substr(1,4));
		int blocknum=0;//下一个节点的块号
		int end;
		/*要查找的数据类型为int型，此时Node中的索引值为5个字节*/
		for(int i=0;i<vnum;i++){
			start=(length+3)*i+8;
			end=start+length-1;
			int compare;
			if(inform.type==INT)
				compare=compareInt(value,info.substr(start,length));
			else if(inform.type==FLOAT)
				compare=compareFloat(value,info.substr(start,length));
			else if(inform.type==CHAR)
				compare=compareStr(value,info.substr(start,length));
			if(compare>=0){
				if(end+3==Node->charNum-1){
					blocknum=getBlockNum(info.substr(end+1,3));
					break;
				}
				else
					continue;//如果当前Node值小于要查找的值
			}
			else{
				blocknum=getBlockNum(info.substr(start-3,3));
				break;
			}
		}
		father=Node->blockNum;
		Node=getBlock(dbName,name,blocknum,1);//调用buffermanager的函数找到下一个节点
		info=Node->cBlock;
	}//end of while
	return father;
}




