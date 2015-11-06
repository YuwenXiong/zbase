%{
    #include <stdio.h>
    #include <string.h>
    #include <algorithm>
    #include "common.h"
    #include "yy.tab.hpp"
    #include <vector>
    using namespace std;
    int checkToken(string s);
%}

anytoken    ({single_quote}([^\'])+{single_quote})|{double_quote}([^\"])+{double_quote}

nondigit    [_A-Za-z]

digit       [0-9]
num         {digit}+
int         [+\-]?{num}
float       {int}\.{digit}*
floate      {int}\.{num}[Ee]{int}
identifier  {nondigit}({nondigit}|{digit})*
single_quote '
double_quote \"
token       ({int}{identifier})|{identifier}


%%

[ \n\t] {}

{anytoken} {
    yylval.sval = yytext;
    return ANYTOKEN;
}

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

. {
    printf("yytext: %s, command error!", yytext);
    return T_ABORT;
}

<<EOF>> {
    return T_EOF;
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
    if (s == "exec")
        return T_EXECFILE;

    yylval.sval = bs;
    return IDENTIFIER;
}

void yyerror(char const *s) {
    string t1 = s, t2 = yytext;
    char c;
    if (t2 != ";")
        while(yylex() != T_SEMICOLON);
    if (strcmp(s, "Invalid string length!")) {
        printf("%s near '%s'\n", s, t2.c_str());
    } else {
        puts(s);
    }
}

int yywrap(void)
{
   yyin = stdin;
   return 0;
}
