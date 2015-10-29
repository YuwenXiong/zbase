//
// Created by Orpine on 10/28/15.
//

#include "parser.h"
#include <iostream>
#include <exception>
using namespace std;

State parseState;


void parser() {
    for(;;) {
        try {
            if (yyparse() == 0 && parseState.type != EMPTY) {
                if (parseState.type == EXIT) {
                    break;
                }
                cout <<"type " <<  parseState.type << endl;
                cout <<"relationName " << parseState.relationName << endl;
                cout <<"attrName " << parseState.attrName << endl;
                cout <<"indexName " << parseState.indexName << endl;
                cout << "attrs: " << endl;
                for (auto x: parseState.attrs) {
                    cout << x.attrName << ' ' << x.attrLength << ' ' << x.attrType << ' ' << x.property << endl;
                }
                cout << "conditions: " << endl;
                for (auto x: parseState.conditions) {
                    cout << x.lAttr << ' ' << x.op << ' ' << x.rValue.type << ' ' << x.rValue.iData << ' ' << x.rValue.fData << ' ' << x.rValue.strData << endl;
                }
                cout << "values: " << endl;
                for (auto x: parseState.values) {
                    cout << x.type << ' ' << x.iData << ' ' << x.fData << ' ' << x.strData << endl;
                }
            }
        } catch (const char *s) {
            printf("error! %s\n", s);
        }
    }
}