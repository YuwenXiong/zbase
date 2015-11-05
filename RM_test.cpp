#include "zbase.h"
#include "RM.h"
#include <iostream>
using namespace std;

int main(){
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    RM_FileHandle rmfh;
    RM_FileScan rmfs;
    rmm.CreateFile("test",4);
    rmm.OpenFile("test",rmfh);
    RID rid[100];
    RC rc;
    RM_Record record;
    for(int i=0;i<100;i++){
        if(rc=rmfh.InsertRecord((char*)&i,rid[i]))
            return rc;
    }
    for(int i=0;i<50;i+=2){
        if(rc=rmfh.DeleteRecord(rid[i]))
            return rc;
    }
    for(int i=10;i<20;i++){
        if(rc=rmfh.InsertRecord((char*)&i,rid[i]))
            return rc;
    }
    if(rc=rmfs.OpenScan(rmfh,INT,4,0,NO,Value(35)))
        return rc;
    while(true){
        if(rc=rmfs.GetNextRecord(record))
            return rc;
        char* data;
        record.GetData(data);
        cout<<*(int*)data<<endl;
    }


    if(rc=rmfs.GetNextRecord(record))
        return rc;


}


