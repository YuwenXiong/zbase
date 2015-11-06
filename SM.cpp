#include "SM.h"
#include "zbase.h"

SM_Manager::SM_Manager(IX_Manager &ixm, RM_Manager &rmm): ixm(ixm), rmm(rmm) {

}

SM_Manager::~SM_Manager() {

}

RC SM_Manager::CreateDb(const string &dbName) {
	RC rc;

	if((rc = rmm.CreateFile("relcat", sizeof(RelationCatRecordC))) != 0)
		return rc;
	if((rc = rmm.CreateFile("attrcat", sizeof(AttrCatRecordC))) != 0)
		return rc;

	return 0;
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
		attrRecord.property = attrs[i].property;
		attrRecord.indexNo = -1;
		AttrCatRecordC temp(attrRecord);
		if((rc = attrfh.InsertRecord((char*)&temp, rid)) != 0)
			return rc;
		size += attrs[i].attrLength;
	}

	if((rc = rmm.CreateFile(relationName, size)) != 0)
		return rc;

	relRecord.relationName = relationName;
	relRecord.tupleLength = size;
	relRecord.attrCount = attrs.size();
	relRecord.indexCount = 0;
	RelationCatRecordC temp(relRecord);
	if((rc = relfh.InsertRecord((char *) &temp, rid)) != 0)
		return rc;

	return 0;
}

RC SM_Manager::DropTable(const string &relationName) {
	RM_FileScan rmfs;
	RC rc;
	RID rid;
	bool isFound = false;
	RM_Record rec;
	char* data;
	Value value;

	value.type = CHARN;
	value.strData = relationName;

	if((rc = rmfs.OpenScan(relfh, CHARN, MAXNAME, offsetof(RelationCatRecordC, relationName), EQ, value)) != 0 )
		return rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData(data);
		if(relationName == ((RelationCatRecordC *)data)->relationName) {
			isFound = true;
			break;
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(!isFound)
		return SM_NOTFOUND;

	if((rc = rmm.DestroyFile(relationName)) != 0)
		return rc;

	rec.GetRid(rid);
	if((rc = relfh.DeleteRecord(rid)) != 0)
		return rc;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXNAME, offsetof(AttrCatRecordC, relationName), EQ, value)) != 0)
		return rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData(data);
		if(relationName == ((AttrCatRecordC *)data)->relationName) {
			if(((AttrCatRecordC *)data)->indexNo != -1)
				DropIndex(relationName, ((AttrCatRecordC *)data)->attrName);
			rec.GetRid(rid);
			if((rc = attrfh.DeleteRecord(rid)) != 0)
				return rc;
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	return 0;
}

RC SM_Manager::CreateIndex(const string &relationName, const string &attrName, const string &indexName) {
	RC rc;
	RID rid;
	AttrCatRecord attrRecord;
	RelationCatRecord relRecord;
	RM_Record rec;
	IX_IndexHandle ixh;
	RM_FileHandle rmfh;
	RM_FileScan rmfs;
	char *pdata, *pdata1, *pdata2;
	string s;
	Value value, data;
	map<string, string> m;

	if((rc = GetAttrInfo(relationName, attrName, attrRecord, rid)) != 0)
		return rc;
	if(attrRecord.indexNo != -1)
		return SM_INDEXEXISTS;
	if((rc = attrfh.GetRecord(rid, rec)) != 0)
		return rc;
	rec.GetData(pdata1);
	attrRecord.indexNo = attrRecord.offset;
	((AttrCatRecordC *)pdata1)->indexNo = attrRecord.offset;
	if((rc = attrfh.UpdateRecord(rec)) != 0)
		return rc;

	mapGet(m);
	s = relationName + "," + attrName;
	m[indexName] = s;
	mapSave(m);

	if((rc = GetRelationInfo(relationName, relRecord, rid)) != 0)
		return rc;
	if((rc = relfh.GetRecord(rid, rec)) != 0)
		return rc;
	rec.GetData(pdata2);
	((RelationCatRecordC *)pdata2)->indexCount++;
	if((rc = relfh.UpdateRecord(rec)) != 0)
		return rc;

	if((rc = ixm.CreateIndex(relationName, attrRecord.indexNo, attrRecord.attrType, attrRecord.attrLength)) != 0)
		return rc;
	if((rc = ixm.OpenIndex(relationName, attrRecord.indexNo, ixh)) != 0)
		return rc;
	if((rc = rmm.OpenFile(relationName, rmfh)) != 0)
		return rc;

	rc = rmfs.OpenScan(rmfh, attrRecord.attrType, attrRecord.attrLength, attrRecord.offset, NO, value);
	if(rc == RM_SCAN_EMPTY_RECORD) {
		if((rc = rmm.CloseFile(rmfh)) != 0)
			return rc;
		if((rc = ixm.CloseIndex(ixh)) != 0)
			return rc;
		return 0;
	}
	else if(rc != 0)
		return rc;

	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData(pdata);
		rec.GetRid(rid);
		data.type = attrRecord.attrType;
		if(data.type == INT)
			data.iData = *(int *)(pdata + attrRecord.offset);
		else if(data.type == FLOAT)
			data.fData = *(float *)(pdata + attrRecord.offset);
		else if(data.type == CHARN)
			data.strData = string(pdata + attrRecord.offset);
		ixh.InsertEntry(data, rid);
	}

	if((rc = rmfs.CloseScan()) != 0)
		return rc;
	if((rc = rmm.CloseFile(rmfh)) != 0)
		return rc;
	if((rc = ixm.CloseIndex(ixh)) != 0)
		return rc;

	return 0;
}

