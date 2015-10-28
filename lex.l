%{
    #include <stdio.h>
    #include <string.h>
    #include <algorithm>
    #include "common.h"
    #include "yy.tab.h"
    #include <vector>
    using namespace std;
    int checkToken(string s);
%}

nondigit    [_A-Za-z]

digit       [0-9]
num         {digit}+
int         [+\-]?{num}
float       {int}\.{num}
floate      {int}\.{num}[Ee]{int}
identifier  {nondigit}({nondigit}|{digit})*


%%

[ \n\t] {}

{num} {
    sscanf(yytext, "%d", &yylval.ival);
    return NUM;
}

{int} {
    sscanf(yytext, "%d", &yylval.ival);
    return P_INT;
}

{float} {
    sscanf(yytext, "%f", &yylval.fval);
    return P_FLOAT;
}

{floate} {
    sscanf(yytext, "%f", &yylval.fval);
    return P_FLOAT;
}

{identifier} {
    return checkToken(yytext);
}

"<" {
    yylval.cval = LT;
    return OP;
}
"<=" {
    yylval.cval = LE;
    return OP;
}

">" {
    yylval.cval = GT;
    return OP;
}

">=" {
    yylval.cval = GE;
    return OP;
}

"=" {
    yylval.cval = EQ;
    return OP;
}

"!=" {
    yylval.cval = NE;
    return OP;
}

"<>" {
    yylval.cval = NE;
    return OP;
}

"'" {
    return T_SINGLE_QUOTE;
}

\" {
    return T_DOUBLE_QUOTE;
}

"(" {
    return T_LEFT_BRACKET;
}

")" {
    return T_RIGHT_BRACKET;
}

"," {
    return T_COMMA;
}

"*" {
    return T_STAR;
}

";" {
    return T_SEMICOLON;
}


<<EOF>> {
    return T_EOF;
}

. {
    printf("yytext: %s command error!\n", yytext);
    return T_ABORT;
}



%%

int checkToken(string s) {
    string bs = s;
    std::transform(s.begin(), s.end(), s.begin(), tolower);
    yylval.sval = s;
    if (s == "create")
        return T_CREATE;
    if (s == "drop")
        return T_DROP;
    if (s == "table")
        return T_TABLE;
    if (s == "index")
        return T_INDEX;
    if (s == "select")
        return T_SELECT;
    if (s == "from")
        return T_FROM;
    if (s == "where")
        return T_WHERE;
    if (s == "and")
        return T_AND;
    if (s == "insert")
        return T_INSERT;
    if (s == "into")
        return T_INTO;
    if (s == "values")
        return T_VALUES;
    if (s == "delete")
        return T_DELETE;
    if (s == "int")
        return T_INT;
    if (s == "float")
        return T_FLOAT;
    if (s == "char")
        return T_CHAR;
    if (s == "unique")
        return T_UNIQUE;
    if (s == "primary")
        return T_PRIMARY;
    if (s == "key")
        return T_KEY;
    if (s == "on")
        return T_ON;
    if (s == "exit")
        return T_EXIT;

    yylval.sval = bs;
    return IDENTIFIER;
}