//
// Created by Orpine on 10/31/15.
//

#include <assert.h>
#include "QL.h"
#include "SM.h"
#include "QL_Scan.h"

QL_Manager::QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm) {
    smManager = &smm;
    ixManager = &ixm;
    rmManager = &rmm;
}
QL_Manager::~QL_Manager() { }

// actually we would query all attrs and only one relation
RC QL_Manager::Select(vector <RelationAttr> &selectAttrs, const vector <string> &relations,
                      const vector <Condition> &conditions) {
    RC rc;
    RelationCatRecord rcRecord;
    AttrCatRecord attr;
    int attrCount;
    if ((rc = smManager->GetRelationInfo(relations[0], rcRecord))) {
        return rc;
    }
    attrCount = rcRecord.attrCount;
    vector<AttrCatRecord> attrs;
    if ((rc = smManager->GetAttrInfo(relations[0], attrCount, attrs))) {
        return rc;
    }

    if ((rc = ValidateConditions(attrs, conditions))) {
        return rc;
    }

    selectAttrs.clear();
    for (int i = 0; i < rcRecord.attrCount; i++) {
        selectAttrs.push_back(attrs[i]);
    }

    shared_ptr<QL_ScanHandle> scanHandle;
    bool useIndex = false;
    for (int i = 0; i < conditions.size(); i++) {
        if ((rc = smManager->GetAttrInfo(relations[0], conditions[i].lAttr, attr))) {
            return rc;
        }
        if (attr.indexNo != -1) {
            scanHandle.reset(new QL_IndexScanHandle(smManager, ixManager, rmManager, relations[0], conditions[i].lAttr, conditions[i].op, conditions[i].rValue));
            removeCondition(conditions, i);
            useIndex = true;
        }
    }

    if (!useIndex) {
        scanHandle.reset(new QL_FileScanHandle(smManager, rmManager, relations[0]));
    }

    auto lastHandle = scanHandle;
    if (conditions.size() > 0) {
        shared_ptr<QL_ScanHandle> filterHandle[conditions.size()];
        for (int i = 0; i < conditions.size(); i++) {
            filterHandle[i].reset(new QL_FilterHandle(smManager, lastHandle, conditions[i]));
            lastHandle = filterHandle[i];
        }
    }

    shared_ptr<QL_ScanHandle> rootHandle;
    rootHandle.reset(new QL_ProjectHandle(smManager, lastHandle, relations[0]));


    Printer printer(attrs);
    char* recordData = new char[rcRecord.tupleLength];
    rootHandle->Open();
    while (rootHandle->GetNext(recordData) != QL_EOF) {
        printer.Print(recordData);
    }
    rootHandle->Close();
    delete[] recordData;
}

RC QL_Manager::Insert(const string &relation, const vector<Value> values) {
    RC rc;
    RelationCatRecord rcRecord;

    if ((rc = smManager->GetRelationInfo(relation, rcRecord))) {
        return rc;
    }

    vector<AttrCatRecord> attrs;
    if ((rc = smManager->GetAttrInfo(relation, rcRecord.attrCount, attrs))) {
        return rc;
    }

    char* tupleData = new char[rcRecord.tupleLength];
    memset(tupleData, 0, sizeof(char) * rcRecord.tupleLength);

    if (values.size() != rcRecord.tupleLength) {
        return QL_INVALID_ATTR_COUNT;
    }

    for (int i = 0; i < values.size(); i++) {
        if (values[i].type != attrs[i].attrType) {
            return QL_INVALID_ATTR_TYPE;
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

    shared_ptr<QL_ScanHandle> scanHandle;

    RM_FileHandle rmFH;
    RID rid;
    RM_Record record;
    char* recordData;
    if ((rc = rmManager->OpenFile(relation, rmFH))) {
        return rc;
    }
    if (indexPos != -1) {
        string attrName = conditions[indexPos].lAttr;
        CmpOp op = conditions[indexPos].op;
        AttrCatRecord attrData;
        smManager->GetAttrInfo(relation, attrName, attrData);
        scanHandle.reset(new QL_IndexScanHandle(smManager, ixManager, rmManager, relation, attrName, op, conditions[indexPos].rValue));
    } else {
        if (conditions.size() > 0) {
            string attrName = conditions[0].lAttr;
            CmpOp op = conditions[0].op;
            AttrCatRecord attrData;
            smManager->GetAttrInfo(relation, attrName, attrData);
            scanHandle.reset(new QL_FileScanHandle(smManager, rmManager, relation, attrName, op, conditions[0].rValue));
        } else {
            scanHandle.reset(new QL_FileScanHandle(smManager, rmManager, relation));
        }
    }

    if ((rc = scanHandle->Open())) {
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

    while (scanHandle->GetNext(rid) != EOF) {
        if ((rc = rmFH.GetRecord(rid, record)) || (rc = record.GetData(recordData))) {
            return rc;
        }

        if (MatchConditions(recordData, attrs, conditions)) {
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

    if ((rc = scanHandle->Close()) || (rc = rmManager->CloseFile(rmFH))) {
        return rc;
    }
    return RC_OK;
}

bool QL_Manager::MatchConditions(char *recordData, const vector<AttrCatRecord> &attrs,
                                 const vector<Condition> &conditions) {
    AttrCatRecord lacRecord;
    string relation = attrs[0].relationName;
    for (int i = 0; i < conditions.size(); i++) {
        string lAttr = conditions[i].lAttr;
        smManager->GetAttrInfo(relation, lAttr, lacRecord);
        switch (lacRecord.attrType) {
            case INT: {
                int lValue;
                memcpy(&lValue, recordData + (lacRecord.offset), sizeof(lValue));
                if (!matchRecord(lValue, conditions[i].rValue.iData, conditions[i].op)) {
                    return false;
                }
            }
            case FLOAT: {
                float lValue;
                memcpy(&lValue, recordData + (lacRecord.offset), sizeof(lValue));
                if (!matchRecord(lValue, conditions[i].rValue.fData, conditions[i].op)) {
                    return false;
                }
            }
            case CHARN: {
                string lValue(recordData + (lacRecord.offset));
                if (!matchRecord(lValue, conditions[i].rValue.strData, conditions[i].op)) {
                    return false;
                }
            }
        }
    }
    return true;
}


template<class T>
bool QL_Manager::matchRecord(const T &lValue, const T &rValue, CmpOp op) {
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

RC QL_Manager::ValidateConditions(const vector<AttrCatRecord> &attrs, const vector<Condition> &conditions) {
    for (int i = 0; i < conditions.size(); i++) {
        bool found = false;
        for (int j = 0; j < attrs.size(); j++) {
            if (attrs[j].attrName == conditions[i].lAttr && attrs[j].attrType == conditions[i].rValue.type) {
                found = true;
                break;
            }
        }
        if (!found) {
            return QL_INVALID_WHERE_CLAUSE;
        }
    }
    return RC_OK;
}