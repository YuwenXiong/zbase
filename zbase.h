//
// Created by Orpine on 9/19/15.
//

#ifndef ZBASE_ZBASE_H
#define ZBASE_ZBASE_H

#include <string>
using std::string

// return code for traceback
typedef int RC;

const RC RC_OK = 0;

const RC PF_RC = 100;

const int ALL_PAGES = -1;

enum AttrType {
    INT,
    FLOAT,
    CHARN
};

enum CmpOp {
    EQ, NE
};

struct AttrInfo {
    string attrName;
    AttrType attrType;
    int attrLength;
};

struct RelationCatRecord {
    string relationName;
    int tupleLength; // byte size of tuple, used by Record Manager
    int attrCount;
    int indexCount;
};

struct AttrCatRecord {
    string relationName;
    string attrName;
    int offset;
    AttrType attrType;
    int attrLength;
    int indexNo;
};

#endif //ZBASE_ZBASE_H
