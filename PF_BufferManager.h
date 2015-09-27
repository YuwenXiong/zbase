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
//    int next;
//    int prev;
    std::list<int>::iterator it;
    bool dirty;
    short pinCount;
    PageNum pageNum;
    int fd;
};


class PF_BufferManager {
public:
    PF_BufferManager(int numPages);
    ~PF_BufferManager();

    RC GetPage(int fd, PageNum pageNum, char *&buffer, bool multiplePins = true);
    RC AlloatePage(int fd, PageNum pageNum, char *&buffer);
    RC MarkDirty(int fd, PageNum pageNum);
    RC UnpinPage(int fd, PageNum pageNum);
    RC FlushPages(int fd);
    RC ForcePages(int fd, PageNum pageNum);
    RC ClearBuffer();
    RC PrintBuffer();
    RC ResizeBuffer(int newSize);
    RC GetBlockSize(int &length) const;
    RC AllocateBlock(std::vector<char> &buffer);
    RC DisposeBlock(char *buffer);

private:
    RC InsertFree(int slot);
    RC MakeMRU(int slot);
    RC LinkHead(int slot);
    RC Unlink(int slot);
    RC InternalAlloc(int &slot);

    RC ReadPage(int fd, PageNum pageNum, char *dest);
    RC WritePage(int fd, PageNum pageNum, char *source);

    RC InitPageDesc(int fd, PageNum pageNum, int slot);


    std::vector<PF_BufferPageDesc> bufTable;
    PF_HashTable hashTable;
    int numPages;
    size_t pageSize;
//    int first;  // MRU page slot
//    int last;   // LRU page slot
//    int free;   // head of free list
    std::list<int> free;
    std::list<int> used;
};


#endif //ZBASE_PF_BUFFERMANAGER_H
