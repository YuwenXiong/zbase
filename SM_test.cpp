#include "SM.h"
#include <iostream>

void print(const vector<AttrCatRecord> &v) {
    for(int i = 0; i < v.size(); i++) {
        cout << v[i].relationName << v[i].attrName << endl;
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

    AttrInfo s1("studentID", INT, 4);
    AttrInfo s2("name", CHARN, 10);
    attrs1.push_back(s1);
    attrs1.push_back(s2);

    AttrInfo s3("techerID", FLOAT, 4);
    AttrInfo s4("name", CHARN, 8);
    AttrInfo s5("salary", INT, 4);
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

    if((rc = smm.DropTable("Student")))
        return rc;

    rc = smm.GetAttrInfo("Student", 2, attrRecords1);
    if((rc == SM_NOTFOUND))
        cout << "Not Found!" << endl;
    else if((rc))
        return rc;

    rc = smm.GetAttrInfo("Teacher", 3, attrRecords2);
    if((rc == SM_NOTFOUND))
        cout << "Not Found!" << endl;
    else if((rc))
        return rc;

    print(attrRecords1);
    print(attrRecords2);

    if ((rc = smm.CloseDb())) {
        return rc;
    }
    return 0;
}