%{
    #define YYLTYPE YYLTYPE
    typedef struct YYLTYPE {
        int first_line;
        int first_column;
        int last_line;
        int last_column;
        const char *filename;
    } YYLTYPE;

    #include "obj_parser.h"
    #include "obj_scanner.h"

    #define YYERROR_VERBOSE

    // Uncomment the following line to enable Bison debug tracing
    // int yydebug = 1;

    void
    yyerror(YYLTYPE *locp,
        Model *model,
        const char *filename,
        yyscan_t scanner,
        const char *msg);
%}

%code provides{
    #define YY_DECL int yylex (YYSTYPE *yylval_param, \
        YYLTYPE *yylloc_param, \
        const char *filename, \
        yyscan_t yyscanner)
    YY_DECL;
}

%code requires{
    #include <CL/cl_gl.h>

    #include "model.h"
    #include "vector.h"

    # define YYLLOC_DEFAULT(Cur, Rhs, N)                            \
        do {                                                        \
          if (N)                                                    \
            {                                                       \
              (Cur).first_line   = YYRHSLOC(Rhs, 1).first_line;     \
              (Cur).first_column = YYRHSLOC(Rhs, 1).first_column;   \
              (Cur).last_line    = YYRHSLOC(Rhs, N).last_line;      \
              (Cur).last_column  = YYRHSLOC(Rhs, N).last_column;    \
            }                                                       \
          else                                                      \
            {                                                       \
              (Cur).first_line   = (Cur).last_line   =              \
                YYRHSLOC(Rhs, 0).last_line;                         \
              (Cur).first_column = (Cur).last_column =              \
                YYRHSLOC(Rhs, 0).last_column;                       \
            }                                                       \
          (Cur).filename = YYRHSLOC(Rhs, 1).filename;               \
        } while (0)

    #ifndef YY_TYPEDEF_YY_SCANNER_T
    #define YY_TYPEDEF_YY_SCANNER_T
    typedef void* yyscan_t;
    #endif
}

%define api.pure full
%define parse.error verbose
%define parse.trace
%define api.token.prefix {T_}
%locations
%parse-param { Model *model }
%param { const char *filename } { yyscan_t scanner }

%union {
    int ival;
    double dval;
    char *sval;
    char **group;
    cl_int3 i3;
}

%token ERROR "parse error"
       V     "v"
       F     "f"
       S     "s"
       VT    "vt"
       VN    "vn"
%token<ival> INT "int"
%token<dval> DOUBLE "double"
%token<sval> IDENT "ident"
%token<group> GROUP "group"

%type<i3> v_index triangle polygon

%start file

%initial-action {
    @$ = (YYLTYPE) {
        1, 1, 1, 1, filename
    };
}

%%

file
    : opt_lines

opt_lines
    : %empty
    | lines

lines
    : line
    | lines line

line
    : GROUP
    | smooth
    | vertex
    | tex
    | norm
    | polygon {
        append_Model_tri(model, $1);
    }

smooth
    : S INT

vertex
    : V DOUBLE DOUBLE DOUBLE {
        append_Model_vert(model, (cl_double4){ .s={$2, $3, $4, 1.0 } });
    }
    | V DOUBLE DOUBLE DOUBLE DOUBLE {
        append_Model_vert(model, (cl_double4){ .s={$2, $3, $4, $5 } });
    }

tex
    : VT DOUBLE
    | VT DOUBLE DOUBLE
    | VT DOUBLE DOUBLE DOUBLE

norm
    : VN DOUBLE DOUBLE DOUBLE

polygon
    : triangle
    | polygon v_index {
        append_Model_tri(model, $1);
        $$ = (cl_int3){ { $1.s[0], $1.s[2], $2.s[0] } };
    }

triangle
    : F v_index v_index v_index {
        $$ = (cl_int3){ { $2.s[0], $3.s[0], $4.s[0] } };
    }



v_index
    : INT {
        $$ = (cl_int3){ { $1, 0, 0 } };
    }
    | INT '/' INT {
        $$ = (cl_int3){ { $1, $3, 0 } };
    }
    | INT '/' INT '/' INT {
        $$ = (cl_int3){ { $1, $3, $5 } };
    }
    | INT '/' '/' INT {
        $$ = (cl_int3){ { $1, 0, $4 } };
    }

%%

void yyerror(YYLTYPE *locp,
    Model *model,
    const char *filename,
    yyscan_t scanner,
    const char *msg
) {
    fprintf(stderr,
        "%s:%d:%d: %s\n",
        filename,
        locp->first_line,
        locp->first_column,
        msg);
}