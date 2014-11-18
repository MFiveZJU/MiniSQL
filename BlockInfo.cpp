#include "Definition.h"

BlockInfo::BlockInfo(){
	blockNum = -1;
	dirtyBit = 0;
	next = NULL;
	file = NULL;
	charNum = 0;
	cBlock = new char[BLOCKSIZE];
	father = -1;
	iTime = 0;
	lock = 0;	
}

BlockInfo::~BlockInfo(){
	delete [] cBlock;
}

void BlockInfo::clearBlock(){
	blockNum = -1;
	dirtyBit = 0;
	next = NULL;
	file = NULL;
	charNum = 0;
	delete [] cBlock;
	cBlock = new char[BLOCKSIZE];
	father = -1;
	iTime = 0;
	lock = 0;
}