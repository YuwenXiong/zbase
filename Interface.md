# Interface Conventiions

## Interpreter

## API

~~~cpp
struct RelationAttr {
	string relationName;
	string attrName;
}

struct Value {
	AttrType type;
	void *data;
}

struct Condition {
	RelationAttr lAttr; // left-hand side attr
	CmpOp op;
	bool rAttrIsAttr; // true if right-hand side is an attribute
	RelationAttr rAttr; // right-hand side attr
	Value rValue;
}

struct AttrInfo {
	string attrName;
	AttrType attrType;
	int attrLength;
};

struct DataAttrInfo {
	string relationName;
	string attrName;
	int offset;
	AttrType attrType;
	int attrLength;
	int indexNo;
};

class QL_Manager {
public:
	QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
	~QL_Manager;
	RC Select (const vector<RelationAttr> &selectAttrs,
			   const vector<string> &relations,
			   const vector<Condition> &conditions);
	RC Insert (const string &relationName, 
			   const vector<Value> &values);
	RC Delete (const string &relationName,
			   const vector<Condition> &conditions);
	RC Update (const string &relationName,
			   const RelationAttr &updateAttr,
			   bool isValue, // true if right-hand side is a value
			   const RelationAttr &rAttr,
			   const Value &rValue,
			   const vector<Condition> &conditions);
};

class SM_Manager {
public:
	SM_Manager(IX_Manager &ixm, RM_Manager &rmm);
	~SM_Manager();
	RC OpenDb(const string &dbName);
	RC CloseDb();
	RC CreateTable(const string &relationName, 
				   const vector<AttrInfo> &attrs);
	RC DropTable(const string &relationName);
	RC CreateIndex(const string &relationName,
				   const string &attrName);
	RC DropIndex(const string &relationName,
				 const string &attrName);
}

~~~


## Catalog Manager

## Record Manager
~~~cpp
class RM_Manager {
public:
	RM_Manager (PF_Manager &pfm);
	~RM_Manager();
	RC CreateFile(string fileName, int recordSize);
	RC DestroyFile(string fileName);
	RC OpenFile(string fileName, RM_FileHandle &fileHandle);
	RC CloseFile(RM_FileHandle &fileHandle);
}

class RM_FileHandle {
public:
	RM_FileHandle();
	~RM_FileHandle();
	RC GetRecord(const RID &rid, RM_Record &rec) const;
	RC InsertRecord(const char *data, RID &rid);
	RC DeleteRecord(const RID &rid);
	RC UpdateRecord(const RM_Record &record);
	RC ForcePages(PageNum pageNum = ALL_PAGES) const;
}

class RM_FileScan {
public:
	RM_FileScan();
	~RM_FileScan();
	RC OpenScan(const RM_FileHandle &fileHandle, 
				  AttrType attrType,
				  int attrLength,
				  int attrOffset,
				  CmpOp op,
				  void *value);
	RC GetNextRecord(RM_Record &rec);
	RC CloseScan();
}

class RM_Record {
public:
	RM_Record();
	~RM_Record();
	RC GetData(char *&data) const;
	RC GetRid(RID &rid) const;
}

class RID {
public:
	RID();
	~RID();
	RID(PageNum pageNum, SlotNum slotNum);
	RC GetPageNum(PageNum &pageNum) const;
	RC GetSlotNum(SlotNum &slotNum) const;
}

~~~


## Index Manager


## Buffer Manager

