//
// Created by Orpine on 9/20/15.
//

#include "PF_FileHandle.h"

PF_FileHandle::PF_FileHandle() {
    fileOpen = false;
    bufferManager = NULL;
}

PF_FileHandle::~PF_FileHandle() {

}

PF_FileHandle::PF_FileHandle(const PF_FileHandle &fileHandle) {
    this->bufferManager = fileHandle.bufferManager;
    this->header = fileHandle.header;
    this->fileOpen = fileHandle.fileOpen;
    this->headerChanged = fileHandle.headerChanged;
    this->fp = fileHandle.fp;
}

PF_FileHandle& PF_FileHandle::operator=(const PF_FileHandle &fileHandle) {
    if (this != &fileHandle) {
        this->bufferManager = fileHandle.bufferManager;
        this->header = fileHandle.header;
        this->fileOpen = fileHandle.fileOpen;
        this->headerChanged = fileHandle.headerChanged;
        this->fp = fileHandle.fp;
    }

    return *this;
}

RC PF_FileHandle::GetFirstPage(PF_FileHandle &pageHandle) const {

}


RC PF_FileHandle::GetThisPage(PageNum pageNum, PF_PageHandle &pageHandle) const {
    RC rc;
    char *pageBuf;

    if (!fileOpen) {
        return PF_FILE_CLOSED;
    }

    if (!IsValidPageNum(pageNum)) {
        return PF_INVALID_PAGE;
    }

    if (((PF_PageHeader *)pageBuf)->nextFree ==PF_PAGE_USED) {
        pageHandle.pageNum = pageNum;
        pageHandle.pageData = pageBuf + sizeof(PF_PageHeader);

        return RC_OK;
    }

    if ((rc = UnpinPage(pageNum)) != 0) {
        return rc;
    }

    return PF_INVALID_PAGE;
}


bool PF_FileHandle::IsValidPageNum(PageNum pageNum) const {
    return fileOpen && pageNum >= 0 && pageNum < header.numPages;
}