//
// Created by Zax on 2015/10/28.
//

#include <iostream>
#include "B_Tree.h"
#include "RM.h"
#include "IX.h"


B_Tree::B_Tree() {
    root_ptr=NULL;
}

RC B_Tree::Init() {
    root_ptr=NewOneNode();
    root_ptr->Init(0,this);
    header.root=root_ptr->header.pageNum;
    DelRoot();

    // wating for check
//    root_ptr=NewOneNode();
//    root_ptr->Init(0,this);
//    header.root=root_ptr->header.pageNum;
//    DelRoot();
}
RC B_Tree::LoadRoot(){
    if(root_ptr==NULL){
        root_ptr=NewOneNode();
        RC rc;
        if(rc=root_ptr->Init(this,header.root))
            return rc;
    }
    return 0;
}
RC B_Tree::DelRoot(){
    PageNum pageNum;
    RC rc;
    if(root_ptr){
//        if(rc=root_ptr->pfph.GetPageNum(pageNum))
//            return rc;
//        if(rc=pffh.ForcePages(pageNum))
//            return rc;
        delete root_ptr;
        root_ptr=NULL;
    }

    return 0;
}

B_Node* B_Tree::NewOneNode() {
    B_Node *node= new B_Node;
    return node;
}


B_Node::B_Node() {
    header.pageNum=NULL_PAGE;
    header.capacity=-1;
    header.leftSibling=header.rightSibling=NULL_PAGE;
    header.level=-1;
    b_tree=NULL;
    header.numEntries=-1;

}

RC B_Node::Init(int level, B_Tree *b_tree) {
    this->b_tree=b_tree;
    MarkDirty();
    header.level=level;
    header.numEntries=0;
    header.capacity=(PF_PAGE_SIZE-sizeof(B_NodeHeader))/(b_tree->GetEntrySize(level));
    RC rc;
    if(rc=b_tree->pffh.AllocatePage(pfph))
        return rc;
    PageNum pageNum;
    if(rc=pfph.GetPageNum(pageNum))
        return rc;
    header.pageNum=pageNum;
    if(rc=GetEntries())
        return rc;
    return 0;
}
RC B_Node::Init(B_Tree *b_tree, int pageNum) {
    this->b_tree=b_tree;
    RC rc;
    if(rc=b_tree->pffh.GetThisPage(pageNum,pfph))
        return rc;
    char* data;
    if(rc=pfph.GetData(data))
        return rc;
    memcpy(&header,data,sizeof(B_NodeHeader));
    if(rc=GetEntries())
        return rc;
    int offset=sizeof(B_NodeHeader);
    for(int i=0;i<header.numEntries;i++){
        entries[i]->ReadFromPage(data+offset);
        offset+=b_tree->GetEntrySize(entries[i]->level);
    }
    return 0;
}



RC B_Node::GetEntries(){
    if (header.capacity==0)
        return IX_NO_CAPACITY;
    entries =new B_Entry*[header.capacity];
    for(int i=0;i<header.capacity;i++){
        entries[i]=new B_Entry;
        entries[i]->Init(b_tree,header.level);
    }
    return 0;
}

int B_Tree::GetEntrySize(const int& level) {
    int ret;
    if(level==0)
        ret=header.keysize+sizeof(RID);
    else
        ret=header.keysize+sizeof(PageNum);
    return ret;
}

RC B_Entry::Init(B_Tree* b_tree1,const int &level){
    b_tree=b_tree1;
    son_ptr=NULL;
    son=NULL_PAGE;
    this->level=level;
    return RC_OK;
}
RC B_Entry::ReadFromPage(char* data){
    switch(b_tree->header.type){
        case INT: memcpy(&ikey,data,b_tree->header.keysize);break;
        case FLOAT: memcpy(&fkey,data,b_tree->header.keysize);break;
        case CHARN: skey=string(data);break;
    }
    if(level==0)
        memcpy(&rid,data+b_tree->header.keysize,sizeof(RID));
    else
        memcpy(&son,data+b_tree->header.keysize,sizeof(PageNum));
    return 0;
}

RC B_Entry::WriteToPage(char* data){
    switch(b_tree->header.type){
        case INT: memcpy(data,&ikey,b_tree->header.keysize);break;
        case FLOAT: memcpy(data,&fkey,b_tree->header.keysize);break;
        case CHARN: memcpy(data,skey.c_str(),skey.length()+1);;break;
    }
    if(level==0)
        memcpy(data+b_tree->header.keysize,&rid,sizeof(RID));
    else
        memcpy(data+b_tree->header.keysize,&son,sizeof(PageNum));


    return 0;
}

