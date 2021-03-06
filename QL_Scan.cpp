//
// Created by Orpine on 10/31/15.
//

#include "QL_Scan.h"
#include "QL.h"

QL_IndexScanHandle::QL_IndexScanHandle(SM_Manager *smm, IX_Manager *ixm, RM_Manager *rmm, const string &_relationName,
                                       const string &_attrName, CmpOp _op, const Value &v) {
    smManager = smm;
    ixManager = ixm;
    rmManager = rmm;
    relationName = _relationName;
    attrName = _attrName;
    op = _op;
    value = v;
    RelationCatRecord rcRecord;
    smManager->GetRelationInfo(relationName, rcRecord);
    attrCount = rcRecord.attrCount;
    tupleLength = rcRecord.tupleLength;
    smManager->GetAttrInfo(relationName, attrCount, attrs);
}

QL_IndexScanHandle::~QL_IndexScanHandle() { }

RC QL_IndexScanHandle::Open() {
    RC rc;
    AttrCatRecord attr;
    if ((rc = smManager->GetAttrInfo(relationName, attrName, attr))) {
        return rc;
    }
    int attrIndexNo = attr.indexNo;
    if ((rc = rmManager->OpenFile(relationName, rmFH))) {
        return rc;
    }
    if ((rc = ixManager->OpenIndex(relationName, attrIndexNo, ixIH))) {
        return rc;
    }
    if ((rc = ixIS.OpenScan(ixIH, op, value))) {
        return rc;
    }

    return RC_OK;
}

RC QL_IndexScanHandle::Close() {
    RC rc;
    if ((rc = ixIS.CloseScan())) {
        return rc;
    }
    if ((rc = ixManager->CloseIndex(ixIH))) {
        return rc;
    }
    if ((rc = rmManager->CloseFile(rmFH))) {
        return rc;
    }
    return RC_OK;
}


RC QL_IndexScanHandle::GetNext(char *recordData) {
    RC rc;
    RID rid;
    RM_Record record;
    char* data;

    if ((rc = ixIS.GetNextEntry(rid))) {
        return (rc == IX_EOF) ? QL_EOF : rc;
    }

    if ((rc = rmFH.GetRecord(rid, record)) || (rc = record.GetData(data))) {
        return rc;
    }

    memcpy(recordData, data, tupleLength);
    return RC_OK;
}

RC QL_IndexScanHandle::GetNext(RID &rid) {
    RC rc;
    RM_Record record;

    rc = ixIS.GetNextEntry(rid);
    if ((rc = ixIS.GetNextEntry(rid))) {
        return (rc == IX_EOF) ? QL_EOF : rc;
    }
    return RC_OK;
}

//void QL_IndexScanHandle::GetAttrCount(int &attrCount) {
//    attrCount = this->attrCount;
//}
//
//void QL_IndexScanHandle::GetAttrInfo(vector<AttrCatRecord> &attrs) {
//    attrs = this->attrs;
//}


QL_FileScanHandle::QL_FileScanHandle(SM_Manager *smm, RM_Manager *rmm, const string &_relationName) {
    smManager = smm;
    rmManager = rmm;
    relationName = _relationName;
    RelationCatRecord rcRecord;
    smManager->GetRelationInfo(relationName, rcRecord);
    attrCount = rcRecord.attrCount;
    tupleLength = rcRecord.tupleLength;
    smManager->GetAttrInfo(relationName, attrCount, attrs);

    haveValue = false;

}

QL_FileScanHandle::QL_FileScanHandle(SM_Manager *smm, RM_Manager *rmm, const string &_relationName,
                                     const string &_attrName, CmpOp _op, const Value &v) {
    smManager = smm;
    rmManager = rmm;
    relationName = _relationName;
    RelationCatRecord rcRecord;
    smManager->GetRelationInfo(relationName, rcRecord);
    attrCount = rcRecord.attrCount;
    tupleLength = rcRecord.tupleLength;
    smManager->GetAttrInfo(relationName, attrCount, attrs);

    haveValue = true;
    attrName = _attrName;
    op = _op;
    value = v;
}


RC QL_FileScanHandle::Open() {
    RC rc;
    if ((rc = rmManager->OpenFile(relationName, rmFH))) {
        return rc;
    }
    if (haveValue) {
        AttrCatRecord attr;
        if ((rc = smManager->GetAttrInfo(relationName, attrName, attr))) {
            return rc;
        }
        if ((rc = rmFS.OpenScan(rmFH, attr.attrType, attr.attrLength, attr.offset, op, value))) {
            return rc;
        }
    } else {
        if ((rc = rmFS.OpenScan(rmFH, INT, 4, 0, NO, 0))) {
            return rc;
        }
    }
    return RC_OK;
}

