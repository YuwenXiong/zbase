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




RC RM_Manager::CreateFile(const string &fileName, int recordSize){
    return pfm.CreateFile(fileName)

}
RM_Manager::RM_Manager (PF_Manager &pfm):pfm(pfm){
}
RC RM_Manager::OpenFile(const string &fileName, RM_FileHandle &fileHandle) {
    RC rc;
    if(rc=pfm.OpenFile(fileName,fileHandle.pffh)!=0)
        return rc;

}
RC RM_Manager::DestoryFile(const string &fileName) {
    return pfm.DestoryFile(fileName);

}
RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
    RC rc;
    if(rc=fileHandle.ForcePages()!=0)return rc;
    return pfm.CloseFile(fileHandle.pffh);
}

RC RM_FileHandle::GetRecord(const RID &rid, RM_Record &rec) const {
    pffh.GetThisPage(rid.GetPageNum())
}

RC RM_FileHandle::InsertRecord(const char *data, RID &rid) {
    return 0;
}
