%{
#include <cstdio>
#include "common.h"
%}


%token
    CREATE
    DROP
    TABLE
    INDEX
    SELECT
    FROM
    WHERE
    AND
    INSERT
    INTO
    LT
    LE
    GT
    GE
    EQ
    NE

%token<ival> INT
%token<fval> FLOAT
%token<cval> op
%token<tval>
    createtable
    droptable
    createindex
    dropindex
    select
    insert
    delete


