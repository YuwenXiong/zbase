//
// Created by Orpine on 9/19/15.
//

#include <cstdio>
#include <string.h>
#include <unistd.h>
#include "PF.h"
#include "PF_BufferManager.h"


PF_Manager::PF_Manager() {
    bufferManager = new PF_BufferManager(PF_BUFFER_SIZE);
}

PF_Manager::~PF_Manager() {
    delete bufferManager;
}

RC PF_Manager::CreateFile(const char *fileName) {
    FILE *fid; // file identifier, we don't expose fd to avoid portability problem

    if ((fid = fopen(fileName, "w")) == NULL) {
        return PF_SYSTEM_ERROR;
    }

    PF_FileHeader fileHeader;
    fileHeader.firstFree = PF_PAGE_LIST_END;
    fileHeader.numPages = 0;

    size_t numBytes = fwrite(&fileHeader, sizeof(fileHeader), 1, fid);
    if (numBytes != 1) {
        fclose(fid);
        unlink(fileName);
        return PF_SYSTEM_ERROR;
    }

    if (fclose(fid) != 0) {
        return PF_SYSTEM_ERROR;
    }

    return RC_OK;
}

RC PF_Manager::DestroyFile(const char *fileName) {
    if (unlink(fileName) < 0) {
        return PF_SYSTEM_ERROR;
    }
    return RC_OK;
}

RC PF_Manager::OpenFile(const char *fileName, PF_FileHandle &fileHandle) {
    RC rc;
    if (fileHandle.fileOpen) {
        return PF_FILE_ALREADY_OPEN;
    }
    if ((fileHandle.fp = fopen(fileName, "r+")) == NULL) {
        return PF_SYSTEM_ERROR;
    }
    size_t numBytes = fread(&(fileHandle.header), sizeof(PF_FileHeader), 1, fileHandle.fp);
    if (numBytes != 1) {
        rc = PF_SYSTEM_ERROR;
        fclose(fileHandle.fp);
        fileHandle.fileOpen = false;
        return rc;
    }

    fileHandle.headerChanged = false;
    fileHandle.bufferManager = this->bufferManager;
    fileHandle.fileOpen = true;

    return RC_OK;
}

RC PF_Manager::CloseFile(PF_FileHandle &fileHandle) {
    RC rc;
    if (!fileHandle.fileOpen) {
        return PF_FILE_ALREADY_CLOSE;
    }
    if ((rc = fileHandle.FlushPages()) != 0) {
        return rc;
    }
    if (fclose(fileHandle.fp) != 0) {
        return PF_SYSTEM_ERROR;
    }
    fileHandle.fileOpen = false;
    fileHandle.bufferManager = NULL;
    return RC_OK;
}
