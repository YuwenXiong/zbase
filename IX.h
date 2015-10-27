//
// Created by Orpine on 10/25/15.
//

#ifndef ZBASE_IX_H
#define ZBASE_IX_H

#include "PF.h"
#include "RM.h"
#include <string>
using namespace std;

class IX_IndexHandle {
public:
    IX_IndexHandle();
    ~IX_IndexHandle();
    RC InsertEntry(Value &data, const RID &rid);
    RC DeleteEntry(Value &data, const RID &rid);
    RC ForcePages();
};

class IX_IndexScan {
public:
    IX_IndexScan();
    ~IX_IndexScan();
    RC OpenScan(const IX_IndexHandle &indexHandle, CmpOp op, void* value);
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
    RC CloseIndxe(IX_IndexHandle &indexHandle);
};

#endif //ZBASE_IX_H
