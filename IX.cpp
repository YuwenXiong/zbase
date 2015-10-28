#include "IX.h"
#include "zbase.h"
IX_Manager::IX_Manager(PF_Manager &pfm) {
    this->pfm=&pfm;
}

RC IX_Manager::CreateIndex(const string &fileName, int indexNo, AttrType attrType, int attrLength) {

}

RC IX_Manager::OpenIndex(const string &fileName, int indexNo, IX_IndexHandle &indexHandle) {
    RC rc;
    if(rc=pfm->OpenFile(fileName.c_str(),indexHandle.pffh)!=0)
        return rc;
    size_t numBytes = fread(&(indexHandle.b_tree.header), sizeof(indexHandle.b_tree.header), 1, indexHandle.pffh.fp);
    if (numBytes != sizeof(B_TreeHeader)) {
        rc = PF_SYSTEM_ERROR;
        fclose(indexHandle.pffh.fp);
        indexHandle.pffh.fileOpen = false;
        return rc;
    }



    return 0;
}

RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle) {
    RC rc;
    if(indexHandle.b_tree.root_ptr){
        delete indexHandle.b_tree.root_ptr;
    }
    if (!indexHandle.pffh.fileOpen) {
        return PF_FILE_CLOSED;
    }
    if (indexHandle.b_tree.headerChanged) {
        if (fseek(indexHandle.pffh.fp, 0,B_TREE_HEADER_OFFSET) < 0) {
            return PF_SYSTEM_ERROR;
        }
        if (fwrite(&indexHandle.b_tree.header, sizeof(B_TreeHeader), 1, indexHandle.pffh.fp) != 1) {
            return PF_WRITE_ERROR;
        }
        indexHandle.b_tree.headerChanged = false;
    }
    RC rc;
    if((rc=indexHandle.pffh.ForcePages()))
        return rc;
    return pfm->CloseFile(indexHandle.pffh);

}