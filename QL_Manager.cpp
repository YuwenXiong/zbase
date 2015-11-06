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
RC QL_Manager::Select(vector <AttrInfo> &selectAttrs, const string &relation,
                      const vector <Condition> &conditions) {
    RC rc;
    RelationCatRecord rcRecord;
    AttrCatRecord attr;
    int attrCount;
    if ((rc = smManager->GetRelationInfo(relation, rcRecord))) {

        return rc;
    }
    attrCount = rcRecord.attrCount;
    vector<AttrCatRecord> attrs;
    if ((rc = smManager->GetAttrInfo(relation, attrCount, attrs))) {
        return rc;
    }

    if ((rc = ValidateConditions(attrs, conditions))) {
        return rc;
    }

//    selectAttrs.clear();
//    for (int i = 0; i < rcRecord.attrCount; i++) {
//        selectAttrs.push_back(attrs[i]);
//    }

    shared_ptr<QL_ScanHandle> scanHandle, lastHandle;
    bool useIndex = false;
    vector<Condition> changedCondition(conditions);
    for (auto i = changedCondition.begin(); i != changedCondition.end(); ) {
        if ((rc = smManager->GetAttrInfo(relation, (i->lAttr).attrName, attr))) {
            return rc;
        }
        if (attr.indexNo != -1) {
            scanHandle.reset(new QL_IndexScanHandle(smManager, ixManager, rmManager, relation, (i->lAttr).attrName, i->op, i->rValue));
            i = changedCondition.erase(i);
            useIndex = true;
        } else {
            i++;
        }
    }

    if (!useIndex) {
        scanHandle.reset(new QL_FileScanHandle(smManager, rmManager, relation));
    }

    lastHandle = scanHandle;
    vector<shared_ptr<QL_ScanHandle>> filterHandle(changedCondition.size());
    for (int i = 0; i < changedCondition.size(); i++) {
        filterHandle[i].reset(new QL_FilterHandle(smManager, lastHandle, changedCondition[i], relation));
        lastHandle = filterHandle[i];
    }
    shared_ptr<QL_ScanHandle> rootHandle;
    rootHandle.reset(new QL_RootHandle(smManager, lastHandle));


    Printer printer(attrs);
    char* recordData = new char[rcRecord.tupleLength];
    rootHandle->Open();
    while (rootHandle->GetNext(recordData) != QL_EOF) {
        printer.Print(recordData);
    }
    rootHandle->Close();
    delete[] recordData;
    return RC_OK;
}

RC QL_Manager::Insert(const string &relation, vector<Value> values) {
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

    if (values.size() != rcRecord.attrCount) {
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
            case INT: {
                memcpy(tupleData + attrs[i].offset, &values[i].iData, attrs[i].attrLength);
                break;
            }
            case FLOAT: {
                memcpy(tupleData + attrs[i].offset, &values[i].fData, attrs[i].attrLength);
                break;
            }
            case CHARN: {
                char *value = new char[attrs[i].attrLength];
                memset(value, 0, attrs[i].attrLength);
                strcpy(value, values[i].strData.c_str());
                memcpy(tupleData + attrs[i].offset, value, attrs[i].attrLength);
                break;
            }
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
            if (attrs[j].attrName == conditions[i].lAttr.attrName) {
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
        CmpOp op = conditions[indexPos].op;
        AttrCatRecord attrData;
        smManager->GetAttrInfo(relation, conditions[indexPos].lAttr.attrName, attrData);
        scanHandle.reset(new QL_IndexScanHandle(smManager, ixManager, rmManager, relation, conditions[indexPos].lAttr.attrName, op, conditions[indexPos].rValue));
    } else {
        if (conditions.size() > 0) {
            CmpOp op = conditions[0].op;
            AttrCatRecord attrData;
            smManager->GetAttrInfo(relation, conditions[0].lAttr.attrName, attrData);
            scanHandle.reset(new QL_FileScanHandle(smManager, rmManager, relation, conditions[0].lAttr.attrName, op, conditions[0].rValue));
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
    for (int i = 0; i < conditions.size(); i++) {
        smManager->GetAttrInfo(conditions[i].lAttr.relationName, conditions[i].lAttr.attrName, lacRecord);
        switch (lacRecord.attrType) {
            case INT: {
                Value lValue(*((int *)(recordData + lacRecord.offset)));
                if (!matchRecord(lValue.iData, conditions[i].rValue.iData, conditions[i].op)) {
                    return false;
                }
                break;
            }
            case FLOAT: {
                Value lValue(*((float *)(recordData + lacRecord.offset)));
                if (!matchRecord(lValue.fData, conditions[i].rValue.fData, conditions[i].op)) {
                    return false;
                }
                break;
            }
            case CHARN: {
                string lValue(recordData + (lacRecord.offset));
                if (!matchRecord(lValue, conditions[i].rValue.strData, conditions[i].op)) {
                    return false;
                }
                break;
            }
        }
    }
    return true;
}


template<class T>
bool matchRecord(const T &lValue, const T &rValue, CmpOp op) {
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
            if (attrs[j].attrName == conditions[i].lAttr.attrName && attrs[j].attrType == conditions[i].rValue.type) {
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