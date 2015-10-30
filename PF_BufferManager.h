//
// Created by Orpine on 9/19/15.
//

#ifndef ZBASE_PF_BUFFERMANAGER_H
#define ZBASE_PF_BUFFERMANAGER_H

#include "PF.h"
#include "PF_HashTable.h"
#include <vector>
#include <list>

const int INVALID_SLOT = -1;

class PF_BufferPageDesc {
public:
    ~PF_BufferPageDesc() {
        delete [] data;
    }

    char* data;
    std::list<int>::iterator it;
    bool dirty;
    short pinCount;
    PageNum pageNum;
    FILE* fd;
};


class PF_BufferManager {
public:
    PF_BufferManager(int numPages);
    ~PF_BufferManager();

    RC GetPage(FILE* fd, PageNum pageNum, char* &buffer, bool multiplePins = true);
    RC AllocatePage(FILE* fd, PageNum pageNum, char* &buffer);
    RC MarkDirty(FILE* fd, PageNum pageNum);
    RC UnpinPage(FILE* fd, PageNum pageNum);
    RC ForcePages(FILE* fd, PageNum pageNum);
    RC FlushPages(FILE* fd);

private:
    RC InsertFree(int slot);
    RC MakeMRU(int slot);
    RC LinkHead(int slot);
    RC Unlink(int slot);
    RC InternalAlloc(int &slot);

    RC ReadPage(FILE* fd, PageNum pageNum, char *dest);
    RC WritePage(FILE* fd, PageNum pageNum, char *source);

    RC InitPageDesc(FILE* fd, PageNum pageNum, int slot);


    std::vector<PF_BufferPageDesc> bufTable;
    PF_HashTable hashTable;
    int numPages;
    size_t pageSize;
    std::list<int> free;
    std::list<int> used;
};


#endif //ZBASE_PF_BUFFERMANAGER_H
