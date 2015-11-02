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

//    if(rc=ixm.CreateIndex("indextest",0,INT,4))
//        return rc;
//    if(rc=ixm.OpenIndex("indextest",0,ixih))
//        return rc;
//    for(int i=0;i<10000;i++){
//        Value value;
//        value.type=INT;
//        value.iData=i;
//        if(rc=ixih.InsertEntry(value,RID(i/10,i%10)))
//            return rc;
//    }
//    ixm.CloseIndex(ixih);
    rc=ixm.OpenIndex("indextest",0,ixih);
    for(int i=535;i<565;i++){
        Value value;
        value.type=INT;
        value.iData=i;
        if(rc=ixih.DeleteEntry(value,RID(i/10,i%10)))
            return rc;
    }

     return 0;

}