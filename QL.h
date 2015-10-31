//
// Created by Orpine on 10/25/15.
//

#ifndef ZBASE_QL_H
#define ZBASE_QL_H

#include "zbase.h"

class QL_Manager {
public:
    QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
    ~QL_Manager();
    RC Select(const vector<RelationAttr> &selectAttrs, const vector<string> &relations, const vector<Condition> &conditions);
    RC Insert(const string &relation, const vector<Value> values);
    RC Delete(const string &relation, const vector<Condition> conditions);
private:
    SM_Manager* smManager;
    IX_Manager* ixManager;
    RM_Manager* rmManager;
};



#endif //ZBASE_QL_H
