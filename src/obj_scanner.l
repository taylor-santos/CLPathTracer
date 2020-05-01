%{
    #include "obj_parser.h"
    #include "vector.h"
    #include "util.h"

    #include <errno.h>
    #include <limits.h>
    #include <unistd.h>

    #define YY_USER_ACTION                                                  \
        yylloc->first_line = yylloc->last_line;                             \
        yylloc->first_column = yylloc->last_column;                         \
        if (yylloc->last_line == yylineno)                                  \
            yylloc->last_column += yyleng;                                  \
        else {                                                              \
            yylloc->last_line = yylineno;                                   \
            yylloc->last_column = yytext + yyleng - strrchr(yytext, '\n');  \
        }

    YY_DECL;
    void yyerror(YYLTYPE *locp,
        const void **root,
        const char *filename,
        yyscan_t scanner,
        const char *msg);

    static char **group;
%}

%option warn
%option nounput
%option nodefault
%option reentrant noyywrap never-interactive
%option bison-bridge bison-locations
%option noinput
%option yylineno
%option noyywrap
%option fast

%x GROUP

%%

<GROUP>{
    [^ \n\r]+ {
        vector_append(group, strdup(yytext));
    }
    " "
    .|[\n\r] {
        yylval->group = group;
        BEGIN(INITIAL);
        return T_GROUP;
    }
}

[ \t\n\r]*
#.*

[+-]?[0-9]+[.][0-9]+(e[+-]?[0-9]+)?  {
    yylval->dval = atof(yytext);
    return T_DOUBLE;
}
[+-]?[0-9]+  {
    yylval->ival = atol(yytext);
    return T_INT;
}
g   {
    group = new_vector();
    BEGIN(GROUP);
}
vt  { return T_VT; }
vn  { return T_VN; }
v   { return T_V; }
f   { return T_F; }
s   { return T_S; }
"/" { return *yytext; }
[a-zA-Z0-9_]+  {
    yylval->sval = yytext;
    return T_IDENT;
}
. {
    char msg[] = "unrecognized symbol \' \'";
    sprintf(msg, "unrecognized symbol \'%c\'", *yytext);
    yyerror(yylloc, NULL, filename, NULL, msg);
    return T_ERROR;
}

%%