#include <assert.h>
#include "zbase.h"
#include "RM.h"
#include "SM.h"

RID::RID(PageNum pageNum, SlotNum slotNum):pageNum(pageNum),slotNum(slotNum){ }

RC RID::GetPageNum(PageNum &pageNum) const {
    if (this->pageNum==NULL_PAGE)return RM_NULL_PAGE;
    else {
        pageNum=this->pageNum;
        return 0;
    }
}
RC RID::GetSlotNum(SlotNum &slotNum) const {
    if(this->slotNum==NULL_SLOT)return RM_NULL_SLOT;
    else{
        slotNum=this->slotNum;
        return 0;
    }
}
bool RID::operator==(const RID &rid)const{
    return (pageNum==rid.pageNum)&&(slotNum==rid.slotNum);
}
bool RID::operator>=(const RID &rid)const{
    return (pageNum>rid.pageNum)||((pageNum==rid.pageNum)&&(slotNum>=rid.slotNum));
}
RM_Manager::RM_Manager(PF_Manager &pfm):pfm(&pfm){
}

RC RM_Record::GetRid(RID &rid) const{
    rid=this->rid;
    return 0;
}
RC RM_Record::GetData(char *&data) const {
    RC rc;
    if(data==NULL)
        return RM_NO_VALID_DATA;
    else {
        data=this->data;
        return 0;
    }

}
RM_Record::RM_Record(int recordSize,RID rid):rid(rid){
    data=new char[recordSize];
}
RM_Record::~RM_Record(){
    if(data)
        delete []data;
}
RC RM_Manager::CreateFile(const string &fileName, int recordSize){
    FILE *fid; // file identifier, we don't expose fd to avoid portability problem

    if ((fid = fopen(fileName.c_str(), "w")) == NULL) {
        return PF_SYSTEM_ERROR;
    }

    PF_FileHeader fileHeader;
    fileHeader.firstFree = PF_PAGE_LIST_END;
    fileHeader.numPages = 0;
    RM_RecordFileHeader recordFileHeader;
    recordFileHeader.firstFreeSlot=RID();
    recordFileHeader.lastFreeSlot=RID(0,0);
    recordFileHeader.recordSize=recordSize;
    recordFileHeader.slotSize=recordSize+sizeof(RM_SlotHeader);
    recordFileHeader.slotPerPage=PF_PAGE_SIZE/recordFileHeader.slotSize;

    size_t numBytes = fwrite(&fileHeader, sizeof(fileHeader), 1, fid);
    if (numBytes != 1) {
        fclose(fid);
        unlink(fileName.c_str());
        return PF_SYSTEM_ERROR;
    }
    numBytes = fwrite(& recordFileHeader, sizeof( recordFileHeader), 1, fid);
    if (numBytes != 1) {
        fclose(fid);
        unlink(fileName.c_str());
        return PF_SYSTEM_ERROR;
    }

    if (fclose(fid) != 0) {
        return PF_SYSTEM_ERROR;
    }

    return RC_OK;

}

RC RM_Manager::OpenFile(const string &fileName, RM_FileHandle &fileHandle) {
    RC rc;
    if((rc=pfm->OpenFile(fileName.c_str(),fileHandle.pffh)))
        return rc;
    size_t numBytes = fread(&(fileHandle.header), sizeof(fileHandle.header), 1, fileHandle.pffh.fp);
    if (numBytes != 1) {
        rc = PF_SYSTEM_ERROR;
        fclose(fileHandle.pffh.fp);
        fileHandle.pffh.fileOpen = false;
        return rc;
    }
    fileHandle.headerChanged=false;
    return RC_OK;

}
RC RM_Manager::DestroyFile(const string &fileName) {
    return pfm->DestroyFile(fileName.c_str());

}
RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
    RC rc;
    if((rc=fileHandle.ForcePages()))return rc;
    return pfm->CloseFile(fileHandle.pffh);
}

