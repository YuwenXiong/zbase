# Interface Conventiions

## base type
~~~cpp
enum AttrType {
    INT,
    FLOAT,
    CHARN
};

enum CmpOp {
	EQ, NE, NO
};

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

struct RelationCatRecord {
	string relationName;
	int tupleLength;	// byte size of tuple, used by Record Manager
	int attrCount;
	int indexCount;
};

struct AttrCatRecord {
	string relationName;
	string attrName;
	int offset;
	AttrType attrType;
	int attrLength;
	int indexNo;
};

typedef int RC; // return codes

~~~

## Interpreter

## API

~~~cpp

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
~~~


## Catalog Manager
~~~cpp
// catalog manager should create a special relation to store metadata when create a database
// this job will be done outside the class
class SM_Manager {
public:
	SM_Manager(IX_Manager &ixm, RM_Manager &rmm);
	~SM_Manager();
	RC OpenDb(const string &dbName);
	RC CloseDb();
	RC CreateTable(const string &relationName, 
				   const vector<AttrInfo> &attrs);
	RC DropTable(const string &relationName);
	RC CreateIndex(const string &relationName,
				   const string &attrName);
	RC DropIndex(const string &relationName,
				 const string &attrName);
				 
	RC GetAttrInfo(const string &relationName, int attrCount, vector<AttrCatRecord> &attrs);
	RC GetAttrInfo(const string &relationName, const string &attrName, AttrCatRecord &attrData, RID &rid);
	RC GetRelationInfo(const string &relationName, RelationCatRecord &relationData, RID &rid);
}

~~~

## Record Manager
~~~cpp
class RM_Manager {
public:
	RM_Manager (PF_Manager &pfm);
	~RM_Manager();
	RC CreateFile(const string &fileName, int recordSize);
	RC DestroyFile(const string &fileName);
	RC OpenFile(const string &fileName, RM_FileHandle &fileHandle);
	RC CloseFile(RM_FileHandle &fileHandle);
}

class RM_FileHandle {
public:
	RM_FileHandle();
	~RM_FileHandle();
	RC GetRecord(const RID &rid, RM_Record &rec) const;
	RC InsertRecord(const char* data, RID &rid);
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
				void* ingvalue);
	RC GetNextRecord(RM_Record &record);
	RC CloseScan();
}

class RM_Record {
public:
	RM_Record();
	~RM_Record();
	RC GetData(char* &data) const;
	RC GetRid(RID &rid) const;
}

class RID {
public:
	RID();
	~RID();
	RID(PageNum pageNum, SlotNum slotNum);
	RC GetPageNum(PageNum &pageNum) const;
	RC GetSlotNum(SlotNum &slotNum) const;
}

~~~


## Index Manager
~~~cpp
class IX_Manager {
public:
	IX_Manager(PF_Manager &pfm);
	~IX_Manager();
	RC CreateIndex(const string &fileName,
				   int indexNo,
				   AttrType attrType,
				   int attrLength);
	RC DestoryIndex(const string &fileName,
					int indexNo);
	RC OpenIndex(const string &fileName,
				 int indexNo,
				 IX_IndexHandle &indexHandle);
	RC CloseIndex(IX_IndexHandle &indexHandle);
};

class IX_IndexHandle {
public:
	IX_IndexHandle();
	~IX_IndexHandle();
	RC InsertEntry(void* Data, const RID &rid);
	RC DeleteEntry(void* data, const RID &rid);
	RC ForcePages();
};

class IX_IndexScan {
public:
	IX_IndexScan();
	~IX_IndexScan();
	RC OpenScan(const IX_IndexHandle &indexHandle,
				CmpOp, op,
				void* value);
	RC GetNextEntry(RID &rid);
	RC CloseScan();
};
~~~

## Buffer Manager
~~~cpp
class PF_Manager {
public:
	PF_Manager();
	~PF_Manager();
	RC CreateFile(const string &fileName);
	RC DestoryFile(const string &fileName);
	RC OpenFile(const char &fileName, PF_FileHandle &fileHandle);
	RC CloseFile(PF_FileHandle &fileHandle);
};

class PF_FileHandle {
public:
	PF_FileHandle();
	~PF_FileHandle();
	PF_FileHandle(const PF_FileHandle &fileHandle);
	PF_FileHandle& operator= (const PF_FileHandle &fileHandle);
	
	RC GetFirstPage(PF_PageHandle &pageHandle) const;
	RC GetLastPage(PF_PageHandle &pageHandle) const;
	RC GetNextPage(PageNum current, PF_PageHandle &pageHandle) const;
	RC GetPrevPage(PageNum current, PF_PageHandle &pageHandle) const;
	RC GetThisPage(PageNum pageNum, PF_PageHandle &pageHandle) const;
	
	RC AllocatePage(PF_PageHandle &pageHandle);
	RC DisposePage(PageNum pageNum);
	RC MarkDirty(PageNum pageNum) const;
	RC UnpinPage(PageNum pageNum) const;
	RC ForcePages(PageNum pageNum = ALL_PAGES) const;
};

class PF_PageHandle {
public:
	PF_PageHandle();
	~PF_PageHandle();
	PF_PageHandle(const PF_PageHandle &pageHandle);
	PF_PageHandle& operator=(const PF_PageHandle &pageHandle);
	
	RC GetData(char* &data) const;
	RC GetPageNum(PageNum &pageNum) const;
};

~~~
