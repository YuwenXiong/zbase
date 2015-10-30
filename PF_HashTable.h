//
// Created by Orpine on 9/20/15.
//

#ifndef ZBASE_PF_HASHTABLE_H
#define ZBASE_PF_HASHTABLE_H

#include "PF.h"
#include <vector>
#include <list>
#include <unordered_map>
#include <stdlib.h>
#include <map>
using namespace std;

struct PF_HashElement {
    FILE* fd;
    PageNum pageNum;
    int slot;
    PF_HashElement(FILE* _fd, PageNum _pageNum, int _slot):fd(_fd), pageNum(_pageNum), slot(_slot) {}
};

typedef std::list<PF_HashElement> PF_HashEntry;

class PF_HashTable {
public:
    PF_HashTable(int numBuckets);
    ~PF_HashTable();
    RC Find(FILE* fd, PageNum pageNum, int &slot);
    RC Insert(FILE* fd, PageNum pageNum, int slot);
    RC Delete(FILE* fd, PageNum pageNum);

private:
    int numBuckets;
    std::vector<PF_HashEntry> hashTable;
//    std::map<unsigned long, int> fdm;
    int Hash(FILE* fd, PageNum pageNum) const {
//        if (fdm.find((unsigned long)fd & 0xffff) == fdm.end()) {
//            fdm[(unsigned long)fd & 0xffff] = rand() % numBuckets;
////            fdm.insert(std::make_pair(, ));
//        }
        return (int)(((unsigned long)fd & 0xffff) + pageNum) % numBuckets;
    }
};


#endif //ZBASE_PF_HASHTABLE_H
