//
// Created by Orpine on 9/20/15.
//

#ifndef ZBASE_PF_HASHTABLE_H
#define ZBASE_PF_HASHTABLE_H

#include "PF.h"
#include <vector>
#include <list>

struct PF_HashElement {
//    PF_HashEntry *next;
//    PF_HashEntry *prev;
    int fd;
    PageNum pageNum;
    int slot;
    PF_HashElement(int _fd, PageNum _pageNum, int _slot):fd(_fd), pageNum(_pageNum), slot(_slot) {}
};

typedef std::list<PF_HashElement> PF_HashEntry;

class PF_HashTable {
public:
    PF_HashTable(int numBuckets);
    ~PF_HashTable();
    RC Find(int fd, PageNum pageNum, int &slot);
    RC Insert(int fd, PageNum pageNum, int slot);
    RC Delete(int fd, PageNum pageNum);

private:
    int numBuckets;
    std::vector<PF_HashEntry> hashTable;
    int Hash(int fd, PageNum pageNum) const {
        return (fd + pageNum) % numBuckets;
    }
};


#endif //ZBASE_PF_HASHTABLE_H
