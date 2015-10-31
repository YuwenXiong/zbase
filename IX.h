//
// Created by Orpine on 10/25/15.
//

#ifndef ZBASE_IX_H
#define ZBASE_IX_H

#include "PF.h"
#include "RM.h"
#include "B_Tree.h"
#include <string>
using namespace std;

const size_t B_TREE_HEADER_OFFSET=sizeof(PF_PageHeader);

class IX_IndexHandle {
    friend class IX_Manager;
public:
    IX_IndexHandle();
    ~IX_IndexHandle();
    RC InsertEntry(const Value &data, const RID &rid);
    RC DeleteEntry(const Value &data, const RID &rid);
    RC ForcePages();
private:
    B_Tree b_tree;
    PF_FileHandle pffh;
    PF_PageHandle pfph;
};

class IX_IndexScan {
public:
    IX_IndexScan();
    ~IX_IndexScan();
    RC OpenScan(const IX_IndexHandle &indexHandle, CmpOp op, Value &value);
    RC GetNextEntry(RID &rid);
    RC CloseScan();
};

class IX_Manager {
public:
    IX_Manager(PF_Manager &pfm);
    ~IX_Manager();
    RC CreateIndex(const string &fileName, int indexNo, AttrType attrType, int attrLength);
    RC DestoryIndex(const string &fileName, int indexNo);
    RC OpenIndex(const string &fileName, int indexNo, IX_IndexHandle &indexHandle);
    RC CloseIndex(IX_IndexHandle &indexHandle);
private:
    PF_Manager *pfm;
};

#endif //ZBASE_IX_H
