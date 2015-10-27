%{
    #include <stdio.h>
    #include <string.h>
    #include <algorithm>
    #include "common.h"
    #include "yy.tab.h"
%}

nondigit    [_A-Za-z]

digit       [0-9]
num         {digit}+
int         [+\-]?{num}
float       {int}\.{num}
floate      {int}\.{num}[Ee]{int}
identifier  {nondigit}({nondigit}|{digit})*

%%

{int} {
    sscanf(yytext, "%d", &yylval.ival);
    return INT;
}

{float} {
    sscanf(yytext, "%f", &yylval.fval);
}

{floate} {
    sscanf(yytext, "%f", &yylval.fval);
}

{identifier} {
    return checkToken(yytext);
}

"<" {
    return LT;
}
"<=" {
    return LE;
}

">" {
    return GT;
}

">=" {
    return GE;
}

"=" {
    return EQ;
}

"!=" {
    return NE;
}

"<>" {
    return NE;
}

. {
    printf("command error!\n");
}

%%

int checkToken(string s) {
    string bs = s;
    std::transfrom(s.begin(), s.end(), s.begin(), tolower);
    yylval.sval = s;
    if (s == "create")
        return CREATE;
    if (s == "drop")
        return DROP;
    if (s == "table")
        return TABLE;
    if (s == "index")
        return INDEX:
    if (s == "select")
        return SELECT;
    if (s == "from")
        return FROM;
    if (s == "where")
        return WHERE;
    if (s == "and")
        return AND;
    if (s == "insert")
        return INSERT;
    if (s == "into")
        return INTO;
    if (s == "values")
        return VALUES:
    if (s == "delete")
        return DELETE;

    yylval.sval = bs;
    return IDENTIFIER;
}