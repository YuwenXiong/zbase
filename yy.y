%{
#include <cstdio>
#include <exception>
#include "common.h"
extern State parseState;

%}


%token
    T_CREATE
    T_DROP
    T_TABLE
    T_INDEX
    T_SELECT
    T_FROM
    T_WHERE
    T_AND
    T_INSERT
    T_INTO
    T_VALUES
    T_DELETE
    T_LT
    T_LE
    T_GT
    T_GE
    T_EQ
    T_NE
    T_INT
    T_FLOAT
    T_CHAR
    T_SINGLE_QUOTE
    T_DOUBLE_QUOTE
    T_COMMA
    T_LEFT_BRACKET
    T_RIGHT_BRACKET
    T_STAR
    T_SEMICOLON
    T_UNIQUE
    T_PRIMARY
    T_KEY
    T_ON
    T_EOF
    T_EXIT
    T_ABORT

%token<ival> P_INT NUM
%token<fval> P_FLOAT
%token<cval> OP
%token<sval> IDENTIFIER ANYTOKEN
%type<pval> property
%type<tval>
    start
    command
    ddl
    dml
    createTable
    dropTable
    createIndex
    dropIndex
    select
    insert
    delete
    attrList
    attrCat
    valueList
    value
    whereClause
    condList
    cond
%%

start
    : command T_SEMICOLON {
        parseState = $1;
        YYACCEPT;
    }
    | T_EOF {
        parseState.type = EXIT;
        YYACCEPT;
    }
    | T_ABORT {
        YYABORT;
    };

command
    : ddl
    | dml
    | T_EXIT {
        $$.type = EXIT;
    }
    | {
        $$.type = EMPTY;
    };

ddl
    : createTable
    | dropTable
    | createIndex
    | dropIndex;

dml
    : select
    | insert
    | delete;

createTable
    : T_CREATE T_TABLE IDENTIFIER T_LEFT_BRACKET attrList T_RIGHT_BRACKET {
        $$.type = CREATETABLE;
        $$.relationName = $3;
        $$.attrs = $5.attrs;
    };

dropTable
    : T_DROP T_TABLE IDENTIFIER {
        $$.type = DROPTABLE;
        $$.relationName = $3;
    };

createIndex
    : T_CREATE T_INDEX IDENTIFIER T_ON IDENTIFIER T_LEFT_BRACKET IDENTIFIER T_RIGHT_BRACKET {
        $$.type = CREATEINDEX;
        $$.indexName = $3;
        $$.relationName = $5;
        $$.attrName = $7;
    };

dropIndex
    : T_DROP T_INDEX IDENTIFIER {
        $$.type = DROPINDEX;
        $$.indexName = $3;
    };

select
    : T_SELECT T_STAR T_FROM IDENTIFIER whereClause {
        $$.type = SELECT;
        $$.relationName = $4;
        if ($5.type != EMPTY)
            $$.conditions = $5.conditions;
    };

insert
    : T_INSERT T_INTO IDENTIFIER T_VALUES T_LEFT_BRACKET valueList T_RIGHT_BRACKET {
        $$.type = INSERT;
        $$.relationName = $3;
        $$.values = $6.values;
    };


delete
    : T_DELETE T_FROM IDENTIFIER whereClause {
        $$.type = DELETE;
        $$.relationName = $3;
        if ($4.type != EMPTY)
            $$.conditions = $4.conditions;
    };

attrList
    : attrList T_COMMA attrCat {
        $$ = $1;
        $$.attrs.push_back($3.attrs[0]);
    }
    | attrList T_COMMA T_PRIMARY T_KEY T_LEFT_BRACKET IDENTIFIER T_RIGHT_BRACKET {
        $$ = $1;
        for (auto &x: $$.attrs) {
            if (x.attrName == $6) {
                x.property = PRIMARY;
                break;
            }
        }
    }
    | attrCat {
        $$ = $1;
    };

attrCat
    : IDENTIFIER T_CHAR T_LEFT_BRACKET NUM T_RIGHT_BRACKET property {
        $$.attrs.push_back(AttrInfo($1, CHARN, $4, $6));
    }
    | IDENTIFIER T_INT property{
        $$.attrs.push_back(AttrInfo($1, INT, sizeof(int), $3));
    }
    | IDENTIFIER T_FLOAT property {
        $$.attrs.push_back(AttrInfo($1, FLOAT, sizeof(float), $3));
    };

valueList
    : valueList T_COMMA value {
        $$.values = $1.values;
        $$.values.push_back($3.values[0]);
    }
    | value {
        $$.values.push_back($1.values[0]);
    };

value
    : ANYTOKEN {
        $$.values.push_back(Value($1.substr(1, $1.length() - 2)));
        $$.type = VALUE;
    }
    | P_INT {
        $$.values.push_back(Value($1));
        $$.type = VALUE;
    }
    | NUM {
        $$.values.push_back(Value($1));
        $$.type = VALUE;
    }
    | P_FLOAT {
        $$.values.push_back(Value($1));
        $$.type = VALUE;
    };

whereClause
    : T_WHERE condList {
        $$.conditions = $2.conditions;
    }
    | { // epsilon
        $$.type = EMPTY;
    };

condList
    : condList T_AND cond {
        $$.conditions = $1.conditions;
        $$.conditions.push_back($3.conditions[0]);
    }
    | cond {
        $$.conditions = $1.conditions;
    };

cond
    : IDENTIFIER OP value {
        $$.conditions.push_back(Condition($1, $2, $3.values[0]));
    };

property
    : T_UNIQUE {
        $$ = UNIQUE
    }
    | {
        $$ = NONE;
    };


%%

void yyerror(char const *s) {
    throw s;
}

int yywrap(void)
{
   return 1;
}
