//
// Created by Orpine on 10/25/15.
//

#ifndef ZBASE_QL_H
#define ZBASE_QL_H

#include <vector>
#include "zbase.h"
#include "RM.h"
#include "IX.h"
#include "SM.h"

class QL_Manager {
public:
    QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
    ~QL_Manager();
    RC Select(vector<RelationAttr> &selectAttrs, const vector<string> &relations, const vector<Condition> &conditions);
    RC Insert(const string &relation, const vector<Value> values);
    RC Delete(const string &relation, const vector<Condition> conditions);
private:

    bool MatchConditions(char* recordData, const vector<AttrCatRecord> &attrs, const vector<Condition> &conditions);
    template<class T>
    bool matchRecord(const T &lValue, const T &rValue, CmpOp op);
    RC ValidateConditions(const vector<AttrCatRecord> &attrs, const vector<Condition> &conditions);

    SM_Manager* smManager;
    IX_Manager* ixManager;
    RM_Manager* rmManager;
};


// warning
const RC QL_INVALID_WHERE_CLAUSE = QL_RC - 1;
const RC QL_INVALID_ATTR_COUNT = QL_RC - 2;
const RC QL_INVALID_ATTR_TYPE = QL_RC - 3;
const RC QL_EOF = QL_RC - 4;


#endif //ZBASE_QL_H