RC RM_FileHandle::GetRecord(const RID &rid, RM_Record &rec) {
    PageNum pageNum;
    SlotNum slotNum;
    RC rc;
    if ((rc=rid.GetPageNum(pageNum)))
        return rc;
    if(rid.GetSlotNum(slotNum)!=0)
        return rc;

    if((rc=pffh.GetThisPage(pageNum,pfph)))
        return rc;
    if((*(RM_SlotHeader*)(pfph.pageData+slotNum*header.slotSize)).empty)
        return RM_READ_EMPTY_SLOT;
    rec=RM_Record(header.recordSize,rid);
    memcpy(rec.data,pfph.pageData+slotNum*header.slotSize+sizeof(RM_SlotHeader),header.recordSize);
    return 0;
}
RC RM_FileHandle::DeleteRecord(const RID &rid) {
    PageNum pageNum;
    SlotNum slotNum;
    RC rc;
    if ((rc=rid.GetPageNum(pageNum)))
        return rc;
    if((rc=rid.GetSlotNum(slotNum)))
        return rc;

    if((rc=pffh.GetThisPage(pageNum,pfph)))
        return rc;
    if((*(RM_SlotHeader*)(pfph.pageData+slotNum*header.slotSize)).empty)
        return RM_ALREADY_DELETED;
    RM_SlotHeader slotHeader;
    slotHeader.empty=true;
    slotHeader.nextFreeSlot=header.firstFreeSlot;
    header.firstFreeSlot=rid;
    memcpy(pfph.pageData+slotNum*header.slotSize,&slotHeader,sizeof(RM_SlotHeader));

    if((rc=pffh.MarkDirty(pageNum)))
        return rc;
    return 0;
}

RC RM_FileHandle::UpdateRecord(const RM_Record &record) {
    RID rid;
    PageNum pageNum;
    SlotNum slotNum;
    RC rc;
    if((rc=record.GetRid(rid)))
        return rc;
    if ((rc=rid.GetPageNum(pageNum)))
        return rc;
    if((rc=rid.GetSlotNum(slotNum)))
        return rc;
    if((rc=pffh.GetThisPage(pageNum,pfph)))
        return rc;
    memcpy(pfph.pageData+slotNum*header.slotSize+sizeof(RM_SlotHeader),record.data,header.recordSize);
    if((rc=pffh.MarkDirty(pageNum)))
        return rc;
    return 0;
}

RC RM_FileHandle::InsertRecord(const char *data, RID &rid){
    PageNum pageNumF,pageNumL;
    RC rc;
    SlotNum slotNumF,slotNumL;
    bool insertAtTail=false;
    if(header.firstFreeSlot.GetPageNum(pageNumF)==RM_NULL_PAGE){
        if((rc=pffh.AllocatePage(pfph)))
            return rc;
        pageNumF=0;
        slotNumF=0;
        header.lastFreeSlot=RID(0,1);
    }
    else {
        header.firstFreeSlot.GetPageNum(slotNumF);
        if (slotNumF==header.slotPerPage){
            if((rc=pffh.AllocatePage(pfph)))
                return rc;
            pageNumF++;
            slotNumF=0;
            header.lastFreeSlot=RID(pageNumF,slotNumF+1);

        }
        else{
            if((rc=pffh.GetThisPage(pageNumF,pfph)))
                return rc;
            if(header.lastFreeSlot==RID(pageNumF,slotNumF)){
                insertAtTail=true;
                header.lastFreeSlot=RID(pageNumF,slotNumF+1);
            }
        }
    }
    if(!insertAtTail)
        header.firstFreeSlot=*(RID*)(pfph.pageData+slotNumF*header.slotSize+sizeof(bool));
    else
        header.firstFreeSlot=RID(pageNumF,slotNumF+1);
    RM_SlotHeader slotHeader;
    slotHeader.empty=false;
    slotHeader.nextFreeSlot=RID();
    memcpy(pfph.pageData+slotNumF*header.slotSize,&slotHeader,sizeof(RM_SlotHeader));
    memcpy(pfph.pageData+slotNumF*header.slotSize+sizeof(RM_SlotHeader),data,header.recordSize);
    rid=RID(pageNumF,slotNumF);
    if((rc=pffh.MarkDirty(pageNumF)))
        return rc;
    return 0;

}

