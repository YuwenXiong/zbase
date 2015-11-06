#include "SM.h"
#include "QL.h"
#include <iostream>

void print(const vector<AttrCatRecord> &v) {
    cout << v[0].relationName << endl;

    for(int i = 0; i < v.size(); i++) {
        cout << v[i].attrName << endl;
    }
}

int main() {
    RC rc;
    PF_Manager pfm;
    IX_Manager ixm(pfm);
    RM_Manager rmm(pfm);
    SM_Manager smm(ixm, rmm);
    vector<AttrInfo> attrs1, attrs2;
    vector<AttrCatRecord> attrRecords1, attrRecords2;
    AttrCatRecord attrRecord;
    QL_Manager qlm(smm, ixm, rmm);
    Value value;
    vector<Value> values[10];
    AttrCatRecord attrData;

    AttrInfo s1("ID", INT, 4, UNIQUE);
    AttrInfo s2("name", CHARN, 10);
    attrs1.push_back(s1);
    attrs1.push_back(s2);

    AttrInfo s3("ID", INT, 4, PRIMARY);
    AttrInfo s4("name", CHARN, 8);
    AttrInfo s5("salary", FLOAT, 4);
    attrs2.push_back(s3);
    attrs2.push_back(s4);
    attrs2.push_back(s5);

    if((rc = smm.CreateDb("myDB")) != 0)
        return rc;
    if((rc = smm.OpenDb("myDB")) != 0)
        return rc;

    if((rc = smm.CreateTable("Student", attrs1)) != 0)
        return rc;
    if((rc = smm.CreateTable("Teacher", attrs2)) != 0)
        return rc;

//    if((rc = smm.DropTable("Student")))
//        return rc;
//
//    rc = smm.GetAttrInfo("Student", 2, attrRecords1);
//    if(rc == SM_NOTFOUND)
//        cout << "Not Found!" << endl;
//    else if((rc))
//        return rc;
//
//    rc = smm.GetAttrInfo("Teacher", 3, attrRecords2);
//    if(rc == SM_NOTFOUND)
//        cout << "Not Found!" << endl;
//    else if((rc))
//        return rc;
//
//    print(attrRecords1);
//    print(attrRecords2);

    if((rc = smm.CreateIndex("Student", "ID", "StuIndex")))
        return rc;

//    for(int i = 0; i < 10; i++) {
//        value.type = INT;
//        value.iData = i;
//        values[i].push_back(value);
//        value.type = CHARN;
//        value.strData = "Tom" + i;
//        values[i].push_back(value);
//        if ((rc = qlm.Insert("Student", values[i])))
//            return rc;
//    }
//
    if((rc = smm.DropIndex("StuIndex")))
        return rc;
//
//    smm.GetAttrInfo("Student", "ID", attrData);
//    cout << attrData.indexNo << endl;

    if ((rc = smm.CloseDb())) {
        return rc;
    }
    return 0;
}