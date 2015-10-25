//
// Created by Orpine on 9/20/15.
//

#ifndef ZBASE_PF_HASHTABLE_H
#define ZBASE_PF_HASHTABLE_H

#include "PF.h"
#include <vector>
#include <list>

struct PF_HashElement {
    uintptr_t fd;
    PageNum pageNum;
    int slot;
    PF_HashElement(uintptr_t _fd, PageNum _pageNum, int _slot):fd(_fd), pageNum(_pageNum), slot(_slot) {}
};

typedef std::list<PF_HashElement> PF_HashEntry;

class PF_HashTable {
public:
    PF_HashTable(int numBuckets);
    ~PF_HashTable();
    RC Find(uintptr_t fd, PageNum pageNum, int &slot);
    RC Insert(uintptr_t fd, PageNum pageNum, int slot);
    RC Delete(uintptr_t fd, PageNum pageNum);

private:
    int numBuckets;
    std::vector<PF_HashEntry> hashTable;
    int Hash(uintptr_t fd, PageNum pageNum) const {
        return (int)((fd + pageNum) % numBuckets);
    }
};


#endif //ZBASE_PF_HASHTABLE_H