RC RM_FileHandle::ForcePages(PageNum pageNum)  {
    if (!pffh.fileOpen) {
        return PF_FILE_CLOSED;
    }
    if (headerChanged) {
        if (fseek(pffh.fp,RM_RECORD_HEADER_OFFSET, SEEK_SET) < 0) {
            return PF_SYSTEM_ERROR;
        }
        if (fwrite(&header, sizeof(RM_RecordFileHeader), 1, pffh.fp) != 1) {
            return PF_WRITE_ERROR;
        }
        headerChanged = false;
    }
    RC rc;
    if((rc=pffh.ForcePages(pageNum)))
        return rc;
    return RC_OK;
}

RC RM_FileScan::OpenScan(RM_FileHandle &fileHandle, AttrType attrType, size_t attrLength, int attrOffset, CmpOp op,
                         Value value) {
    if(open)return RM_FILESCAN_ALREADY_OPEN;
    if(fileHandle.header.lastFreeSlot==RID(0,0))return RM_SCAN_EMPTY_RECORD;
    type=attrType;
    length=attrLength;
    offset=attrOffset;
    this->op=op;
    this->value=value;
    currentRID=RID(0,0);
    open=true;
    rmfh=&fileHandle;
    return 0;
}
RC RM_FileScan::CloseScan() {
    open=false;
    rmfh=NULL;
    return 0;
}
RC RM_FileScan::GetNextRecord(RM_Record &record) {
    RC rc;
    if(currentRID>=rmfh->header.lastFreeSlot)return RM_EOF;
    while(rmfh->GetRecord(currentRID,record)==RM_READ_EMPTY_SLOT||!IsValid(record)){
        if((rc=UpdateRID()))
            return rc;
    }
    UpdateRID();
    return 0;
}
bool RM_FileScan::IsValid(RM_Record &record){
    char* data;
    record.GetData(data);
    if (type==INT){
        int rec=*(int*)(data+offset);
        int val=value.iData;
        switch (op){
            case EQ:
                return rec==val;
            case NE:
                return rec!=val;
            case GT:
                return rec>val;
            case GE:
                return rec>=val;
            case LT:
                return rec<val;
            case LE:
                return rec<=val;
            case NO:
                return true;
        }
    }
    if (type==FLOAT){
        float rec=*(float*)(data+offset);
        float val=value.fData;
        switch (op){
            case EQ:
                return rec==val;
            case NE:
                return rec!=val;
            case GT:
                return rec>val;
            case GE:
                return rec>=val;
            case LT:
                return rec<val;
            case LE:
                return rec<=val;
            case NO:
                return true;
        }
    }
    if(type==CHARN){
        char* recC=new char[length];
        memcpy(recC,data+offset,length);
        string rec(recC);
        delete[] recC;
        string val=value.strData;
        switch (op){
            case EQ:
                return rec==val;
            case NE:
                return rec!=val;
            case GT:
                return rec>val;
            case GE:
                return rec>=val;
            case LT:
                return rec<val;
            case LE:
                return rec<=val;
            case NO:
                return true;
        }
    }
    assert(0);
    return RC_OK;
}

RC RM_FileScan::UpdateRID() {
    PageNum pageNum;
    SlotNum slotNum;
    RC rc;
    if((rc=currentRID.GetPageNum(pageNum)))
        return rc;
    if((rc=currentRID.GetSlotNum(slotNum)))
        return rc;
    if(++slotNum==rmfh->header.slotPerPage){
        slotNum=0;
        pageNum++;
    }
    currentRID=RID(pageNum,slotNum);
    if(currentRID>=rmfh->header.lastFreeSlot)
        return RM_EOF;
    return 0;
}


RM_FileScan::RM_FileScan(){
    open=false;
}



