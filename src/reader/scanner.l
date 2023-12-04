%option noyywrap
%{
#include <string.h>
#include <string>
#include <set>

using namespace std;

#include "parser.h"
#include "expressions.hpp"
#include "nodes.hpp"

extern "C" int yylex();
%}

ident [a-zA-Z][_a-zA-Z0-9]*
single [\[\]():!,+\-*/%]
comment #([ \t]*[a-zA-Z][_a-zA-Z0-9]*)*
blank [ \n\t]
%%

"Agent"        { return T_AGENT; }
"shared"       { return T_SHARED; }
"init"         { return T_INIT;}
"LOCAL:"        { return T_LOCAL; }
"INITIAL:"      { return T_INITIAL; }
"PERSISTENT:"   { return T_PERSISTENT; }
"FORMULA:"      { return T_FORMULA; }
"->"           { return T_TO; }
":="           { return T_ASSIGN; }
"||"           { return T_OR; }
"&&"           { return T_AND; }
"=="           { return T_EQ; }
"!="           { return T_NE; }
">="           { return T_GE; }
">"            { return T_GT; }
"<="           { return T_LE; }
"<"            { return T_LT; }
[0-9]+       { yylval.val = atoi(yytext); return T_NUM; }
{ident}        { yylval.ident = strdup(yytext); return T_IDENT; }
{single}       { return yytext[0]; }
"&K_"           { return T_KNOW; }
{comment}      { /* puste - pomiń */ }
{blank}        { /* puste - pomiń */ }
%%

