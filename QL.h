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
    RC Select(const vector<RelationAttr> &selectAttrs, const vector<string> &relations, const vector<Condition> &conditions);
    RC Insert(const string &relation, const vector<Value> values);
    RC Delete(const string &relation, const vector<Condition> conditions);
private:

    bool MatchConditions(char* recordData, const vector<AttrCatRecord> &attrs, const vector<Condition> &conditions);

    SM_Manager* smManager;
    IX_Manager* ixManager;
    RM_Manager* rmManager;
};


// warning
const RC QL_WHERE_CLAUSE_ERROR = QL_RC - 1;
const RC QL_ATTR_COUNT_ERROR = QL_RC - 2;
const RC QL_ATTR_TYPE_ERROR = QL_RC - 3;

#endif //ZBASE_QL_H
