//
// Created by Orpine on 9/19/15.
//

#ifndef ZBASE_ZBASE_H
#define ZBASE_ZBASE_H

#include <string>
#include "string.h"

using std::string;

#define MAXNAME 255

// return code for traceback
typedef int RC;

const RC RC_OK = 0;

const RC PF_RC = 100;
const RC RM_RC = 200;
const RC IX_RC = 300;
const RC SM_RC = 400;
const RC QL_RC = 500;


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
    size_t attrLength;
    Property property;
    AttrInfo(string n, AttrType t, size_t l, Property p = NONE): attrName(n), attrType(t), attrLength(l), property(p) { }
    AttrInfo(){};
};

struct RelationCatRecordC;
struct AttrCatRecordC;


struct RelationCatRecord {
    string relationName;
    size_t tupleLength; // byte size of tuple, used by Record Manager
    int attrCount;
    int indexCount;
    RelationCatRecord();
    RelationCatRecord(const RelationCatRecordC &x);
};

struct RelationCatRecordC {
    char relationName[MAXNAME];
    size_t tupleLength; // byte size of tuple, used by Record Manager
    int attrCount;
    int indexCount;
    RelationCatRecordC(const RelationCatRecord &x);
};

struct AttrCatRecord {
    string relationName;
    string attrName;
    int offset;
    AttrType attrType;
    size_t attrLength;
    int indexNo;
    Property property;
    int x;
    AttrCatRecord();
    AttrCatRecord(const AttrCatRecordC &x);
};

struct AttrCatRecordC {
    char relationName[MAXNAME];
    char attrName[MAXNAME];
    int offset;
    AttrType attrType;
    size_t attrLength;
    int indexNo;
    Property property;
    int x;
    AttrCatRecordC(const AttrCatRecord &x);
};




struct Value {
    AttrType type;
    int iData;
    float fData;
    string strData;
    Value(){

    }
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
    RelationAttr(){}
    RelationAttr(string x): attrName(x) {}
};

struct Condition {
    RelationAttr lAttr; // left-hand side attr
    CmpOp op;
    Value rValue;
    Condition(){}
    Condition(RelationAttr l, CmpOp m, Value r): lAttr(l), op(m), rValue(r) { }
};


#endif //ZBASE_ZBASE_H
