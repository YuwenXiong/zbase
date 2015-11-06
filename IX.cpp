#include "IX.h"
#include "zbase.h"
#include <sstream>
using namespace std;
RC IX_IndexHandle::InsertEntry(const Value &data, const RID &rid){
    AttrType type=b_tree.header.type;
    B_Entry* newEntry=new B_Entry;
    newEntry->Init(&b_tree,0);
    switch(type){
        case INT: newEntry->ikey=data.iData;break;
        case FLOAT: newEntry->fkey=data.fData;break;
        case CHARN: newEntry->skey=data.strData;break;
    }
    newEntry->rid=rid;
    return b_tree.Insert(newEntry);

}
RC IX_IndexHandle::DeleteEntry(Value &data, const RID &rid){
    AttrType type=b_tree.header.type;
    B_Entry* newEntry=new B_Entry;
    newEntry->Init(&b_tree,0);
    switch(type){
        case INT: newEntry->ikey=data.iData;break;
        case FLOAT: newEntry->fkey=data.fData;break;
        case CHARN: newEntry->skey=data.strData;break;
    }
    newEntry->rid=rid;
    return b_tree.Delete(newEntry);
}

IX_Manager::IX_Manager(PF_Manager &pfm1) {
    pfm=&pfm1;
}
RC IX_IndexScan::OpenScan(IX_IndexHandle &indexHandle, CmpOp op, const Value& data){
    ixh=&indexHandle;
    AttrType type=ixh->b_tree.header.type;
    cmpEntry=new B_Entry;
    cmpEntry->Init(&ixh->b_tree,0);
    ixh->b_tree.LoadRoot();
    curNode=ixh->b_tree.root_ptr;
    curPos=0;
    this->op=op;
    B_Node* son;
    if(curNode->header.numEntries==0)
        return IX_NO_SCAN_RESULT;
    switch(type){
        case INT: cmpEntry->ikey=data.iData;break;
        case FLOAT: cmpEntry->fkey=data.fData;break;
        case CHARN: cmpEntry->skey=data.strData;break;
    }

    switch(op){
        case NE:case NO:case LT:case LE:
            while(curNode->header.level!=0){
                curNode=curNode->entries[0]->GetSon();
//                delete curNode;
//                curNode=son;
            }
            break;
        case GT:case GE:case EQ:
            while(curNode->header.level!=0){
                for(int i=0;i<curNode->header.numEntries;i++){
                    if(i==(curNode->header.numEntries-1)){

                            curNode=curNode->entries[i]->GetSon();
//                            B_Node* son=curNode->entries[i]->GetSon();
//                            delete curNode;
//                            curNode=son;
                            break;

                    }
                    if(curNode->entries[i]->Compare(LE,cmpEntry)&&curNode->entries[i+1]->Compare(GT,cmpEntry)){
                        curNode=curNode->entries[i]->GetSon();
//                        B_Node* son=curNode->entries[i]->GetSon();
//                        delete curNode;
//                        curNode=son;
                        break;
                    }
                }
            }
            for(int i=0;i<curNode->header.numEntries;i++){
                if(curNode->entries[i]->Compare(op,cmpEntry)){
                    curPos=i;
                    break;
                }
                if(i==curNode->header.numEntries-1){
                    delete curNode;
                    curNode=NULL;
                    break;
                }
            }
            break;

    }

    return 0;
}

RC IX_IndexScan::GetNextEntry(RID &rid){
    if (curNode==NULL)
        return IX_EOF;
    else{
        if(curNode->entries[curPos]->Compare(op,cmpEntry)){
            rid=curNode->entries[curPos]->rid;
        }
        else if(op==LT||op==LE){
                    delete curNode;
                    curNode=NULL;
                    return IX_EOF;
        }
        if((++curPos)==curNode->header.numEntries) {
            if (curNode->header.rightSibling == NULL_PAGE) {
                    delete curNode;
                curNode = NULL;
            }
            else {
                B_Node *right = curNode->GetRightSibling();

                    delete curNode;
                curNode = right;
                curPos = 0;
            }
        }
    }
    return 0;
}

