//
// Created by Orpine on 9/20/15.
//

#include "PF_HashTable.h"

PF_HashTable::PF_HashTable(int _numBuckets) {
    numBuckets = _numBuckets;
    hashTable.resize((size_t)numBuckets);
}

PF_HashTable::~PF_HashTable() {
}

RC PF_HashTable::Find(int fd, PageNum pageNum, int &slot) {
    int bucket = Hash(fd, pageNum);

    if (bucket < 0) {
        return PF_HASH_NOT_FOUND;
    }

    for (const auto &entry: hashTable[bucket]) {
        if (entry.fd == fd && entry.pageNum == pageNum) {
            slot = entry.slot;
            return 0;
        }
    }
    return PF_HASH_NOT_FOUND;
}

RC PF_HashTable::Insert(int fd, PageNum pageNum, int slot) {
    int bucket = Hash(fd, pageNum);
    for (const auto &entry: hashTable[bucket]) {
        if (entry.fd == fd && entry.pageNum == pageNum) {
            return PF_HASH_PAGE_EXIST;
        }
    }
    hashTable[bucket].push_front(PF_HashElement(fd, pageNum, slot));
    return 0;
}

RC PF_HashTable::Delete(int fd, PageNum pageNum) {
    int bucket = Hash(fd, pageNum);

    for (auto entry = hashTable[bucket].begin(); entry != hashTable[bucket].end(); entry++) {
        if (entry->fd == fd && entry->pageNum == pageNum) {
            hashTable[bucket].erase(entry);
            return 0;
        }
    }
    return PF_HASH_NOT_FOUND;
}