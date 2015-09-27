//
// Created by Orpine on 9/19/15.
//

#ifndef ZBASE_PF_H
#define ZBASE_PF_H

#include <stddef.h>
#include <stdio.h>
#include "zbase.h"

typedef int PageNum;

struct PF_PageHeader {
    int nextFree;
};

struct PF_FileHeader {
    int firstFree;
    int numPages;
};

// Page size is 4k bytes - sizeof(pageHeader)
// pageHeader is a integer
const int PF_PAGE_SIZE = 4096 - sizeof(PF_PageHeader);

const size_t PF_FILE_HEADER_SIZE = PF_PAGE_SIZE + sizeof(PF_PageHeader);

// bufferSize is the number of pages in buffer
const int PF_BUFFER_SIZE = 40;
// hashTableSize is the size of hash table;
const int PF_HASH_TABLE_SIZE = 20;

const int PF_PAGE_LIST_END = -1;


class PF_BufferManager;

class PF_Manager {
public:
    PF_Manager();
    ~PF_Manager();
    RC CreateFile(const char *fileName);
    RC DestoryFile(const char *fileName);
    RC OpenFile(const char *fileName, PF_FileHandle & fileHandle);

    RC CloseFile(PF_FileHandle &fileHandle);
    RC AllocateBlock(char *&buffer);
    RC DisposeBlock(char *buffer);
private:
    PF_BufferManager *bufferManager;
};

class PF_FileHandle {
    friend class PF_Manager;
public:
    PF_FileHandle();
    ~PF_FileHandle();
    PF_FileHandle (const PF_FileHandle &fileHandle);
    PF_FileHandle& operator= (const PF_FileHandle &fileHandle);

    RC GetFirstPage(PF_FileHandle &pageHandle) const;
    RC GetLastPage(PF_FileHandle &pageHandle) const;

    RC GetNextPage(PageNum current, PF_PageHandle &pageHandle) const;
    RC GetPrevPage(PageNum current, PF_PageHandle &pageHandle) const;
    RC GetThisPage(PageNum pageNum, PF_PageHandle &pageHandle) const;

    RC AllocatePage(PF_PageHandle &pageHandle);
    RC DisposePage(PageNum pageNum);
    RC MarkDirty(PageNum  pageNum) const;
    RC  UnpinPage(PageNum pageNum) const;
    RC ForcePages(PageNum pageNum = ALL_PAGES) const;

private:

    bool IsValidPageNum(PageNum pageNum) const;

    PF_BufferManager *bufferManager;
    PF_FileHeader header;
    bool fileOpen;
    bool headerChanged;
    FILE *fp;

};

class PF_PageHandle {
    friend class PF_FileHandle;
public:
    PF_PageHandle();
    ~PF_PageHandle();
    PF_PageHandle(const PF_PageHandle &pageHandle);
    PF_PageHandle& operator= (const PF_PageHandle &pageHandle);

    RC GetData(char *&data) const;
    RC GetPageNum(PageNum &pageNum) const;

private:
    int pageNum;
    char *pageData;
};

// Warning
const RC PF_FILE_ALREADY_OPEN = PF_RC - 1;
const RC PF_FILE_ALREADY_CLOSE = PF_RC - 2;
const RC PF_PAGE_NOT_IN_BUFFER = PF_RC - 3;
const RC PF_PAGE_UNPINNED = PF_RC - 4;
const RC PF_PAGE_PINNED = PF_RC - 5;


// Error
const RC PF_SYSTEM_ERROR = PF_RC + 1;
const RC PF_HASH_NOT_FOUND = PF_RC + 2;
const RC PF_PAGE_IN_BUFFER = PF_RC + 3;
const RC PF_NO_BUFFER = PF_RC + 4;
const RC PF_READ_ERROR = PF_RC + 5;
const RC PF_WRITE_ERROR = PF_RC + 6;
const RC PF_HASH_PAGE_EXIST = PF_RC + 7;


#endif //ZBASE_PF_H