RC IX_IndexScan::CloseScan(){
    if(cmpEntry)
        delete cmpEntry;
    if(curNode != NULL && curNode != ixh->b_tree.root_ptr)
        delete curNode;
    return 0;
}
RC IX_Manager::CreateIndex(const string &fileName, int indexNo, AttrType attrType, int attrLength) {
    FILE *fid; // file identifier, we don't expose fd to avoid portability problem

    stringstream temp;
    temp << indexNo;
    if ((fid = fopen((fileName + temp.str()).c_str(), "w")) == NULL) {
        return PF_SYSTEM_ERROR;
    }

    PF_FileHeader fileHeader;
    fileHeader.firstFree = PF_PAGE_LIST_END;
    fileHeader.numPages = 0;
    B_TreeHeader b_treeHeader;
    b_treeHeader.root=NULL_PAGE;
    b_treeHeader.keysize=attrLength;
    b_treeHeader.type=attrType;

    size_t numBytes = fwrite(&fileHeader, sizeof(fileHeader), 1, fid);
    if (numBytes != 1) {
        fclose(fid);
        unlink(fileName.c_str());
        return PF_SYSTEM_ERROR;
    }
    numBytes = fwrite(& b_treeHeader, sizeof( b_treeHeader), 1, fid);
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

RC IX_Manager::OpenIndex(const string &fileName, int indexNo, IX_IndexHandle &indexHandle) {
    RC rc;

    stringstream temp;
    temp << indexNo;
    if((rc=pfm->OpenFile((fileName + temp.str()).c_str(),indexHandle.b_tree.pffh)))
        return rc==PF_FILE_ALREADY_OPEN?IX_INDEX_ALREADY_OPEN:IX_INDEX_NOT_FOUND;

//    if (fseek(indexHandle.b_tree.pffh.fp,B_TREE_HEADER_OFFSET, SEEK_SET) < 0) {
//        return PF_SYSTEM_ERROR;
//    }
    size_t numBytes = fread(&(indexHandle.b_tree.header), sizeof(indexHandle.b_tree.header), 1, indexHandle.b_tree.pffh.fp);
    if (numBytes != 1) {
        rc = PF_SYSTEM_ERROR;
        fclose(indexHandle.b_tree.pffh.fp);
        indexHandle.b_tree.pffh.fileOpen = false;
        return rc;
    }
    if(indexHandle.b_tree.header.root==NULL_PAGE){
        if((rc=indexHandle.b_tree.Init()))
            return rc;
    }
    if((rc=indexHandle.b_tree.LoadRoot()))
        return rc;
    return 0;
}
RC IX_Manager::DestroyIndex(const string &fileName, int indexNo){
    RC rc;
    stringstream temp;
    temp << indexNo;
    if((rc=pfm->DestroyFile((fileName + temp.str()).c_str())))
        return rc;
    return RC_OK;
}

RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle) {
    RC rc;
//    if((rc=indexHandle.b_tree.DelRoot())){
//        return rc;
//    }
    if (!indexHandle.b_tree.pffh.fileOpen) {
        return PF_FILE_CLOSED;
    }

        if (fseek(indexHandle.b_tree.pffh.fp,B_TREE_HEADER_OFFSET, SEEK_SET) < 0) {
            return PF_SYSTEM_ERROR;
        }
        if (fwrite(&indexHandle.b_tree.header, sizeof(B_TreeHeader), 1, indexHandle.b_tree.pffh.fp) != 1) {
            return PF_WRITE_ERROR;
        }

 //   indexHandle.b_tree.DelRoot();
    if((rc=indexHandle.b_tree.pffh.ForcePages()))
        return rc;

    return pfm->CloseFile(indexHandle.b_tree.pffh);

}