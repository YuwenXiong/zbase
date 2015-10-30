//
// Created by Orpine on 9/19/15.
//

#include "PF_BufferManager.h"

#include <iostream>
#include <unistd.h>

PF_BufferManager::PF_BufferManager(int _numPages): hashTable(PF_HASH_TABLE_SIZE) {
    numPages = _numPages;
    pageSize = PF_PAGE_SIZE + sizeof(PF_PageHeader);


    bufTable.resize((size_t)numPages);
    for(int i = 0; i < numPages; i++) {
        if ((bufTable[i].data = new char[pageSize]) == NULL) {
            std::cerr << "Not enough memory" << std::endl;
            exit(1);
        }
        memset(bufTable[i].data, 0, sizeof pageSize);
        bufTable[i].it = free.insert(free.end(), i);
    }
}

PF_BufferManager::~PF_BufferManager() {
}

RC PF_BufferManager::GetPage(FILE* fd, PageNum pageNum, char* &buffer, bool multiplePins) {
    RC rc;
    int slot;
    if (((rc = hashTable.Find(fd, pageNum, slot)) != 0) && (rc != PF_HASH_NOT_FOUND)) {
        return rc;
    }

    if (rc == PF_HASH_NOT_FOUND) {
        if ((rc = InternalAlloc(slot)) != 0) {
            return rc;
        }

        if ((rc = ReadPage(fd, pageNum, bufTable[slot].data)) ||
            (rc = hashTable.Insert(fd, pageNum, slot)) ||
            (rc = InitPageDesc(fd, pageNum, slot))) {
            Unlink(slot);
            InsertFree(slot);
            return rc;
        }
    } else {
        if (!multiplePins && bufTable[slot].pinCount > 0) {
            return PF_PAGE_PINNED;
        }

        bufTable[slot].pinCount++;
        if ((rc = Unlink(slot)) != 0 ||
            (rc = LinkHead(slot)) != 0) {
            return rc;
        }
    }
    buffer = bufTable[slot].data;
    return RC_OK;
}

RC PF_BufferManager::AllocatePage(FILE* fd, PageNum pageNum, char* &buffer) {
    RC rc;
    int slot;

    if ((rc = hashTable.Find(fd, pageNum, slot)) == 0) {
        return PF_PAGE_IN_BUFFER;
    } else if (rc != PF_HASH_NOT_FOUND) {
        return rc;
    }

    if ((rc = InternalAlloc(slot)) != 0) {
        return rc;
    }

    if ((rc = hashTable.Insert(fd, pageNum, slot)) != 0 || (rc = InitPageDesc(fd, pageNum, slot)) != 0) {
        Unlink(slot);
        InsertFree(slot);
        return rc;
    }

    buffer = bufTable[slot].data;
    return RC_OK;
}

RC PF_BufferManager::MarkDirty(FILE* fd, PageNum pageNum) {
    RC rc;
    int slot;

    if ((rc = hashTable.Find(fd, pageNum, slot)) != 0) {
        return rc == PF_HASH_NOT_FOUND ? PF_PAGE_NOT_IN_BUFFER : rc;
    }

    if (bufTable[slot].pinCount == 0) {
        return PF_PAGE_UNPINNED;
    }

    bufTable[slot].dirty = true;

    if ((rc = MakeMRU(slot)) != 0) {
        return rc;
    }

    return RC_OK;
}

RC PF_BufferManager::UnpinPage(FILE* fd, PageNum pageNum) {
    RC rc;
    int slot;

    if ((rc = hashTable.Find(fd, pageNum, slot)) != 0) {
        return rc == PF_HASH_NOT_FOUND ? PF_PAGE_NOT_IN_BUFFER : rc;
    }
    if (bufTable[slot].pinCount == 0) {
        return PF_PAGE_UNPINNED;
    }
    if (--bufTable[slot].pinCount == 0) {
        if ((rc = MakeMRU(slot)) != 0) {
            return rc;
        }
    }

    return RC_OK;
}

RC PF_BufferManager::MakeMRU(int slot) {
    RC rc;
    if ((rc = Unlink(slot)) != 0 || (rc = LinkHead(slot)) != 0) {
        return rc;
    }
    return RC_OK;
}

