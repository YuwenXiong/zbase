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
    vector<AttrInfo> attrs;
    vector<AttrCatRecord> attrs2;
    AttrCatRecord attrRecord;

    AttrInfo s1("studentID", INT, 4);
    AttrInfo s2("name", CHARN, 10);
    attrs.push_back(s1);
    attrs.push_back(s2);

    if((rc = smm.CreateDb("myDB")) != 0)
        return rc;
    if((rc = smm.OpenDb("myDB")) != 0)
        return rc;
    if((rc = smm.CreateTable("test", attrs)) != 0)
        return rc;
    if((rc = smm.GetAttrInfo("test", "studentID", attrRecord)) != 0)
        return rc;
    if((rc = smm.GetAttrInfo("test", 2, attrs2)))
        return rc;

    print(attrs2);

    if ((rc = smm.CloseDb())) {
        return rc;
    }
    return 0;
}