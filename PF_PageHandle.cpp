//
// Created by Orpine on 9/20/15.
//

#include "PF_PageHandle.h"

const int INVALUD_PAGE = -1;

PF_PageHandle::PF_PageHandle() {
    pageNum = INVALUD_PAGE;
    pageData = NULL;
}

PF_PageHandle::~PF_PageHandle() {

}

PF_PageHandle::PF_PageHandle(const PF_PageHandle &pageHandle) {
    this->pageNum = pageHandle.pageNum;
    this->pageData = pageHandle.pageData;
}

PF_PageHandle& PF_PageHandle::operator=(const PF_PageHandle &pageHandle) {
    if (this != &pageHandle) {
        this->pageNum = pageHandle.pageNum;
        this->pageData = pageHandle.pageData;
    }

    return *this;
}

RC PF_PageHandle::GetData(char *&data) const {
    if (pageData == NULL) {
        return PF_PAGE_UNPINNED;
    }

    data = pageData;
    return RC_OK;
}

RC PF_PageHandle::GetPageNum(PageNum &_pageNum) const {
    if (pageData == NULL) {
        return PF_PAGE_UNPINNED;
    }
    _pageNum = this->pageNum;

    return RC_OK;
}