RC B_Entry::SetFromSon(B_Node* nodeSon){
    son=nodeSon->header.pageNum;
    switch(b_tree->header.type){
        case INT: ikey=nodeSon->entries[0]->ikey;break;
        case FLOAT: ikey=nodeSon->entries[0]->ikey;break;
        case CHARN: skey=nodeSon->entries[0]->skey;break;
    }
    return RC_OK;
}
RC B_Tree::Insert(B_Entry* newEntry){
    LoadRoot();
    B_Node* newNode=NULL;
    BINSRT insertRet=root_ptr->Insert(newEntry,newNode);
    if(insertRet==B_OVRFLW){
        B_Node* newRoot=new B_Node;
        newRoot->Init(root_ptr->header.level+1,this);
        newRoot->AddNewSon(root_ptr);
        newRoot->AddNewSon(newNode);
        header.root=newRoot->header.pageNum;
        root_ptr=newRoot;
    }
    DelRoot();
    return 0;
}
RC B_Node::RemoveEntry(int pos) {
    for(int i=pos;i<header.numEntries-1;i++){
        entries[i]->SetFromEntry(entries[i+1]);
    }
    header.numEntries--;
    MarkDirty();
    return 0;
}
RC B_Tree::Delete(B_Entry* b_entry) {
    RC rc;
    LoadRoot();
    BDEL cret = root_ptr->Delete(b_entry);
    if (cret == B_UNDRFLW)
        header.root = root_ptr->entries[0]->son;
    if (cret == B_NOTFOUND)
        return IX_DELETE_NOT_FOUND;
    DelRoot();
    return 0;
}

BDEL B_Node::Delete(B_Entry* b_entry){
    BDEL ret=B_NOTFOUND;
    if(header.level==0){
        for(int i=0;i<header.numEntries;i++){
            if(entries[i]->IsSameRecord(b_entry)) {
                delete b_entry;
                RemoveEntry(i);
                MarkDirty();
                if(CheckUnderFlow())
                    ret=B_UNDRFLW;
                else
                    ret=B_NONE;
                break;
            }
        }
        return ret;
    }
    int follow=MaxLEKeyPos(b_entry);
    if(follow==-1)
        return B_NOTFOUND;
    B_Node *succ=entries[follow]->GetSon();
    BDEL cret=succ->Delete(b_entry);
    entries[follow]->SetFromSon(succ);
    MarkDirty();
    entries[follow]->DeleteSon();
    if(cret==B_NONE)
        ret=B_NONE;
    if(cret==B_UNDRFLW){
        TreatUnderFlow(follow);
        if(CheckUnderFlow())
            ret=B_UNDRFLW;
        else
            ret=B_NONE;
    }
    return ret;
}

RC B_Entry::DeleteSon() {
    if(son_ptr){
        delete son_ptr;
        son_ptr=NULL;
    }
    return RC_OK;
}


RC B_Entry::SetFromEntry(B_Entry *b_entry) {
    if(level!=b_entry->level)
        return IX_SET_FROM_DIFFERENT_LEVEL;
    switch(b_tree->header.type){
        case INT: ikey=b_entry->ikey;break;
        case FLOAT: ikey=b_entry->ikey;break;
        case CHARN: skey=b_entry->skey;break;
    }
    if(level==0)
        rid=b_entry->rid;
    else
        son=b_entry->son;
    return 0;
}
bool B_Node::CheckOverFlow(){
    if(header.numEntries==header.capacity-1)return true;
    else return false;
}
bool B_Node::CheckUnderFlow() {
    if(b_tree->root_ptr->header.level==header.level)
        if(header.numEntries==1&&header.level>0)
            return true;
        else return false;
    if(header.numEntries<=(header.capacity-2)/2)return true;
    else return false;
}

B_Node::~B_Node() {
    PageNum pageNum;
    char* data;
    RC rc;
    rc=pfph.GetData(data);
    rc=pfph.GetPageNum(pageNum);
    if(dirty){
        memcpy(data,&header,sizeof(B_NodeHeader));
    }
    if(header.numEntries){
        int offset=sizeof(B_NodeHeader);
        int size=b_tree->GetEntrySize(header.level);
        for(int i=0;i<header.capacity;i++){
            if(entries[i]){
                if(i<header.numEntries)
                    entries[i]->WriteToPage(data+offset+size*i);
                delete entries[i];
            }
        }
        delete []entries;
    }
    b_tree->pffh.UnpinPage(pageNum);
    b_tree->pffh.ForcePages(pageNum);
}
RC B_Node::TreatOverFlow(B_Node *&b_node) {
    b_node=new B_Node;
    RC rc;
    if(rc=b_node->Init(header.level,b_tree))
        return rc;
    int i=(header.capacity-1)/2;
    while(i<header.numEntries){
        b_node->Enter(entries[i]);
        RemoveEntry(i);
    }
    b_node->MarkDirty();
    b_node->header.rightSibling=header.rightSibling;
    b_node->header.leftSibling=header.pageNum;
    header.rightSibling=b_node->header.pageNum;

    if(b_node->header.rightSibling!=NULL_PAGE){
        B_Node* node=new B_Node();
        node->Init(b_tree,b_node->header.rightSibling);
        node->header.leftSibling=b_node->header.pageNum;
        node->MarkDirty();
        delete node;
    }

    return 0;
}