RC QL_FileScanHandle::Close() {
    RC rc;
    if ((rc = rmFS.CloseScan()) || (rc = rmManager->CloseFile(rmFH))) {
        return rc;
    }
    return RC_OK;
}

RC QL_FileScanHandle::GetNext(char *recordData) {
    RC rc;
    RM_Record record;
    char* data;
    if ((rc = rmFS.GetNextRecord(record)) || (rc = record.GetData(data))) {
        return (rc == RM_EOF) ? QL_EOF : rc;
    }
    memcpy(recordData, data, tupleLength);
    return RC_OK;
}

RC QL_FileScanHandle::GetNext(RID &rid) {
    RC rc;
    RM_Record record;
    if ((rc = rmFS.GetNextRecord(record)) || (rc = record.GetRid(rid))) {
        return (rc == RM_EOF) ? QL_EOF : rc;
    }
    return RC_OK;
}

//void QL_FileScanHandle::GetAttrCount(int &attrCount) {
//    attrCount = this->attrCount;
//}
//
//void QL_FileScanHandle::GetAttrInfo(vector<AttrCatRecord> &attrs) {
//    attrs = this->attrs;
//}

//QL_RootHandle::QL_RootHandle(SM_Manager *smm, shared_ptr<QL_ScanHandle> _child) {
//    smManager = smm;
//    child = _child;
////    RelationCatRecord rcRecord;
////    smManager->GetRelationInfo(relationName, rcRecord);
////    tupleLength = rcRecord.tupleLength;
//}
//
//QL_RootHandle::~QL_RootHandle() { }
//
//RC QL_RootHandle::Open() {
//    RC rc;
//    if ((rc = child->Open())) {
//        return rc;
//    }
//    return RC_OK;
//}
//
//RC QL_RootHandle::Close() {
//    RC rc;
//    if ((rc = child->Close())) {
//        return rc;
//    }
//    return RC_OK;
//}
//
//RC QL_RootHandle::GetNext(char *recordData) {
//    RC rc;
////    char* data = new char[child->GetTupleLength()];
//    if ((rc = child->GetNext(recordData))) {
//        return rc;
//    }
////    int childAttrCount = child->GetAttrCount();
////    vector<AttrCatRecord> childAttrInfo = child->GetAttrInfo();
////    int tupleLength = 0;
////    for (int i = 0; i < )
//
//    return RC_OK;
//}


QL_FilterHandle::QL_FilterHandle(SM_Manager *smm, shared_ptr<QL_ScanHandle> _child, Condition _filter, const string &relationName): filter(_filter) {
    smManager = smm;
    child = _child;
    RelationCatRecord rcRecord;
    smManager->GetRelationInfo(relationName, rcRecord);
    tupleLength = rcRecord.tupleLength;
}

QL_FilterHandle::~QL_FilterHandle() { }

RC QL_FilterHandle::Open() {
    RC rc;
    if ((rc = child->Open())) {
        return rc;
    }
    return RC_OK;
}

RC QL_FilterHandle::Close() {
    RC rc;
    if ((rc = child->Close())) {
        return rc;
    }
    return RC_OK;
}

RC QL_FilterHandle::GetNext(char *recordData) {
    RC rc;
    char* data = new char [tupleLength];
    bool match = false;
    while (child->GetNext(data) != QL_EOF) {
        AttrCatRecord lacRecord;
        if ((rc = smManager->GetAttrInfo(filter.lAttr.relationName, filter.lAttr.attrName, lacRecord))) {
            return rc;
        }
        switch (lacRecord.attrType) {
            case INT: {
                Value lValue(*((int *)(data + lacRecord.offset)));
                match = matchRecord(lValue.iData, filter.rValue.iData, filter.op);
                break;
            }
            case FLOAT: {
                Value lValue(*((float *)(data + lacRecord.offset)));
                match = matchRecord(lValue.fData, filter.rValue.fData, filter.op);
                break;
            }
            case CHARN: {
                Value lValue(string(data + lacRecord.offset));
                match = matchRecord(lValue.strData, filter.rValue.strData, filter.op);
                break;
            }
        }
        if (match) {
            break;
        }
    }
    if (!match) {
        delete[] data;
        return QL_EOF;
    }
    memcpy(recordData, data, tupleLength);
    delete[] data;
    return RC_OK;
}

