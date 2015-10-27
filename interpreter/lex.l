%{
    #include <stdio.h>
    #include <string.h>
        
%}

nondigit    ([_A-Za-z])

digit       ([0-9])

num         ({digit}+)
s_num       [+\-]?{num}
identifier  {nondigit}({nondigit}|{digit})*

%%

{identifier} {
    printf("identifier\n");
}

{s_num} {
    printf("num\n");
}