RC SM_Manager::DropIndex(const string &indexName) {
	string s, relationName, attrName;
	int x;
	map<string, string> m;

	mapGet(m);
	if(m.find(indexName) != m.end())
		s = m[indexName];
	else
		return SM_NOTFOUND;
	m.erase(indexName);
	mapSave(m);

	x = s.find(",");
	relationName = s.substr(0, x);
	attrName = s.substr(x+1);

	DropIndex(relationName, attrName);

	return 0;
}

RC SM_Manager::DropIndex(const string &relationName, const string &attrName) {
	RM_FileScan rmfs;
	bool isFound = false;
	RM_Record rec;
	RelationCatRecord relRecord;
	char* pdata;
	RID rid;
	RC rc;
	Value value;

	value.type = CHARN;
	value.strData = relationName;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXNAME, offsetof(AttrCatRecordC, relationName), EQ, value)) != 0)
		return rc;

	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData(pdata);
		if(attrName == ((AttrCatRecordC *)pdata)->attrName) {
			((AttrCatRecordC *)pdata)->indexNo = -1;
			isFound = true;
			break;
		}
	}

	if((rc = rmfs.CloseScan()) != 0)
		return rc;
	if(!isFound)
		return SM_NOTFOUND;

	rec.GetRid(rid);
	if((rc = ixm.DestroyIndex(relationName, ((AttrCatRecordC *)pdata)->offset)))
		return rc;

	if((rc = attrfh.UpdateRecord(rec)) != 0)
		return rc;

	if((rc = GetRelationInfo(relationName, relRecord, rid)) != 0)
		return rc;
	if((rc = relfh.GetRecord(rid, rec)) != 0)
		return rc;
	rec.GetData(pdata);
	((RelationCatRecordC *)pdata)->indexCount--;
	if((rc = relfh.UpdateRecord(rec)) != 0)
		return rc;

	return 0;
}

RC SM_Manager::GetAttrInfo(const string &relationName, int attrCount, vector<AttrCatRecord> &attrs) {
	RM_FileScan rmfs;
	RC rc;
	RM_Record rec;
	bool isFound = false;
	AttrCatRecord attrData;
	char* data;
	Value value;

	value.type = CHARN;
	value.strData = relationName;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXNAME, offsetof(AttrCatRecordC, relationName), EQ, value)) != 0)
		return rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData(data);
		if(relationName == ((AttrCatRecordC *)data)->relationName) {
			isFound = true;
			AttrCatRecordC temp(attrData);
			memcpy(&temp, data, sizeof(AttrCatRecordC));
			attrs.push_back(AttrCatRecord(temp));
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(!isFound)
		return SM_NOTFOUND;

	return 0;
}

