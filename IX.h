//
// Created by Orpine on 10/25/15.
//

#ifndef ZBASE_IX_H
#define ZBASE_IX_H

#include "PF.h"
#include "RM.h"
#include "io.h"
#include "B_Tree.h"
#include <string>
using namespace std;

const size_t B_TREE_HEADER_OFFSET=sizeof(PF_PageHeader);

class IX_IndexHandle {
    friend class IX_Manager;
    friend class IX_IndexScan;
public:

    RC InsertEntry(Value &data, const RID &rid);
    RC DeleteEntry(Value &data, const RID &rid);
private:
    B_Tree b_tree;

};

class IX_IndexScan {
public:

    RC OpenScan(IX_IndexHandle &indexHandle, CmpOp op, Value& value);
    RC GetNextEntry(RID &rid);
    RC CloseScan();

private:
    B_Entry* cmpEntry;
    CmpOp op;
    IX_IndexHandle* ixh;
    B_Node* curNode;
    int curPos;
};

class IX_Manager {
public:
    IX_Manager(PF_Manager &pfm);
    RC CreateIndex(const string &fileName, int indexNo, AttrType attrType, int attrLength);
    RC DestoryIndex(const string &fileName, int indexNo);
    RC OpenIndex(const string &fileName, int indexNo, IX_IndexHandle &indexHandle);
    RC CloseIndex(IX_IndexHandle &indexHandle);
private:
    PF_Manager *pfm;
};
const RC IX_NO_CAPACITY=IX_RC+1;
const RC IX_SET_FROM_DIFFERENT_LEVEL=IX_RC+2;
const RC IX_DELETE_NOT_FOUND=IX_RC+3;
const RC IX_NO_SCAN_RESULT=IX_RC+4;
const RC IX_NO_MORE_SCAN_RESULT=IX_RC+5;
const RC IX_INDEX_ALREADY_OPEN=IX_RC+6;
const RC IX_INDEX_NOT_FOUND=IX_RC+7;
#endif //ZBASE_IX_H
