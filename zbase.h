//
// Created by Orpine on 9/19/15.
//

#ifndef ZBASE_ZBASE_H
#define ZBASE_ZBASE_H

#include <string>
#include "PF.h"

using std::string;

#define MAXRELATIONNAME 255

// return code for traceback
typedef int RC;

const RC RC_OK = 0;

const RC PF_RC = 100;
const RC RM_RC =200;

const int ALL_PAGES = -1;
const int NULL_PAGE=-2;
const int NULL_SLOT=-2;

enum AttrType {
    INT,
    FLOAT,
    CHARN
};

enum CmpOp {
    EQ, NE, NO
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
