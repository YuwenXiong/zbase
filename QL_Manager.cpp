//
// Created by Orpine on 10/31/15.
//

#include <assert.h>
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


}

RC QL_Manager::Insert(const string &relation, const vector<Value> values) {
    RC rc;
    RelationCatRecord rcRecord;

    if ((rc = smManager->GetAttrInfo(relation, rcRecord))) {
        return rc;
    }

    vector<AttrCatRecord> attrs;
    if ((rc = smManager->GetAttrInfo(relation, rcRecord.attrCount, attrs))) {
        return rc;
    }

    char* tupleData = new char[rcRecord.tupleLength];
    memset(tupleData, 0, sizeof(char) * rcRecord.tupleLength);

    if (values.size() != rcRecord.tupleLength) {
        return QL_ATTR_COUNT_ERROR;
    }

    for (int i = 0; i < values.size(); i++) {
        if (values[i].type != attrs[i].attrType) {
            return QL_ATTR_TYPE_ERROR;
        }
    }

    RM_FileHandle rmFH;
    RID rid;

    if ((rc = rmManager->OpenFile(relation, rmFH))) {
        return rc;
    }

    IX_IndexHandle *ixIH = new IX_IndexHandle[rcRecord.attrCount];
    if (rcRecord.indexCount > 0) {
        int now = 0;
        for (int i = 0; i < rcRecord.attrCount; i++) {
            if (attrs[i].indexNo != -1) {
                if((rc = ixManager->OpenIndex(relation, attrs[i].indexNo, ixIH[now]))) {
                    return rc;
                }
            }
            now++;
        }
    }

    for (int i = 0; i < rcRecord.attrCount; i++) {
        switch (attrs[i].attrType) {
            case INT:
                memcpy(tupleData + attrs[i].offset, &values[i].iData, attrs[i].attrLength);
                break;
            case FLOAT:
                memcpy(tupleData + attrs[i].offset, &values[i].fData, attrs[i].attrLength);
                break;
            case CHARN:
                char *value = new char[attrs[i].attrLength];
                memset(value, 0, sizeof value);
                strcpy(value, values[i].strData.c_str());
                memcpy(tupleData + attrs[i].offset, value, attrs[i].attrLength);
                break;
            default:
                assert(0);
        }
    }

    if ((rc = rmFH.InsertRecord(tupleData, rid))) {
        return rc;
    }

    int now = 0;
    for (int i = 0; i < rcRecord.attrCount; i++) {
        if (attrs[i].indexNo != -1) {
            if ((rc = ixIH[now].InsertEntry(values[i], rid))) {
                return rc;
            }
            now++;
        }
    }

    if ((rc = rmManager->CloseFile(rmFH))) {
        return rc;
    }
    if (rcRecord.indexCount > 0) {
        for (int i = 0; i < rcRecord.indexCount; i++) {
            if ((rc = ixManager->CloseIndex(ixIH[i]))) {
                return rc;
            }
        }
    }
    delete[] ixIH;
    delete[] tupleData;

    return RC_OK;
}

RC QL_Manager::Delete(const string &relation, const vector<Condition> conditions) {
    RC rc;
    RelationCatRecord rcRecord;
    if ((rc = smManager->GetRelationInfo(relation, rcRecord))) {
        return rc;
    }

    vector<AttrCatRecord> attrs;
    if ((rc = smManager->GetAttrInfo(relation, rcRecord.attrCount, attrs))) {
        return rc;
    }

    if ((rc = ValidateConditions(attrs, conditions))) {
        return rc;
    }

    int indexPos = -1;
    for (int i = 0; i < conditions.size() && indexPos == -1; i++) {
        for (int j = 0; j < rcRecord.attrCount; j++) {
            if (attrs[j].attrName == conditions[i].lAttr) {
                if (attrs[j].indexNo != -1) {
                    indexPos = i;
                    break;
                }
            }
        }
    }

    shared_ptr<QL_Op> scanOp;

    RM_FileHandle rmFH;
    RID rid;
    RM_Record record;
    char* recordData;
    if ((rc = rmManager->OpenFile(relation, rmFH))) {
        return rc;
    }
    string attrName = conditions[0].lAttr;
    CmpOp op = conditions[0].op;
    AttrCatRecord attrData;
    smManager->GetAttrInfo(relation, attrName, attrData);
    if (indexPos != -1) {
        scanOp.Reset(new QL_IndexScanOp(smManager, ixManager, rmManager, relation, attrName, op, &conditions[indexPos].rValue));
    } else {
        scanOp.Reset(new QL_FileScanOp(smManager, rmManager, relation, true, attrName, op, &conditions[0].rValue));
    }

    if (rc = scanOp->Open()) {
        return rc;
    }

    IX_IndexHandle* ixIH = new IX_IndexHandle[rcRecord.attrCount];
    for (int i = 0; i < rcRecord.attrCount; i++) {
        if (attrs[i].indexNo != -1) {
            if ((rc = ixManager->OpenIndex(relation, attrs[i].indexNo, ixIH[i]))) {
                return rc;
            }
        }
    }

    while ((rc = scanOp->GetNext(rid)) != EOF) {
        if ((rc = rmFH.GetRecord(rid, record)) || (rc = record.GetData(recordData))) {
            return rc;
        }

        bool match;
        if ((rc = MatchCondition(recordData, match, attrs, conditions))) {
            return rc;
        }

        if (match) {
            if ((rc = rmFH.DeleteRecord(rid))) {
                return rc;
            }

            for (int i = 0; i < rcRecord.attrCount; i++) {
                if (attrs[i].indexNo != -1) {
                    Value value;
                    switch (attrs[i].attrType) {
                        case INT:
                            value = Value(*((int *)recordData + attrs[i].offset));
                            break;
                        case FLOAT:
                            value = Value(*((float *)recordData + attrs[i].offset));
                            break;
                        case CHARN:
                            value = Value(*((float *)recordData + attrs[i].offset));
                            break;
                    }
                    if ((rc = ixIH[i].DeleteEntry(value, rid))) {
                        return rc;
                    }
                }
            }
        }
    }

    for (int i = 0; i < attrs.size(); i++) {
        if (attrs[i].indexNo != -1) {
            if ((rc = ixManager->CloseIndex(ixIH[i]))) {
                return rc;
            }
        }
    }
    delete[] ixIH;

    if ((rc = scanOp->Close()) || (rc = rmManager->CloseFile(rmFH))) {
        return rc;
    }

}




template<class T>
bool matchRecord(T lValue, T rValue, CmpOp op) {
    switch (op) {
        case EQ:
            return lValue == rValue;
        case NE:
            return lValue != rValue;
        case LT:
            return lValue < rValue;
        case GT:
            return lValue > rValue;
        case LE:
            return lValue <= rValue;
        case GE:
            return lValue >= rValue;
        default:
            assert(0);
            return false;
    }
}