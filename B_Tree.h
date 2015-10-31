//
// Created by Zax on 2015/10/28.
//

#ifndef ZBASE_B_TREE_H
#include "zbase.h"
#include "PF.h"
#include "RM.h"
enum BINSRT {B_NRML, B_OVRFLW};
enum BDEL {B_NONE, B_UNDRFLW, B_NOTFOUND};

class B_Node;
class B_Entry;


struct  B_TreeHeader{
    PageNum root;
    int keysize;
    AttrType type;
};
struct B_NodeHeader{
    int level;
    int numEntries;
    int capacity;
    int rightSibling;
    int leftSibling;
    PageNum pageNum;
};


class B_Tree{
    friend class IX_Manager;
    friend class IX_IndexHandle;
    friend class IX_IndexScan;
    friend class B_Node;
    friend class B_Entry;
private:
    B_TreeHeader header;
    B_Node* root_ptr;
    PF_FileHandle pffh;
    bool headerChanged;

public:
    B_Tree();
    RC Init();
    RC LoadRoot();
    RC DelRoot();
    B_Node* NewOneNode();
    int GetEntrySize(const int& level);
    RC Insert(B_Entry* b_entry);
    RC Delete(B_Entry* b_entry);

};



class B_Entry{
    friend class B_Node;
    friend class IX_IndexHandle;
    friend class B_Tree;
    friend class IX_IndexScan;
private:
    int level;
    PageNum son;
    int ikey;
    float fkey;
    string skey;
    RID rid;
    B_Node* son_ptr;
    B_Tree* b_tree;
public:
    ~B_Entry();
    RC Init(B_Tree* b_tree1, const int& level);
    RC ReadFromPage(char* data);
    RC WriteToPage(char* data);
    RC SetFromSon(B_Node* sonNode);
    RC SetFromEntry(B_Entry* b_entry);
    B_Node* GetSon();
    RC DeleteSon();
    bool IsSameRecord(B_Entry* b_entry);
    bool Compare(CmpOp op,B_Entry* b_entry);

};
class B_Node{
    friend class B_Tree;
    friend class B_Entry;
    friend class IX_IndexScan;
private:
    B_NodeHeader header;
    B_Tree* b_tree;
    B_Entry** entries;
    bool dirty;
    B_Node* son_ptr;
public:
    B_Node();
    ~B_Node();
    PF_PageHandle pfph;
    RC Init(int level, B_Tree* b_tree);//init new node
    RC Init(B_Tree* b_tree, int pageNum);//init from the file
    RC GetEntries();
    RC Enter(B_Entry* b_entry);
    int MaxLEKeyPos(B_Entry *b_entry);
    B_Node* GetRightSibling();
    RC MarkDirty();
    BINSRT Insert(B_Entry* b_entry,B_Node*& b_node);
    BDEL Delete(B_Entry* b_entry);
    bool CheckOverFlow();
    bool CheckUnderFlow();
    RC TreatOverFlow(B_Node*& b_node);
    RC TreatUnderFlow(int follow);
    RC RemoveEntry(int pos);
    RC AddNewSon(B_Node* b_node);
};
#define ZBASE_B_TREE_H

#endif //ZBASE_B_TREE_H
