#include "SM.h"
#include <iostream>

int main() {
    RC rc;
    PF_Manager pfm;
    IX_Manager ixm(pfm);
    RM_Manager rmm(pfm);
    SM_Manager smm(ixm, rmm);
    string relationName = "test";
    vector<AttrInfo> attrs;
    AttrCatRecord attrRecord;

    AttrInfo s1("studentID", INT, 4);
    AttrInfo s2("name", CHARN, 10);
    attrs.push_back(s1);
    attrs.push_back(s2);

    if((rc = smm.CreateDb("myDB")) != 0)
        return rc;
    if((rc = smm.OpenDb("myDB")) != 0)
        return rc;
    if((rc = smm.CreateTable(relationName, attrs)) != 0)
        return rc;
    if((rc = smm.GetAttrInfo("test", "studentID", attrRecord)) != 0)
        return rc;

    cout << attrRecord.attrName << endl;
    cout << attrRecord.relationName << endl;

    if ((rc = smm.CloseDb())) {
        return rc;
    }
    return 0;
}