RC B_Node::TreatUnderFlow(int follow){
    int mergesub=follow+1;
    if(follow==header.numEntries-1)
        mergesub=follow-1;
    B_Node* succ1=entries[mergesub]->GetSon();
    B_Node* succ2=entries[follow]->GetSon();
    int totalnum=succ1->header.numEntries+succ2->header.numEntries;
    if(totalnum>=succ1->header.capacity){
        int n=succ1->header.capacity;
        if(mergesub>follow){
            for(int i=0;i<totalnum/2-succ2->header.numEntries;i++){
                succ2->Enter(succ1->entries[0]);
                succ1->RemoveEntry(0);
            }
        }
        else{
            for(int i=totalnum/2;i<n;i++){
                succ2->Enter(succ1->entries[totalnum/2]);
                succ1->RemoveEntry(totalnum/2);
            }
        }
    }
    else{
        for(int i=0;i<succ2->header.numEntries;i++)
            succ1->Enter(succ2->entries[i]);
        if(follow<mergesub){
            succ1->header.leftSibling=succ2->header.leftSibling;
            if(succ1->header.leftSibling!=NULL_PAGE){
                B_Node* node=new B_Node();
                node->Init(b_tree,succ1->header.leftSibling);
                node->header.rightSibling=succ1->header.pageNum;
                node->MarkDirty();
                delete node;
            }
        }
        else{
            succ1->header.rightSibling=succ2->header.rightSibling;
            if(succ1->header.rightSibling!=NULL_PAGE){
                B_Node* node=new B_Node();
                node->Init(b_tree,succ1->header.rightSibling);
                node->header.rightSibling=succ1->header.pageNum;
                node->MarkDirty();
                delete node;
            }
        }
        succ1->MarkDirty();
        succ2->MarkDirty();
        entries[mergesub]->SetFromSon(succ1);
        entries[mergesub]->DeleteSon();
        entries[follow]->DeleteSon();
        RemoveEntry(follow);
    }
    MarkDirty();
    return 0;
}


B_Node* B_Entry::GetSon(){
    RC rc;
    if(son_ptr==NULL){
        son_ptr=new B_Node();
        if(rc=son_ptr->Init(b_tree,son))
            cout<<rc;
    }
    return son_ptr;
}

BINSRT B_Node::Insert(B_Entry* b_entry,B_Node*& b_node){
    BINSRT ret=B_NRML;
    if(header.level==0){
        MarkDirty();
        Enter(b_entry);
        delete b_entry;
        if(CheckOverFlow()){
            TreatOverFlow(b_node);
            ret=B_OVRFLW;
        }
        return ret;
    }
    int follow=MaxLEKeyPos(b_entry);
    bool needUpdate;
    if(follow==-1){
        needUpdate=true;
        follow=0;
    }
    B_Node* succ=entries[follow]->GetSon();
    B_Node* tnode=NULL;
    BINSRT c_ret=succ->Insert(b_entry,tnode);
    if(needUpdate){
        entries[follow]->SetFromSon(succ);
        MarkDirty();
    }
    if(c_ret==B_OVRFLW)
        AddNewSon(tnode);
    if(CheckOverFlow()){
        TreatOverFlow(b_node);
        ret=B_OVRFLW;
    }
    return ret;
}

B_Entry::~B_Entry(){
    if(son_ptr){
        delete son_ptr;
    }
}
RC B_Node::AddNewSon(B_Node* b_node){
    B_Entry* b_entry=new B_Entry;
    b_entry->Init(b_tree,header.level);
    b_entry->SetFromSon(b_node);
    Enter(b_entry);
    delete b_entry;
    delete b_node;
}

RC B_Node::MarkDirty() {
    PageNum pageNum;
    pfph.GetPageNum(pageNum);
    dirty=true;
    return b_tree->pffh.MarkDirty(pageNum);
}

RC B_Node::Enter(B_Entry* b_entry){
    int pos=-1;
    pos=MaxLEKeyPos(b_entry);
    pos++;
    RC rc;
    if(rc=MarkDirty())
        return rc;
    for(int i=header.numEntries;i>pos;i--){
        entries[i]->SetFromEntry(entries[i-1]);
    }
    entries[pos]->SetFromEntry(b_entry);
    header.numEntries++;
    return 0;
}
B_Node* B_Node::GetRightSibling(){
    B_Node* ret=NULL;
    if(header.rightSibling!=NULL_PAGE){
        ret=new B_Node;
        ret->Init(b_tree,header.rightSibling);
    }
    return ret;
}
int B_Node::MaxLEKeyPos(B_Entry *b_entry){
    int pos=-1;
    for(int i= header.numEntries-1;i>=0;i--){
        if(entries[i]->Compare(LE,b_entry)){
            pos=i;
            break;
        }
    }
    return pos;
}
bool B_Entry::IsSameRecord(B_Entry* b_entry){
    return (rid==b_entry->rid)&&Compare(EQ,b_entry);
}

bool B_Entry::Compare(CmpOp op,B_Entry* b_entry){
    AttrType type=b_tree->header.type;


    if (type==INT){
        int rec=ikey;
        int val=b_entry->ikey;
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
        float rec=fkey;
        float val=b_entry->fkey;
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
        string rec=skey;
        string val=b_entry->skey;
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
}