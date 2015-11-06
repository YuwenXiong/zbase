//
// Created by Administrator on 2015/11/1.
//
#include "RM.h"
#include "IX.h"
#include "PF.h"
#include <iostream>
using namespace std;

int count=0;
int main(){
    RC rc;
    PF_Manager pfm;
    IX_Manager ixm(pfm);
    IX_IndexHandle ixih;
    IX_IndexScan ixis;

    if(rc=ixm.CreateIndex("indextest",0,INT,4))
        return rc;
    if(rc=ixm.OpenIndex("indextest",0,ixih))
        return rc;
    for(int i=0;i<10;i++){
        Value value((float)i);
        if(rc=ixih.InsertEntry(value,RID(i/10,i%10)))
            return rc;
    }
    if ((rc = ixm.CloseIndex(ixih))) {
        return rc;
    }

    if ((rc=ixm.OpenIndex("indextest",0,ixih))) {
        return rc;
    }
//    for(int i=0;i<1;i++){
//        Value value;
//        value.type=INT;
//        value.iData=i;
//
//        if(rc=ixih.DeleteEntry(value,RID(i/10,i%10)))
//            return rc;
//    }
    ixis.OpenScan(ixih,GE,Value((float)5.0));
    RID id;
    while(ixis.GetNextEntry(id)!=IX_EOF){
        PageNum pageNum;
        SlotNum slotNum;
        id.GetPageNum(pageNum);
        id.GetSlotNum(slotNum);
        cout<<pageNum<<"\t"<<slotNum<<endl;
    }
    if ((rc = ixis.CloseScan())) {
        return rc;
    }
    if ((rc = ixm.CloseIndex(ixih))) {
        return rc;
    }

     return 0;

}