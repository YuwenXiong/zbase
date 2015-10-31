//
// Created by Orpine on 10/31/15.
//

#include "QL.h"
#include "SM.h"

// actually we would query all attrs and only one relation
RC QL_Manager::Select(const vector <RelationAttr> &selectAttrs, const vector <string> &relations,
                      const vector <Condition> &conditions) {
    RC rc;
    RelationCatRecord rcRecord;
    AttrInfo attr;
    int attrCount;
    if ((rc = smManager->GetRelationInfo(relations[0], rcRecord))) {
        return rc;
    }
    attrCount = rcRecord.attrCount;
    vector<AttrCatRecord> attrs;
    if ((rc = smManager->GetAttrInfo(relations[0], attrCount, attrs))) {
        return rc;
    }

    if ((rc = ValidateConditions(rcRecord, conditions))) {
        return QL_WHERE_CLAUSE_ERROR;
    }
    Condition cconditions(conditions);

}