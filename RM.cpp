#include "zbase.h"
#include "RM.h"
#include "io.h"
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
    char * data=new char[recordSize];

}
RM_Record::~RM_Record(){
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
    if (numBytes != sizeof(fileHeader)) {
        fclose(fid);
        unlink(fileName.c_str());
        return PF_SYSTEM_ERROR;
    }
    numBytes = fwrite(& recordFileHeader, sizeof( recordFileHeader), 1, fid);
    if (numBytes != sizeof(recordFileHeader)) {
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
    if (fileHandle.pffh.fileOpen) {
        return PF_FILE_ALREADY_OPEN;
    }
    if ((fileHandle.pffh.fp = fopen(fileName.c_str(), "r")) == NULL) {
        return PF_SYSTEM_ERROR;
    }
    size_t numBytes = fread(&(fileHandle.pffh.header), sizeof(fileHandle.pffh.header), 1, fileHandle.pffh.fp);
    if (numBytes != sizeof(PF_FileHeader)) {
        rc = PF_SYSTEM_ERROR;
        fclose(fileHandle.pffh.fp);
        fileHandle.pffh.fileOpen = false;
        return rc;
    }

    numBytes = fread(&(fileHandle.header), sizeof(fileHandle.header), 1, fileHandle.pffh.fp);
    if (numBytes != sizeof( RM_RecordFileHeader)) {
        rc = PF_SYSTEM_ERROR;
        fclose(fileHandle.pffh.fp);
        fileHandle.pffh.fileOpen = false;
        return rc;
    }
    fileHandle.headerChanged=false;
    fileHandle.pffh.headerChanged = false;
    fileHandle.pffh.bufferManager = this->pfm->bufferManager;
    fileHandle.pffh.fileOpen = true;

    return RC_OK;

}
RC RM_Manager::DestoryFile(const string &fileName) {
    return pfm->DestoryFile(fileName.c_str());

}
RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
    RC rc;
    if(rc=fileHandle.ForcePages()!=0)return rc;
    return pfm->CloseFile(fileHandle.pffh);
}

RC RM_FileHandle::GetRecord(const RID &rid, RM_Record &rec) const {
    PageNum pageNum;
    SlotNum slotNum;
    RC rc;
    if (rid.GetPageNum(pageNum)!=0)
        return rc;
    if(rid.GetSlotNum(slotNum)!=0)
        return rc;

    if(rc=pffh.GetThisPage(pageNum,pfph)!=0)
        return rc;
    if((*(RM_SlotHeader*)(pfph.pageData+slotNum*header.slotSize)).empty==true)
        return RM_READ_EMPTY_SLOT;
    rec=RM_Record(header.recordSize,rid);
    memcpy(rec.data,pfph.pageData+slotNum*header.slotSize+sizeof(RM_SlotHeader),header.recordSize);
    return 0;
}
RC RM_FileHandle::DeleteRecord(const RID &rid) {
    PageNum pageNum;
    SlotNum slotNum;
    RC rc;
    if (rid.GetPageNum(pageNum)!=0)
        return rc;
    if(rid.GetSlotNum(slotNum)!=0)
        return rc;

    if(rc=pffh.GetThisPage(pageNum,pfph)!=0)
        return rc;
    if((*(RM_SlotHeader*)(pfph.pageData+slotNum*header.slotSize)).empty==true)
        return RM_ALREADY_DELETED;
    RM_SlotHeader slotHeader;
    slotHeader.empty=true;
    slotHeader.nextFreeSlot=header.firstFreeSlot;
    header.firstFreeSlot=rid;
    memcpy(pfph.pageData+slotNum*header.slotSize,&slotHeader,sizeof(RM_SlotHeader));

    if(rc=pffh.MarkDirty(pageNum)!=0)
        return rc;
    return 0;
}

RC RM_FileHandle::UpdateRecord(const RM_Record &record) {
    RID rid;
    PageNum pageNum;
    SlotNum slotNum;
    RC rc;
    if(rc=record.GetRid(rid)!=0)
        return rc;
    if (rid.GetPageNum(pageNum)!=0)
        return rc;
    if(rid.GetSlotNum(slotNum)!=0)
        return rc;
    if(rc=pffh.GetThisPage(pageNum,pfph)!=0)
        return rc;
    memcpy(pfph.pageData+slotNum*header.slotSize+sizeof(RM_SlotHeader),record.data,header.recordSize);
    if(rc=pffh.MarkDirty(pageNum)!=0)
        return rc;
    return 0;
}

RC RM_FileHandle::InsertRecord(const char *data, RID &rid){
    PageNum pageNumF,pageNumL;
    RC rc;
    SlotNum slotNumF,slotNumL;
    bool insertAtTail=false;
    if(header.firstFreeSlot.GetPageNum(pageNumF)==RM_NULL_PAGE){
        if(rc=pffh.AllocatePage(pfph)!=0)
            return rc;
        pageNumF=0;
        slotNumF=0;
        header.lastFreeSlot=RID(0,1);
    }
    else {
        header.firstFreeSlot.GetPageNum(slotNumF);
        if (slotNumF==header.slotPerPage){
            if(rc=pffh.AllocatePage(pfph)!=0)
                return rc;
            pageNumF++;
            slotNumF=0;
            header.lastFreeSlot=RID(pageNumF,slotNumF+1);

        }
        else{
            if(rc=pffh.GetThisPage(pageNumF,pfph)!=0)
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
    slotHeader.empty==false;
    slotHeader.nextFreeSlot=RID();
    memcpy(pfph.pageData+slotNumF*header.slotSize,&slotHeader,sizeof(RM_SlotHeader));
    memcpy(pfph.pageData+slotNumF*header.slotSize+sizeof(RM_SlotHeader),data,header.recordSize);
    rid=RID(pageNumF,slotNumF);
    if(rc=pffh.MarkDirty(pageNumF)!=0)
        return rc;
    return 0;

}

RC RM_FileHandle::ForcePages(PageNum pageNum)  {
    if (!pffh.fileOpen) {
        return PF_FILE_CLOSED;
    }
    if (headerChanged) {
        if (fseek(pffh.fp, 0,0) < 0) {
            return PF_SYSTEM_ERROR;
        }
        if (fwrite(&header, sizeof(RM_RecordFileHeader), 1, pffh.fp) != 1) {
            return PF_WRITE_ERROR;
        }
        headerChanged = false;
    }
    RC rc;
    if(rc=pffh.ForcePages(pageNum)!=0)
        return rc;
}

RC RM_FileScan::OpenScan(const RM_FileHandle &fileHandle, AttrType attrType, int attrLength, int attrOffset, CmpOp op,
                         Value value) {
    if(open)return RM_FILESCAN_ALREADY_OPEN;
    if(fileHandle.header.lastFreeSlot==RID(0,0))return RM_SCAN_EMPTY_RECORD;
    type=attrType;
    length=attrLength;
    offset=attrOffset;
    this->op=op;
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
    while(rmfh->GetRecord(currentRID,record)==RM_READ_EMPTY_SLOT){
        if(rc=updateRID()!=0)
            return rc;
    }
    updateRID();
    return 0;
}
RC RM_FileScan::updateRID() {
    PageNum pageNum;
    SlotNum slotNum;
    RC rc;
    if(rc=currentRID.GetPageNum(pageNum)!=0)
        return rc;
    if(rc=currentRID.GetSlotNum(slotNum)!=0)
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





