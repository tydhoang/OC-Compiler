%{
// Created by:
//  Tyler Hoang (tydhoang@ucsc.edu)
//  Eric Vin (evin@ucsc.edu)
// parser.y

#include <cassert>

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_INT TOK_STRING 
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT 
%token TOK_NULLPTR TOK_ARRAY TOK_ARROW TOK_ALLOC TOK_PTR 
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE TOK_NOT 
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON 
%token TOK_ROOT TOK_BLOCK TOK_CALL TOK_FUNCTION TOK_PARAM TOK_TYPE_ID
%token TOK_POS TOK_NEG

%token TOK_INDEX


%right TOK_IF TOK_ELSE
%right '='
%left  TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left  '+' '-'
%left  '*' '/' '%'
%left  TOK_POS TOK_NEG TOK_NOT
%left  TOK_ARRAY TOK_ARROW TOK_FUNCTION TOK_CALL TOK_ALLOC

%start start

%%
start       : program
            {$$ = $1 = nullptr; }
            ;

program     : program structdef
            {$$ = $1->adopt($2); }
            | program vardecl
            {$$ = $1->adopt($2); }
            | program function
            {$$ = $1->adopt($2); }
            | program error '}'
            {$$ = $1; destroy($3); }
            | program error ';'
            {$$ = $1; destroy($3); }
            |
            {$$ = parser::root; }
            ;

structdef   : TOK_STRUCT TOK_IDENT '{' '}' ';'
            {$$ = $1->adopt($2); 
               destroy($3); destroy($4); destroy($5); }
            | TOK_STRUCT TOK_IDENT '{' structcont '}' ';'
            {$$ = $1->adopt($2); $1->adopt_children($4); 
               destroy($3); destroy($4); destroy($5); destroy($6); }
            ;

structcont  : varemptydec
            {$$ = ((new astree(TOK_FUNCTION, $1->lloc, ""))
               ->adopt($1)); }
            | structcont varemptydec
            {$$ = $1->adopt($2); }
            ;

type        : plaintype
            {$$ = $1; }
            | TOK_ARRAY TOK_LT plaintype TOK_GT
            {$$ = $1->adopt($3); destroy($2); destroy($4); }
            ;

plaintype   : TOK_VOID
            {$$ = $1; }
            | TOK_INT
            {$$ = $1; }
            | TOK_STRING
            {$$ = $1; }
            | TOK_PTR TOK_LT TOK_STRUCT TOK_IDENT TOK_GT
            {$$ = $1->adopt($4); 
               destroy($2); destroy($3); destroy($5); }
            ;

function    : varname params block
            {$$ = ((new astree(TOK_FUNCTION, $1->lloc, ""))
               ->adopt($1, $2))->adopt($3); }
            | varname params ';'
            {$$ = (new astree(TOK_FUNCTION, $1->lloc, ""))
               ->adopt($1, $2); destroy($3); }
            ;

params      : '(' ')'
            {$1->symbol = TOK_PARAM; $$ = $1; destroy($2); }
            | '(' varname
            {$1->symbol = TOK_PARAM; $$ = $1->adopt($2); }
            | params ',' varname
            {$$ = $1->adopt($3); destroy($2); }
            | params ')'
            {$$ = $1; destroy($2); }
            ;

block       : ';'
            {$$ = $1; }
            | '{' '}'
            {$1->symbol = TOK_BLOCK; destroy($2); }
            | '{' statements '}'
            {$1->symbol = TOK_BLOCK; $$ = $1->adopt_children($2);
               destroy($2); destroy($3); }
            ;

statements  : statement
            {$$ = (new astree(TOK_ROOT, {0,0,0}, ""))
               ->adopt($1); }
            | statements statement
            {$$ = $1->adopt($2); }
            ;

statement   : vardecl
            {$$ = $1; }
            | block
            {$$ = $1; }
            | while
            {$$ = $1; }
            | ifelse
            {$$ = $1; }
            | return
            {$$ = $1; }
            | expr ';'
            {$$ = $1; destroy($2); }
            ;

vardecl     : varemptydec
            {$$ = $1; }
            | varname '=' expr ';'
            {$$ = $1->adopt($3); destroy ($2); destroy ($4); }
            ;

varemptydec : varname ';'
            {$$ = $1; destroy($2); }
            ;

varname     : type TOK_IDENT
            {$$ = (new astree(TOK_TYPE_ID, $1->lloc, ""))
               ->adopt($1, $2); }
            ;

while       : TOK_WHILE '(' expr ')' statement
            {$$ = $1->adopt ($3, $5); destroy ($2); destroy ($4); }
            ;

ifelse      : TOK_IF '(' expr ')' statement TOK_ELSE statement
            {$$ = $1->adopt ($3, $5); $1->adopt ($7); 
               destroy ($2); destroy ($4); destroy ($6); }
            | TOK_IF '(' expr ')' statement
            {$$ = $1->adopt ($3, $5); destroy ($2); destroy ($4); }
            ;

return      : TOK_RETURN expr ';'
            {$$ = $1->adopt ($2); destroy ($3); }
            | TOK_RETURN ';'
            {$$ = $1; destroy ($2); }
            ;

expr        : expr '=' expr
            {$$ = $2->adopt ($1, $3); }
            | expr TOK_EQ expr
            {$$ = $2->adopt ($1, $3); }
            | expr TOK_NE expr
            {$$ = $2->adopt ($1, $3); }
            | expr TOK_LT expr
            {$$ = $2->adopt ($1, $3); }
            | expr TOK_GT expr
            {$$ = $2->adopt ($1, $3); }
            | expr TOK_LE expr
            {$$ = $2->adopt ($1, $3); }
            | expr TOK_GE expr
            {$$ = $2->adopt ($1, $3); }
            | expr '+' expr
            {$$ = $2->adopt ($1, $3); }
            | expr '-' expr
            {$$ = $2->adopt ($1, $3); }
            | expr '*' expr
            {$$ = $2->adopt ($1, $3); }
            | expr '/' expr
            {$$ = $2->adopt ($1, $3); }
            | expr '%' expr
            {$$ = $2->adopt ($1, $3); }
            | '+' expr
            {$$ = $1->adopt_sym ($2, TOK_POS); }
            | '-' expr
            {$$ = $1->adopt_sym ($2, TOK_NEG); }
            | TOK_NOT expr
            {$$ = $1->adopt ($2); }
            | '(' expr ')'
            {destroy ($1, $3); $$ = $2; }
            | allocator
            {$$ = $1; }
            | call
            {$$ = $1; }
            | variable
            {$$ = $1; }
            | constant
            {$$ = $1; }
            ;

allocator   : TOK_ALLOC TOK_LT TOK_STRING TOK_GT '(' expr ')'
            {$$ = $1->adopt ($3, $6); 
               destroy ($2); destroy ($4); destroy ($5); destroy ($7); }
            | TOK_ALLOC TOK_LT TOK_STRUCT TOK_IDENT TOK_GT '('')'
            {$$ = $1->adopt ($4); 
               destroy ($2); destroy ($3); destroy ($5); destroy ($6); 
                  destroy ($7); }
            | TOK_ALLOC TOK_LT arraytype TOK_GT '(' expr ')'
            {$$ = $1->adopt ($3, $6); 
               destroy ($2); destroy ($4); destroy ($5); destroy ($7); }
            ;

arraytype   : TOK_ARRAY TOK_LT plaintype TOK_GT
            {$$ = $1->adopt($3); destroy($2); destroy($4); }
            ;

call        : TOK_IDENT '(' ')'
            {$$ = $1; destroy($2); destroy($3); }
            | TOK_IDENT '(' callargs ')'
            {$2->symbol = TOK_CALL; $$ = $2->adopt($1); 
               $2->adopt_children($3); destroy($3); destroy($4); }
            ;

callargs    : expr
            {$$ = (new astree(TOK_ROOT, {0,0,0}, ""))->adopt($1); }
            | callargs ',' expr
            {$$ = $1->adopt($3); destroy($2); }
            ;

variable    : TOK_IDENT
            {$$ = $1; }
            | expr '[' expr ']'
            {$2->symbol = TOK_INDEX; destroy ($4); 
               $$ = $2->adopt ($1, $3); }
            | expr TOK_ARROW TOK_IDENT
            {$$ = $2->adopt ($1, $3); }
            ;

constant    : TOK_INTCON
            {$$ = $1; }
            | TOK_CHARCON
            {$$ = $1; }
            | TOK_STRINGCON
            {$$ = $1; }
            | TOK_NULLPTR
            {$$ = $1; }
            ;
%%

const char *parser::get_tname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

