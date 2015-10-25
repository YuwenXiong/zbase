//
// Created by Orpine on 10/25/15.
//

#ifndef ZBASE_SM_H
#define ZBASE_SM_H

#include <vector>
#include "IX.h"
#include "RM.h"
using namespace std;

class SM_Manager {
public:
    SM_Manager(IX_Manager &ixm, RM_Manager &rmm);
    ~SM_Manager();
    RC OpenDb(const string &dbName);
    RC CloseDb();
    RC CreateTable(const string &relationName, const vector<AttrInfo> &attrs);
    RC DropTable(const string &relationName);
    RC CreateIndex(const string &relationName);
    RC DropIndex(const string &relationName);
    RC GetAttrInfo(const string &relationName, int attrCount, AttrCatRecord &attrData);
    RC GetAttrInfo(const string &relationName, const string &attrName, AttrCatRecord &attrData);
    RC GetRelationInfo(const string &relationName, RelationCatRecord &relationData);
};


#endif //ZBASE_SM_H
