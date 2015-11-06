//
// Created by Orpine on 10/25/15.
//

#ifndef ZBASE_QL_H
#define ZBASE_QL_H

#include <vector>
#include <iostream>
#include "zbase.h"
#include "RM.h"
#include "IX.h"
#include "SM.h"
using namespace std;

class Printer {
public:
    Printer(vector<AttrCatRecord> attrs);
    void Print(const char* recordData);

private:
    vector<AttrCatRecord> attrs;
};

class QL_Manager {
public:
    QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
    ~QL_Manager();
    RC Select(vector<AttrInfo> &selectAttrs, const string &relations, const vector<Condition> &conditions);
    RC Insert(const string &relation, const vector<Value> &values);
    RC Delete(const string &relation, vector<Condition> &conditions);
private:

    bool checkUnique(const string &relationName, AttrCatRecord attr, Value value, size_t tupleLength);
    bool MatchConditions(char* recordData, const vector<AttrCatRecord> &attrs, const vector<Condition> &conditions);
    RC ValidateConditions(const vector<AttrCatRecord> &attrs, const vector<Condition> &conditions);

    SM_Manager* smManager;
    IX_Manager* ixManager;
    RM_Manager* rmManager;
};

template<class T>
bool matchRecord(const T &lValue, const T &rValue, CmpOp op);

// warning
const RC QL_INVALID_WHERE_CLAUSE = QL_RC - 1;
const RC QL_INVALID_ATTR_COUNT = QL_RC - 2;
const RC QL_INVALID_ATTR_TYPE = QL_RC - 3;
const RC QL_EOF = QL_RC - 4;
const RC QL_UNIQUE_VALUE_EXISTS = QL_RC - 5;
const RC QL_LENGTH_EXCEED = QL_RC - 6;


#endif //ZBASE_QL_H
