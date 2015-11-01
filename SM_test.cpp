#include "SM.h"
#include <iostream>

int main() {
    PF_Manager pfm;
    IX_Manager ixm(pfm);
    RM_Manager rmm(pfm);
    SM_Manager smm(ixm, rmm);

    

    cout << "hello" << endl;
    return 0;
}