RC PF_BufferManager::ForcePages(FILE* fd, PageNum pageNum) {
    RC rc;

    for (const auto &slot: used) {
        if (bufTable[slot].fd == fd && (pageNum == ALL_PAGES || bufTable[slot].pageNum == pageNum)) {
            if (bufTable[slot].dirty) {
                if ((rc = WritePage(fd, bufTable[slot].pageNum, bufTable[slot].data)) != 0) {
                    return rc;
                }
                bufTable[slot].dirty = false;
            }
        }
    }
    return RC_OK;
}

RC PF_BufferManager::FlushPages(FILE *fd) {
    RC rc, rcWarn = RC_OK;
    for (auto slot = used.begin(), next = used.begin(); slot != used.end(); slot = next) {
        next = slot;
        next++;
        if (bufTable[*slot].fd == fd) {
            if (bufTable[*slot].pinCount) {
                rcWarn = PF_PAGE_PINNED;
            } else {
                if (bufTable[*slot].dirty) {
                    if ((rc = WritePage(fd, bufTable[*slot].pageNum, bufTable[*slot].data))) {
                        return rc;
                    }
                    bufTable[*slot].dirty = false;
                }
                if ((rc = hashTable.Delete(fd, bufTable[*slot].pageNum)) || (rc = Unlink(*slot)) || (rc = InsertFree(*slot))) {
                    return rc;
                }
            }
        }
    }
    return rcWarn;
}

RC PF_BufferManager::InsertFree(int slot) {
    bufTable[slot].it = free.insert(free.begin(), slot);
    return RC_OK;
}

RC PF_BufferManager::LinkHead(int slot) {
    bufTable[slot].it = used.insert(used.begin(), slot);
    return RC_OK;
}


RC PF_BufferManager::Unlink(int slot) {
    used.erase(bufTable[slot].it);
    return RC_OK;
}

RC PF_BufferManager::InternalAlloc(int &slot) {
    RC rc;
    slot = INVALID_SLOT;
    if (!free.empty()) {
        slot = *(free.begin());
        free.pop_front();
    } else {
        for (auto it = used.rbegin(); it != used.rend(); it++) {
            if (bufTable[*it].pinCount == 0) {
                slot = *it;
                break;
            }
        }
        if (slot == -1) {
            return PF_NO_BUFFER;
        }
        if (bufTable[slot].dirty) {
            if ((rc = WritePage(bufTable[slot].fd, bufTable[slot].pageNum, bufTable[slot].data)) != 0) {
                return rc;
            }
            bufTable[slot].dirty = false;
        }

        if ((rc = hashTable.Delete(bufTable[slot].fd, bufTable[slot].pageNum)) != 0 ||
            (rc = Unlink(slot)) != 0) {
            return rc;
        }

    }
    if ((rc = LinkHead(slot)) != 0) {
        return rc;
    }
    return RC_OK;

}

RC PF_BufferManager::ReadPage(FILE* fd, PageNum pageNum, char *dest) {
    long offset = pageNum * pageSize + PF_FILE_HEADER_SIZE;
    if (fseek(fd, offset, SEEK_SET)) {
        return PF_SYSTEM_ERROR;
    }
    ssize_t numByte = fread(dest, pageSize, 1, fd);
    if (numByte < 0) {
        return PF_SYSTEM_ERROR;
    } else if (numByte != 1) {
        return PF_READ_ERROR;
    } else {
        return RC_OK;
    }
}

RC PF_BufferManager::WritePage(FILE* fd, PageNum pageNum, char *source) {
    long offset = pageNum * pageSize + PF_FILE_HEADER_SIZE;
    if (fseek(fd, offset, SEEK_SET)) {
        return PF_SYSTEM_ERROR;
    }
    ssize_t numByte = fwrite(source, pageSize, 1, fd);
    if (numByte < 0) {
        return PF_SYSTEM_ERROR;
    } else if (numByte != 1) {
        return PF_WRITE_ERROR;
    } else {
        return RC_OK;
    }
}

RC PF_BufferManager::InitPageDesc(FILE* fd, PageNum pageNum, int slot) {
    bufTable[slot].fd = fd;
    bufTable[slot].pageNum = pageNum;
    bufTable[slot].pinCount = 1;
    bufTable[slot].dirty = false;
    return RC_OK;
}

