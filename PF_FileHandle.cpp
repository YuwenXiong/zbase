//
// Created by Orpine on 9/20/15.
//

#include "PF.h"
#include "PF_BufferManager.h"

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

RC PF_FileHandle::GetThisPage(PageNum pageNum, PF_PageHandle &pageHandle) const {
    RC rc;
    char *pageBuf;

    if (!fileOpen) {
        return PF_FILE_CLOSED;
    }

    if (!IsValidPageNum(pageNum)) {
        return PF_INVALID_PAGE;
    }

    if ((rc = bufferManager->GetPage(fp, pageNum, pageBuf))) {
        return rc;
    }

    if (((PF_PageHeader *)pageBuf)->nextFree != PF_PAGE_USED) {
        if ((rc = UnpinPage(pageNum)) != 0) {
            return rc;
        } else {
            return PF_INVALID_PAGE;
        }
    }
    pageHandle.pageNum = pageNum;
    pageHandle.pageData = pageBuf + sizeof(PF_PageHeader);
    return RC_OK;
}

//RC PF_FileHandle::GetNextPage(PageNum current, PF_PageHandle &pageHandle) const {
//    int rc;
//    if (!fileOpen) {
//        return PF_FILE_CLOSED;
//    }
//    // here we cannot check current+1 directly because we throw PF_INVALID_PAGE
//    // current = -1 means we want to get the first page
//    if (current != -1 && !IsValidPageNum(current)) {
//        return PF_INVALID_PAGE;
//    }
//    for (current++; current < header.numPages; current++) {
//        if (!(rc = GetThisPage(current, pageHandle))) {
//            return RC_OK;
//        }
//
//        if (rc != PF_INVALID_PAGE) {
//            return rc;
//        }
//    }
//    return PF_EOF;
//}
//
//RC PF_FileHandle::GetPrevPage(PageNum current, PF_PageHandle &pageHandle) const {
//    int rc;
//    if (!fileOpen) {
//        return PF_FILE_CLOSED;
//    }
//    // here we cannot check current-1 directly because we throw PF_INVALID_PAGE
//    // current = header.numPages means we want to get the last page
//    if (current != header.numPages) {
//        return PF_INVALID_PAGE;
//    }
//    for (current--; current >= 0; current--) {
//        if (!(rc = GetThisPage(current, pageHandle))) {
//            return RC_OK;
//        }
//
//        if (rc != PF_INVALID_PAGE) {
//            return rc;
//        }
//    }
//    return PF_EOF;
//}
//
//RC PF_FileHandle::GetFirstPage(PF_PageHandle &pageHandle) const {
//    return GetNextPage(-1, pageHandle);
//}
//
//RC PF_FileHandle::GetLastPage(PF_PageHandle &pageHandle) const {
//    return GetPrevPage(header.numPages, pageHandle);
//}

RC PF_FileHandle::AllocatePage(PF_PageHandle &pageHandle) {
    int rc;
    int pageNum;
    char *pageBuffer;

    if (!fileOpen) {
        return PF_FILE_CLOSED;
    }

    if (header.firstFree != PF_PAGE_LIST_END) {
        pageNum = header.firstFree;
        if ((rc = bufferManager->GetPage(fp, pageNum, pageBuffer))) {
            return rc;
        }
        header.firstFree = ((PF_PageHeader*)pageBuffer)->nextFree;
    } else {
        pageNum = header.numPages;
        if ((rc = bufferManager->AllocatePage(fp, pageNum, pageBuffer))) {
            return rc;
        }
        header.numPages++;
    }
    headerChanged = true;
    ((PF_PageHeader *)pageBuffer)->nextFree = PF_PAGE_USED;
    memset(pageBuffer + sizeof(PF_PageHeader), 0, PF_PAGE_SIZE);

    if ((rc = MarkDirty(pageNum))) {
        return rc;
    }

    pageHandle.pageNum = pageNum;
    pageHandle.pageData = pageBuffer + sizeof(PF_PageHeader);

    return RC_OK;
}

bool PF_FileHandle::IsValidPageNum(PageNum pageNum) const {
    return fileOpen && pageNum >= 0 && pageNum < header.numPages;
}

RC PF_FileHandle::DisposePage(PageNum pageNum) {
    int rc;
    char *pageBuffer;
    if (!fileOpen) {
        return PF_FILE_CLOSED;
    }

    if (!IsValidPageNum(pageNum)) {
        return PF_INVALID_PAGE;
    }

    if ((rc = bufferManager->GetPage(fp, pageNum, pageBuffer, false))) {
        return rc;
    }

    if (((PF_PageHeader *)pageBuffer)->nextFree != PF_PAGE_USED) {
        if ((rc = UnpinPage(pageNum))) {
            return rc;
        }
        return PF_PAGE_FREE;
    }

    ((PF_PageHeader *)pageBuffer)->nextFree = header.firstFree;
    header.firstFree = pageNum;
    headerChanged = true;

    if ((rc = MarkDirty(pageNum)) || (rc = UnpinPage(pageNum))) {
        return rc;
    }
    return RC_OK;
}

RC PF_FileHandle::MarkDirty(PageNum pageNum) const {
    if (!fileOpen) {
        return PF_FILE_CLOSED;
    }
    if (!IsValidPageNum(pageNum)) {
        return PF_INVALID_PAGE;
    }
    return bufferManager->MarkDirty(fp, pageNum);
}

RC PF_FileHandle::UnpinPage(PageNum pageNum) const {
    if (!fileOpen) {
        return PF_FILE_CLOSED;
    }
    if (!IsValidPageNum(pageNum)) {
        return PF_INVALID_PAGE;
    }
    return bufferManager->UnpinPage(fp, pageNum);
}

RC PF_FileHandle::ForcePages(PageNum pageNum) {
    if (!fileOpen) {
        return PF_FILE_CLOSED;
    }

    if (headerChanged) {
        if (fseek(fp, 0, SEEK_SET) < 0) {
            return PF_SYSTEM_ERROR;
        }
        if (fwrite(&header, sizeof(PF_FileHeader), 1, fp) != 1) {
            return PF_WRITE_ERROR;
        }
        headerChanged = false;
    }
    return bufferManager->ForcePages(fp, pageNum);

}

RC PF_FileHandle::FlushPages() {
    if (!fileOpen) {
        return PF_FILE_CLOSED;
    }
    if (headerChanged) {
        if (fseek(fp, 0, SEEK_SET) < 0) {
            return PF_SYSTEM_ERROR;
        }
        if (fwrite(&header, sizeof(PF_FileHeader), 1, fp) != 1) {
            return PF_WRITE_ERROR;
        }
        headerChanged = false;
    }
    return bufferManager->FlushPages(fp);

}