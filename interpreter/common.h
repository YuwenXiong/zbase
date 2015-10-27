//
// Created by Orpine on 10/27/15.
//

#ifndef ZBASE_COMMON_H
#define ZBASE_COMMON_H

#include <string>
#include "parser.h"
using namespace std;

struct Type {
    int ival;
    float fval;
    string sval;
    ComOp cval;
    State tval;
};

#define YYSTYPE Type

extern "C" {
    int yywarp(void);
    extern int yylex(void);
}

#endif //ZBASE_COMMON_H
