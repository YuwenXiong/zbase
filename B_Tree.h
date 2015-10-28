//
// Created by Zax on 2015/10/28.
//

#ifndef ZBASE_B_TREE_H
#include "zbase.h"

struct  B_TreeHeader{
    PageNum root;
    int keysize;
};
struct B_NodeHeader{
    int level;
    int numEntries;
    int capacity;
    int rightSibling;
    int leftSibling;
};

struct B_EntryHeader{

};

class B_Tree{
    friend class IX_Manager;
private:
    B_TreeHeader header;
    B_Node * root_ptr;
    bool headerChanged;
public:
    B_Tree();
    ~B_Tree();
};

class B_Node{
private:
    B_NodeHeader header;

public:

};



#define ZBASE_B_TREE_H

#endif //ZBASE_B_TREE_H
