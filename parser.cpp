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
int fflag;

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
    cout << ">> ";
    for(int i = 0;;i++) {
//        cout << i << " ";
        if (yyparse() == 0 && parseState.type != EMPTY) {
            if (parseState.type == EXIT) {
                smm.CloseDb();
                break;
            }
//                cout <<"type " <<  parseState.type << endl;
//                cout <<"relationName " << parseState.relationName << endl;
//                cout <<"attrName " << parseState.attrName << endl;
//                cout <<"indexName " << parseState.indexName << endl;
//                cout << "attrs: " << endl;
//                for (auto x: parseState.attrs) {
//                    cout << x.attrName << ' ' << x.attrLength << ' ' << x.attrType << ' ' << x.property << endl;
//                }
//                cout << "conditions: " << endl;
//                for (auto x: parseState.conditions) {
//                    cout << x.lAttr.attrName << ' ' << x.op << ' ' << x.rValue.type << ' ' << x.rValue.iData << ' ' << x.rValue.fData << ' ' << x.rValue.strData << endl;
//                }
//                cout << "values: " << endl;
//                for (auto x: parseState.values) {
//                    cout << x.type << ' ' << x.iData << ' ' << x.fData << ' ' << x.strData << endl;
//                }
            switch (parseState.type) {
                case CREATETABLE:
                    if ((rc = smm.CreateTable(parseState.relationName, parseState.attrs))) {
                        switch (rc) {
                            case SM_TABLEEXISTS: {
                                printf("Relation '%s' already exists!\n", parseState.relationName.c_str());
                                break;
                            }
                            default: {
                                printf("%d\n", rc);
                            }
                        }
                    }
                    break;
                case DROPTABLE:
                    if ((rc = smm.DropTable(parseState.relationName))) {
                        switch (rc) {
                            case SM_NOTFOUND: {
                                printf("Relation '%s' not found!\n", parseState.relationName.c_str());
                                break;
                            }
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
                                printf("Index on '%s' already exists!\n", parseState.attrName.c_str());
                                break;
                            }
                            case SM_NOTFOUND: {
                                printf("Relation '%s' not found!\n", parseState.relationName.c_str());
                                break;
                            }
                            case SM_INDEXNAMEEXISTS: {
                                printf("Index name '%s' already exists!\n", parseState.indexName.c_str());
                                break;
                            }
                            case SM_NOTUNIQUE: {
                                printf("Attribute '%s' is not unique!\n", parseState.attrName.c_str());
                                break;
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
                                printf("Index '%s' not found!\n", parseState.indexName.c_str());
                                break;
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
                            case SM_NOTFOUND: {
                                printf("Table '%s' not found!\n", parseState.relationName.c_str());
                                break;
                            }
                            case QL_INVALID_WHERE_CLAUSE: {
                                printf("Invalid condition\n");
                                break;
                            }
                            default: {
                                printf("%d\n", rc);
                            }
                        }
                    }
                    break;
                case INSERT:
                    if ((rc = qlm.Insert(parseState.relationName, parseState.values))) {
                        switch (rc) {
                            case SM_NOTFOUND: {
                                printf("table '%s' not found!\n", parseState.relationName.c_str());
                                break;
                            }
                            case QL_INVALID_ATTR_COUNT: {
                                printf("Invalid attribute num. Please recheck the attribute num of '%s'\n", parseState.relationName.c_str());
                                break;
                            }
                            case QL_INVALID_ATTR_TYPE: {
                                printf("Invalid attribute type. Please recheck the attribute type of '%s'\n", parseState.relationName.c_str());
                                break;
                            }
                            case QL_UNIQUE_VALUE_EXISTS: {
                                printf("Trying to insert same value to a unique attribuate\n");
                                break;
                            }
                            default: {
                                printf("%d\n", rc);
                            }
                        }
                    }
                    break;
                case DELETE:
                    if ((rc = qlm.Delete(parseState.relationName, parseState.conditions))) {
                        switch (rc) {
                            case SM_NOTFOUND: {
                                printf("table '%s' not found!\n", parseState.relationName.c_str());
                                break;
                            }
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
        if (fflag == 0)
            cout << ">> ";
    }
}