RC SM_Manager::GetAttrInfo(const string &relationName, const string &attrName, AttrCatRecord &attrData) {
	RM_FileScan rmfs;
	RC rc;
	RM_Record rec;
	bool isFound = false;
	char* data;
	Value value;

	value.type = CHARN;
	value.strData = attrName;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXNAME, offsetof(AttrCatRecordC, attrName), EQ, value)) != 0)
		return rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData(data);

		if(attrName == ((AttrCatRecordC *)data)->attrName) {
			isFound = true;
			break;
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(!isFound)
		return SM_NOTFOUND;

	AttrCatRecordC temp(attrData);
	memcpy(&temp, data, sizeof(AttrCatRecordC));
	attrData = AttrCatRecord(temp);

	return 0;
}

RC SM_Manager::GetAttrInfo(const string &relationName, const string &attrName, AttrCatRecord &attrData, RID &rid) {
	RM_FileScan rmfs;
	RC rc;
	RM_Record rec;
	bool isFound = false;
	char* data;
	Value value;

	value.type = CHARN;
	value.strData = attrName;

	if((rc = rmfs.OpenScan(attrfh, CHARN, MAXNAME, offsetof(AttrCatRecordC, attrName), EQ, value)) != 0)
		return rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData(data);

		if(attrName == ((AttrCatRecordC *)data)->attrName) {
			rec.GetRid(rid);
			isFound = true;
			break;
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(!isFound)
		return SM_NOTFOUND;

	AttrCatRecordC temp(attrData);
	memcpy(&temp, data, sizeof(AttrCatRecordC));
	attrData = AttrCatRecord(temp);

	return 0;
}

RC SM_Manager::GetRelationInfo(const string &relationName, RelationCatRecord &relationData) {
	RM_FileScan rmfs;
	RC rc;
	RM_Record rec;
	bool isFound = false;
	char* data;
	Value value;

	value.type = CHARN;
	value.strData = relationName;

	if((rc = rmfs.OpenScan(relfh, CHARN, MAXNAME, offsetof(RelationCatRecordC, relationName), EQ, value)) != 0 )
		return rc == RM_SCAN_EMPTY_RECORD ? SM_NOTFOUND : rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData(data);
		if(relationName == ((AttrCatRecordC *)data)->relationName) {
			isFound = true;
			break;
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(!isFound)
		return SM_NOTFOUND;

	RelationCatRecordC temp(relationData);
	memcpy(&temp, data, sizeof(RelationCatRecordC));
	relationData = RelationCatRecord(temp);

	return 0;
}

RC SM_Manager::GetRelationInfo(const string &relationName, RelationCatRecord &relationData, RID &rid) {
	RM_FileScan rmfs;
	RC rc;
	RM_Record rec;
	bool isFound = false;
	char* data;
	Value value;

	value.type = CHARN;
	value.strData = relationName;

	if((rc = rmfs.OpenScan(relfh, CHARN, MAXNAME, offsetof(RelationCatRecordC, relationName), EQ, value)) != 0 )
		return rc;
	while(true) {
		rc = rmfs.GetNextRecord(rec);
		if(rc == RM_EOF)
			break;
		if(rc != 0)
			return rc;

		rec.GetData(data);
		if(relationName == ((AttrCatRecordC *)data)->relationName) {
			rec.GetRid(rid);
			isFound = true;
			break;
		}
	}
	if((rc = rmfs.CloseScan()) != 0)
		return rc;

	if(!isFound)
		return SM_NOTFOUND;

	RelationCatRecordC temp(relationData);
	memcpy(&temp, data, sizeof(RelationCatRecordC));
	relationData = RelationCatRecord(temp);

	return 0;
}

void mapSave(const map<string, string> &m) {
	ofstream fout("mapIndex");

	for(auto it = m.begin(); it != m.end(); it++) {
		fout << it->first << " " << it->second << endl;
	}

	fout.close();
}

void mapGet(map<string, string> &m) {
	string s1, s2;
	ifstream fin("mapIndex");

	if(!fin) {
		ofstream fout("mapIndex");
		fout.close();
		ifstream fin("mapIndex");
	}

	while(fin) {
		fin >> s1;
		fin >> s2;
		m[s1] = s2;
	}

	fin.close();
}
