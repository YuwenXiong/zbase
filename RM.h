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

const size_t FIRST_SLOT=sizeof(PF_PageHeader);
const size_t RM_RECORD_HEADER_OFFSET=sizeof(PF_FileHeader);
struct RM_RecordFileHeader{
    int slotPerPage;
    int recordSize;
    int slotSize;
    RID firstFreeSlot;
    RID lastFreeSlot;
};
struct RM_SlotHeader{
    bool empty;
    RID nextFreeSlot;
};
class RID {
public:

    ~RID();
    RID(PageNum pageNum =NULL_PAGE, SlotNum slotNum=NULL_SLOT);
    RC GetPageNum(PageNum &pageNum) const;
    RC GetSlotNum(SlotNum &slotNum) const;
    bool operator==(const RID &rid)const;
    bool operator>=(const RID &rid)const;
private:
    PageNum pageNum;
    SlotNum slotNum;
};

class RM_Record {
    friend class RM_FileHandle;
public:
    RM_Record();
    RM_Record(int recordSize,RID rid);
    ~RM_Record();
    RC GetData(char* &data) const;
    RC GetRid(RID &rid) const;
private:
    char* data;
    RID rid;
};

class RM_FileHandle {
    friend class RM_Manager;
    friend class RM_FileScan;
public:
    RM_FileHandle();
    ~RM_FileHandle();
    RC GetRecord(const RID &rid, RM_Record &rec) const;
    RC InsertRecord(const char* data, RID &rid);
    RC DeleteRecord(const RID &rid);
    RC UpdateRecord(const RM_Record &record);
    RC ForcePages(PageNum pageNum = ALL_PAGES) ;

private:
    PF_FileHandle pffh;
    PF_PageHandle pfph;
    RM_RecordFileHeader header;
    bool headerChanged;
};

class RM_Manager {
public:
    RM_Manager (PF_Manager &pfm);
    ~RM_Manager();
    RC CreateFile(const string &fileName, int recordSize);
    RC DestoryFile(const string &fileName);
    RC OpenFile(const string &fileName, RM_FileHandle &fileHandle);
    RC CloseFile(RM_FileHandle &fileHandle);
private:
    PF_Manager *pfm;

};

class RM_FileScan {
public:
    RM_FileScan();
    ~RM_FileScan();
    RC OpenScan(const RM_FileHandle &fileHandle, AttrType attrType, size_t attrLength, int attrOffset, CmpOp op, Value value);
    RC GetNextRecord(RM_Record &record);
    RC CloseScan();
private:
    RID currentRID;
    AttrType type;
    int length;
    int offset;
    Value value;
    CmpOp op;
    bool open=false;
    RC UpdateRID();
    bool IsValid(RM_Record &record);

    RM_FileHandle* rmfh;
};
//
const RC RM_NULL_PAGE=RM_RC+1;
const RC RM_NULL_SLOT=RM_RC+2;
const RC RM_READ_EMPTY_SLOT=RM_RC+3;
const RC RM_ALREADY_DELETED=RM_RC+4;
const RC RM_NO_VALID_DATA=RM_RC+5;
const RC RM_FILESCAN_ALREADY_OPEN=RM_RC+6;
const RC RM_SCAN_EMPTY_RECORD=RM_RC+7;
#endif //ZBASE_RM_H
