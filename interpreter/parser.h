//
// Created by Orpine on 10/27/15.
//

#ifndef ZBASE_PARSER_H
#define ZBASE_PARSER_H

#include <string>
#include <vector>
#include "../zbase.h"

using namespace std;

typedef enum {
    CREATETABLE,
    DROPTABLE,
    CREATEINDEX,
    DROPINDEX,
    SELECT,
    INSERT,
    DELETE
} nodeType;

struct State {
    nodeType type;

    string relationName;
    string indexName;
    string attrName;
    string pAttrName;
    vector<AttrInfo> attrs;
    vector<Value> values;
    vector<Condition> conditions;
};


#endif //ZBASE_PARSER_H
