//
// Created by Orpine on 10/25/15.
//

#ifndef ZBASE_SM_H
#define ZBASE_SM_H

#include <vector>
#include <fstream>
#include <map>
#include "IX.h"
#include "RM.h"
#include <iostream>

using namespace std;

const RC SM_NOTFOUND = SM_RC -1;
const RC SM_INDEXEXISTS = SM_RC -2;
const RC SM_INDEXNAMEEXISTS = SM_RC -3;
const RC SM_NOTUNIQUE = SM_RC - 4;
const RC SM_TABLEEXISTS = SM_RC - 5;

class SM_Manager {
private:
    IX_Manager ixm;
    RM_Manager rmm;
    RM_FileHandle relfh;
    RM_FileHandle attrfh;

public:
    SM_Manager(IX_Manager &ixm, RM_Manager &rmm);
    ~SM_Manager();
    RC CreateDb(const string &dbName);
    RC OpenDb(const string &dbName);
    RC CloseDb();
    RC CreateTable(const string &relationName, const vector<AttrInfo> &attrs);
    RC DropTable(const string &relationName);
    RC CreateIndex(const string &relationName, const string &attrName, const string &indexName);
    RC DropIndex(const string &indexName);
    RC DropIndex(const string &relationName, const string &attrName);
    RC GetAttrInfo(const string &relationName, int attrCount, vector<AttrCatRecord> &attrs);
    RC GetAttrInfo(const string &relationName, const string &attrName, AttrCatRecord &attrData);
    RC GetAttrInfo(const string &relationName, const string &attrName, AttrCatRecord &attrData, RID &rid);
    RC GetRelationInfo(const string &relationName, RelationCatRecord &relationData);
    RC GetRelationInfo(const string &relationName, RelationCatRecord &relationData, RID &rid);
};

void mapSave(const map<string, string> &m);
void mapGet(map<string, string> &m);

#endif //ZBASE_SM_H
