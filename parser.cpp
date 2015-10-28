//
// Created by Orpine on 10/28/15.
//

#include "parser.h"
#include <iostream>
using namespace std;

State parseState;


void parser() {
    for(;;) {
        if (yyparse() == 0 && parseState.type != EMPTY) {
            if (parseState.type == EXIT) {
                break;
            }
            cout << parseState.type << endl;
            cout << parseState.relationName << endl;
            cout << parseState.attrName << endl;
            cout << parseState.indexName << endl;
            for (auto x: parseState.attrs) {
                cout << x.attrName << x.attrLength << x.attrType << x.property << endl;
            }
            for (auto x: parseState.conditions) {
                cout << x.lAttr << x.op << x.rValue.type << x.rValue.iData << x.rValue.fData << x.rValue.strData << endl;
            }
            for (auto x: parseState.values) {
                cout << x.type << x.iData << x.fData << x.strData << endl;
            }
        }
    }
}