//
// Created by Orpine on 11/1/15.
//

#include <assert.h>
#include <iomanip>
#include "QL.h"

Printer::Printer(vector<AttrCatRecord> _attrs):attrs(_attrs) {
}

void Printer::Print(const char *recordData) {
    for (auto x:attrs) {
        switch (x.attrType) {
            case INT: {
                cout << setw(12) << *(int *)(recordData + x.offset);
                break;
            }
            case FLOAT: {
                cout << setw(12) << *(float *)(recordData + x.offset);
                break;
            }
            case CHARN: {
                cout << setw(12) << string(recordData + x.offset);
                break;
            }
            default:
                assert(0);

        }
    }
    cout << endl;
}