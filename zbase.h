//
// Created by Orpine on 9/19/15.
//

#ifndef ZBASE_ZBASE_H
#define ZBASE_ZBASE_H

#include <string>
#include "string.h"

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
    EQ, NE, NO, LT, LE, GT, GE
};

enum Property {
    PRIMARY, UNIQUE, NONE
};


struct AttrInfo {
    string attrName;
    AttrType attrType;
    int attrLength;
    Property property;
    AttrInfo(string n, AttrType t, int l, Property p = NONE): attrName(n), attrType(t), attrLength(l), property(p) { }
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
    Property property;
};

struct Value {
    AttrType type;
    int iData;
    float fData;
    string strData;
    Value(int x): iData(x) {
        type = INT;
    }
    Value(float x): fData(x) {
        type = FLOAT;
    }
    Value(string x): strData(x) {
        type = CHARN;
    }
};

struct RelationAttr {
    string relationName;
    string attrName;
};

struct Condition {
    string lAttr; // left-hand side attr
    CmpOp op;
    Value rValue;
    Condition(string l, CmpOp m, Value r): lAttr(l), op(m), rValue(r) { }
};


#endif //ZBASE_ZBASE_H
