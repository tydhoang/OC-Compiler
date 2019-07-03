/*
* CMPS104A
* Created by:
*  Tyler Hoang (tydhoang@ucsc.edu)
*  Eric Vin (evin@ucsc.edu)
* scanner.l
*/
%{
    
#include "auxlib.h"
#include "lyutils.h"

#define YY_USER_ACTION  { lexer::advance (); }

%}

%option 8bit
/*%option debug*/
%option nobackup
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option warn
/*%option verbose*/

TOK_INT         (int)
TOK_STRING      (string)
TOK_VOID        (void)
TOK_IF          (if)
TOK_ELSE        (else)
TOK_NULLPTR     (nullptr)
TOK_PTR         (ptr)
TOK_WHILE       (while)
TOK_RETURN      (return)
TOK_STRUCT      (struct)
TOK_NOT         (not)
TOK_ALLOC       (alloc)
TOK_ARRAY       (array)

TOK_EQ          (==)
TOK_NE          (!=)
TOK_LT          (<)
TOK_GT          (>)
TOK_LE          (<=)
TOK_GE          (>=)
TOK_ARROW       (->)

LETTER          [A-Za-z_]
DIGIT           [0-9]
MANTISSA        ({DIGIT}+\.?{DIGIT}*|\.{DIGIT}+)
EXPONENT        ([Ee][+-]?{DIGIT}+)
TOK_INTCON      ({MANTISSA}{EXPONENT}?)
NOTNUMBER       ({MANTISSA}[Ee][+-]?)
TOK_IDENT       ({LETTER}({LETTER}|{DIGIT})*)
TOK_CHARCON     ('([^\\'\n]|\\[\\'"0nt])')
TOK_STRINGCON   (\"([^\\"\n]|\\[\\'"0nt])*\")

%%

"#".*           { lexer::include(); }
[ \t]+          { }
\n              { lexer::newline(); }

{TOK_INTCON}    { return lexer::token (TOK_INTCON); }

{TOK_ARRAY}     { return lexer::token (TOK_ARRAY); }
{TOK_ARROW}     { return lexer::token (TOK_ARROW); }
{TOK_ALLOC}     { return lexer::token (TOK_ALLOC); }
{TOK_CHARCON}   { return lexer::token (TOK_CHARCON); }
{TOK_ELSE}      { return lexer::token (TOK_ELSE); }
{TOK_EQ}        { return lexer::token (TOK_EQ); }
{TOK_GE}        { return lexer::token (TOK_GE); }
{TOK_GT}        { return lexer::token (TOK_GT); }
{TOK_IF}        { return lexer::token (TOK_IF); }
{TOK_INT}       { return lexer::token (TOK_INT); }
{TOK_LE}        { return lexer::token (TOK_LE); }
{TOK_LT}        { return lexer::token (TOK_LT); }
{TOK_NE}        { return lexer::token (TOK_NE); }
{TOK_NULLPTR}   { return lexer::token (TOK_NULLPTR); }
{TOK_PTR}       { return lexer::token (TOK_PTR); }
{TOK_RETURN}    { return lexer::token (TOK_RETURN); }
{TOK_STRING}    { return lexer::token (TOK_STRING); }
{TOK_STRINGCON} { return lexer::token (TOK_STRINGCON); }
{TOK_STRUCT}    { return lexer::token (TOK_STRUCT); }
{TOK_NOT}       { return lexer::token (TOK_NOT); }
{TOK_VOID}      { return lexer::token (TOK_VOID); }
{TOK_WHILE}     { return lexer::token (TOK_WHILE); }

"="             { return lexer::token ('='); }
"+"             { return lexer::token ('+'); }
"-"             { return lexer::token ('-'); }
"*"             { return lexer::token ('*'); }
"/"             { return lexer::token ('/'); }
"^"             { return lexer::token ('^'); }
"("             { return lexer::token ('('); }
")"             { return lexer::token (')'); }
";"             { return lexer::token (';'); }
"["             { return lexer::token ('['); }
"]"             { return lexer::token (']'); }
"{"             { return lexer::token ('{'); }
"}"             { return lexer::token ('}'); }
"%"             { return lexer::token ('%'); }
","             { return lexer::token (','); }
"."             { return lexer::token ('.'); }
"!"             { return lexer::token ('!'); }
{TOK_IDENT}     { return lexer::token (TOK_IDENT); }


{NOTNUMBER}     { return lexer::badtoken (TOK_INTCON); }
.               { lexer::badchar (*yytext); }

%%

