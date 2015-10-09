# Interface Conventiions

## Interpreter

## API

```
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

class QL_Manager {
public:
	QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
	~QL_Manager;
	RC Select (vector<RelationAttr> selectAttrs,
				 vector<string> relations,
				 vector<Condition> conditions);
	RC Insert (string relationName, 
				 vector<Value> values);
	RC Delete (string relationName,
				 vector<Condition> conditions);
	RC Update (string relationName,
				 RelationAttr &updateAttr,
				 bool isValue, // true if right-hand side is a value
				 RelationAttr &rAttr,
				 Value &rValue,
				 vector<Condition> conditions);
};
```


## Catalog Manager

## Record Manager

## Index Manager

## Buffer Manager

