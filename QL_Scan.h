//
// Created by Orpine on 10/31/15.
//

#ifndef ZBASE_QL_SCAN_H
#define ZBASE_QL_SCAN_H

#include "zbase.h"
#include "RM.h"
#include "SM.h"
#include "QL.h"

class QL_ScanHandle {
public:
    virtual RC Open() = 0;
    virtual RC Close() = 0;
    virtual RC GetNext(RID &rid) {
        return QL_EOF;
    }
    virtual RC GetNext(char* data) = 0;
    size_t GetTupleLength() {
        return tupleLength;
    };
protected:
    size_t tupleLength;
//    virtual void GetAttrCount(int &attrCount) = 0;
//    virtual void GetAttrInfo(vector<AttrCatRecord> &attrs) = 0;
};

class QL_IndexScanHandle: public QL_ScanHandle {
public:
    QL_IndexScanHandle(SM_Manager* smm, IX_Manager* ixm, RM_Manager* rmm, const string &relationName, const string &attrName, CmpOp op, const Value &v);
    ~QL_IndexScanHandle();

    RC Open();
    RC Close();
    RC GetNext(RID &rid);
    RC GetNext(char* data);
//    size_t GetTupleLength();
//    void GetAttrCount(int &attrCount);
//    void GetAttrInfo(vector<AttrCatRecord> &attrs);

private:

    SM_Manager *smManager;
    IX_Manager *ixManager;
    RM_Manager *rmManager;
    IX_IndexHandle ixIH;
    IX_IndexScan ixIS;
    RM_FileHandle rmFH;
    string relationName, attrName;
    CmpOp op;
    Value value;
//    size_t tupleLength;
    int attrCount;
    vector<AttrCatRecord> attrs;
};


class QL_FileScanHandle: public QL_ScanHandle {
public:
    QL_FileScanHandle(SM_Manager* smm, RM_Manager* rmm, const string &relationName);
    QL_FileScanHandle(SM_Manager* smm, RM_Manager* rmm, const string &relationName, const string &attrName, CmpOp op, const Value &v);
    RC Open();
    RC Close();
    RC GetNext(RID &rid);
    RC GetNext(char* data);
//    size_t GetTupleLength();
//    void GetAttrCount(int &attrCount);
//    void GetAttrInfo(vector<AttrCatRecord> &attrs);

private:
    SM_Manager* smManager;
    RM_Manager* rmManager;
    RM_FileHandle rmFH;
    RM_FileScan rmFS;
    string relationName, attrName;
    bool haveValue;
    CmpOp op;
    Value value;
    int attrCount;
    vector<AttrCatRecord> attrs;

};

class QL_RootHandle: public QL_ScanHandle {
public:
    QL_RootHandle(SM_Manager* smm, shared_ptr<QL_ScanHandle> child, const string &relationName);
    ~QL_ProjectHandle();

    RC Open();
    RC Close();
    RC GetNext(char* recordData);

private:
    SM_Manager* smManager;
    shared_ptr<QL_ScanHandle> child;
};
#endif //ZBASE_QL_SCAN_H

class QL_FilterHandle: public QL_ScanHandle{
public:
    QL_FilterHandle(SM_Manager* smm, shared_ptr<QL_ScanHandle> child, Condition filter);
    ~QL_FilterHandle();

    RC Open();
    RC Close();
    RC GetNext(char* recordData);

private:
    SM_Manager* smManager;
    shared_ptr<QL_ScanHandle> child;
    Condition filter;
};