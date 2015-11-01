//
// Created by flydrt on 11/1/15.
//
#include "zbase.h"

RelationCatRecord::RelationCatRecord(){}
RelationCatRecord::RelationCatRecord(const RelationCatRecordC &x): relationName(x.relationName), tupleLength(x.tupleLength), attrCount(x.attrCount), indexCount(x.indexCount) {

}

RelationCatRecordC::RelationCatRecordC(const RelationCatRecord &x) {
    strncpy(relationName, x.relationName.c_str(), MAXNAME);
    tupleLength = x.tupleLength;
    attrCount = x.attrCount;
    indexCount = x.indexCount;
}

AttrCatRecord::AttrCatRecord(){}
AttrCatRecord::AttrCatRecord(const AttrCatRecordC &x) {
    relationName = x.relationName;
    attrName = x.attrName;
    offset = x.offset;
    attrType = x.attrType;
    attrLength = x.attrLength;
    indexNo = x.indexNo;
    property = x.property;
}

AttrCatRecordC::AttrCatRecordC(const AttrCatRecord &x) {
    strncpy(relationName, x.relationName.c_str(), MAXNAME);
    strncpy(attrName, x.attrName.c_str(), MAXNAME);
    offset = x.offset;
    attrType = x.attrType;
    attrLength = x.attrLength;
    indexNo = x.indexNo;
    property = x.property;
}