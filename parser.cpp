//
// Created by Orpine on 10/28/15.
//

#include "parser.h"
#include <iostream>
#include <exception>
#include <assert.h>
#include "SM.h"
#include "IX.h"
#include "RM.h"
#include "PF.h"
#include "QL.h"
using namespace std;

State parseState;

void parser() {
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    IX_Manager ixm(pfm);
    SM_Manager smm(ixm, rmm);
    QL_Manager qlm(smm, ixm, rmm);
    RC rc;
    extern int yy_flex_debug;
    yy_flex_debug = 1;

//    FILE *fp;
    if ((fopen("relcat", "r")) == NULL) {
        smm.CreateDb("ss");
    }
    smm.OpenDb("ss");

    for(;;) {
        try {
            if (yyparse() == 0 && parseState.type != EMPTY) {
//            parseState.relationName = "a";
//            parseState.type = CREATETABLE;
//            AttrInfo x;
//            x.attrName = "s";
//            x.attrLength = 4;
//            x.attrType = INT;
//            x.property = NONE;
//            parseState.attrs.push_back(x);
                if (parseState.type == EXIT) {
                    smm.CloseDb();
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
                    cout << x.lAttr.attrName << ' ' << x.op << ' ' << x.rValue.type << ' ' << x.rValue.iData << ' ' << x.rValue.fData << ' ' << x.rValue.strData << endl;
                }
                cout << "values: " << endl;
                for (auto x: parseState.values) {
                    cout << x.type << ' ' << x.iData << ' ' << x.fData << ' ' << x.strData << endl;
                }
                switch (parseState.type) {
                    case CREATETABLE:
                        if ((rc = smm.CreateTable(parseState.relationName, parseState.attrs))) {
                            switch (rc) {
                                default: {
                                    printf("%d\n", rc);
                                }
                            }
                        }
                        break;
                    case DROPTABLE:
                        if ((rc = smm.DropTable(parseState.relationName))) {
                            switch (rc) {
                                default: {
                                    printf("%d\n", rc);
                                }
                            }
                        }
                        break;
                    case CREATEINDEX:
                        if ((rc = smm.CreateIndex(parseState.relationName, parseState.attrName, parseState.indexName))) {
                            switch (rc) {
                                case SM_INDEXEXISTS: {
                                    printf("index on %s already exists!\n", parseState.attrName.c_str());
                                }
                                default: {
                                    printf("%d\n", rc);
                                }
                            }
                        }
                        break;
                    case DROPINDEX:
                        if ((rc = smm.DropIndex(parseState.indexName))) {
                            switch (rc) {
                                case SM_NOTFOUND: {
                                    printf("index name not found!\n");
                                }
                                default: {
                                    printf("%d\n", rc);
                                }
                            }
                        }
                        break;
                    case SELECT:
                        if ((rc = qlm.Select(parseState.attrs, parseState.relationName, parseState.conditions))) {
                            switch (rc) {
                                default: {
                                    printf("%d\n", rc);
                                }
                            }
                        }
                        break;
                    case INSERT:
                        if ((rc = qlm.Insert(parseState.relationName, parseState.values))) {
                            switch (rc) {
                                default: {
                                    printf("%d\n", rc);
                                }
                            }
                        }
                        break;
                    case DELETE:
                        if ((rc = qlm.Delete(parseState.relationName, parseState.conditions))) {
                            switch (rc) {
                                default: {
                                    printf("%d\n", rc);
                                }
                            }
                        }
                        break;
                    default:
                        throw "error command";
                }

            }
        } catch (const char *s) {
            printf("error! %s\n", s);
        }
    }
}