//
// Created by Orpine on 10/25/15.
//

#ifndef ZBASE_RM_H
#define ZBASE_RM_H

#include <string>
#include "PF.h"
#include "zbase.h"

#define RM_EOF -1

using namespace std;

typedef int PageNum;
typedef int SlotNum;

class RID {
public:
    RID();
    ~RID();
    RID(PageNum pageNum, SlotNum slotNum);
    RC GetPageNum(PageNum &pageNum) const;
    RC GetSlotNum(SlotNum &slotNum) const;

private:
    PageNum pagenum;
    SlotNum slotnum;
};

class RM_Record {
public:
    RM_Record(int recordSize);
    ~RM_Record();
    RC GetData(char* &data) const;
    RC GetRid(RID &rid) const;
};

class RM_FileHandle {
public:
    RM_FileHandle();
    ~RM_FileHandle();
    RC GetRecord(const RID &rid, RM_Record &rec) const;
    RC InsertRecord(const char* data, RID &rid);
    RC DeleteRecord(const RID &rid);
    RC UpdateRecord(const RM_Record &record);
    RC ForcePages(PageNum pageNum = ALL_PAGES) const;
};

class RM_Manager {
public:
    RM_Manager (PF_Manager &pfm);
    ~RM_Manager();
    RC CreateFile(const string &fileName, int recordSize);
    RC DestoryFile(const string &fileName);
    RC OpenFile(const string &fileName, RM_FileHandle &fileHandle);
    RC CloseFile(RM_FileHandle &fileHandle);
};

class RM_FileScan {
public:
    RM_FileScan();
    ~RM_FileScan();
    RC OpenScan(const RM_FileHandle &fileHandle, AttrType attrType, int attrLength, int attrOffset, CmpOp op, void* value);
    RC GetNextRecord(RM_Record &record);
    RC CloseScan();
};

#endif //ZBASE_RM_H
