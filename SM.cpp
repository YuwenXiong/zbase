#include "SM.h"
#include "zbase.h"

SM_Manager::SM_Manager(IX_Manager &ixm, RM_Manager &rmm): ixm(ixm), rmm(rmm) {

}

SM_Manager::~SM_Manager() {

}

RC SM_Manager::OpenDb(const string &dbName) {
	RC rc;

	if((rc = rmm.OpenFile("relcat", relfh)) != 0)
		return rc;
	if((rc = rmm.OpenFile("attrcat", attrfh)) != 0)
		return rc;

	return 0;
}

RC SM_Manager::CloseDb() {
	RC rc;

	if((rc = rmm.CloseFile(relfh)) != 0)
		return rc;
	if((rc = rmm.CloseFile(attrfh)) != 0)
		return rc;

	return 0;
}

RC SM_Manager::CreateTable(const string &relationName, const vector<AttrInfo> &attrs) {
	int size = 0;
	RID rid;
	RC rc;
	RelationCatRecord relRecord;
	AttrCatRecord attrRecord;

	for(int i = 0; i < attrs.size(); i++) {
		attrRecord.relationName = relationName;
		attrRecord.attrName = attrs[i].attrName;
		attrRecord.offset = size;
		attrRecord.attrType = attrs[i].attrType;
		attrRecord.attrLength = attrs[i].attrLength;
		attrRecord.indexNo = -1;
		if((rc = attrfh.InsertRecord((char*) &attrRecord, rid)) != 0)
			return rc;
		size += attrs[i].attrLength;
	}

	if((rc = rmm.CreateFile(relationName, size)) != 0)
		return rc;

	relRecord.relationName = relationName;
	relRecord.tupleLength = size;
	relRecord.attrCount = attrs.size();
	relRecord.indexCount = 0;
	if((rc = relfh.InsertRecord((char *) &relRecord, rid)) != 0)
		return rc;

	return 0;
}

RC SM_Manager::DropTable(const string &relationName) {
	RM_FileScan rmfs;
	RC rc;
	RID rid;
	bool isFound = false;
	RM_Record rec;
	RelationCatRecord *relRecord;
	AttrCatRecord *attrRecord;

	if((rc = rmfs.OpenScan(relfh, CHARN, MAXRELATIONNAME, offsetof(RelationCatRecord, relationName), EQ, (void*) relationName.c_str())) != 0 )
		return rc;
	rc = rmfs.GetNextRecord(rec);
	while(rc != RM_EOF) {
		if(rc != 0)
			return rc;

		rec.GetData((char*&) relRecord);
		if(relRecord->relationName == relationName) {
			isFound = true;
			break;
		}

		rc = rmfs.GetNextRecord(rec);
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(isFound == false)
		return SM_NOTFOUND;

	if((rc = rmm.DestoryFile(relationName)) != 0)
		return rc;

	rec.GetRid(rid);
	if((rc = relfh.DeleteRecord(rid)) != 0)
		return rc;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXRELATIONNAME, offsetof(AttrCatRecord, relationName), EQ, (void*) relationName.c_str())) != 0)
		return rc;
	rc = rmfs.GetNextRecord(rec);
	while(rc != RM_EOF) {
		if(rc != 0)
			return rc;

		rec.GetData((char*&) attrRecord);
		if(attrRecord->relationName == relationName) {
			if(attrRecord->indexNo != -1)
				DropIndex(relationName, attrRecord->attrName);
			rec.GetRid(rid);
			if((rc = attrfh.DeleteRecord(rid)) != 0)
				return rc;
		}

		rc = rmfs.GetNextRecord(rec);
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	return 0;
}

RC SM_Manager::CreateIndex(const string &relationName, const string &attrName) {}

RC SM_Manager::DropIndex(const string &relationName, const string &attrName) {}

RC SM_Manager::GetAttrInfo(const string &relationName, int attrCount, vector<AttrCatRecord> &attrs) {
	RM_FileScan rmfs;
	RC rc;
	RM_Record rec;
	RID rid;
	bool isFound = false;
	RelationCatRecord *relRecord;
	AttrCatRecord *attrRecord;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXRELATIONNAME, offsetof(AttrCatRecord, relationName), EQ, (void*) relationName.c_str())) != 0)
		return rc;
	rc = rmfs.GetNextRecord(rec);
	while(rc != RM_EOF) {
		if(rc != 0)
			return rc;

		rec.GetData((char*&) attrRecord);
		if(attrRecord->relationName == relationName) {
			attrs.push_back(*attrRecord);
		}

		rc = rmfs.GetNextRecord(rec);
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(isFound == false)
		return SM_NOTFOUND;

	return 0;
}

RC SM_Manager::GetAttrInfo(const string &relationName, const string &attrName, AttrCatRecord &attrData) {
	RM_FileScan rmfs;
	RC rc;
	RM_Record rec;
	RID rid;
	bool isFound = false;
	AttrCatRecord *attrRecord;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXRELATIONNAME, offsetof(AttrCatRecord, attrName), EQ, (void*) attrName.c_str())) != 0)
		return rc;
	rc = rmfs.GetNextRecord(rec);
	while(rc != RM_EOF) {
		if(rc != 0)
			return rc;

		rec.GetData((char*&) attrRecord);
		if(attrRecord->attrName == attrName) {
			isFound = true;
			break;
		}

		rc = rmfs.GetNextRecord(rec);
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(isFound == false)
		return SM_NOTFOUND;

	attrData = *attrRecord;

	return 0;
}

RC SM_Manager::GetRelationInfo(const string &relationName, RelationCatRecord &relationData) {
	RM_FileScan rmfs;
	RC rc;
	RM_Record rec;
	RID rid;
	bool isFound = false;
	RelationCatRecord *relRecord;

	if((rc = rmfs.OpenScan(relfh, CHARN, MAXRELATIONNAME, offsetof(RelationCatRecord, relationName), EQ, (void*) relationName.c_str())) != 0 )
		return rc;
	rc = rmfs.GetNextRecord(rec);
	while(rc != RM_EOF) {
		if(rc != 0)
			return rc;

		rec.GetData((char*&) relRecord);
		if(relRecord->relationName == relationName) {
			isFound = true;
			break;
		}

		rc = rmfs.GetNextRecord(rec);
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(isFound == false)
		return SM_NOTFOUND;

	relationData = *relRecord;

	return 0;
}

