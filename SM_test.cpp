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

    AttrInfo s1("studentID", INT, 4);
    AttrInfo s2("name", CHARN, 10);
    attrs.push_back(s1);
    attrs.push_back(s2);

    if((rc = smm.OpenDb("myDB")) != 0)
        return rc;
    if((rc = smm.CreateTable(relationName, attrs)) != 0)
        return rc;

    cout << "hello" << endl;
    return 0;
}
