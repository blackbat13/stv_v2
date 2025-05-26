%option noyywrap
%option prefix="strat"
%{
#include <string.h>
#include <string>
#include <set>
#include <vector>

using namespace std;

#include "strategyParser.h"
#include "strategyNodes.hpp"

extern "C" int stratlex();
%}

ident [a-zA-Z][_a-zA-Z0-9]*
single [\[\]():!,+\-*/%]
comment #[^\n]*
blank [ \n\t]
floatno [0-9]*\.[0-9]+
%%

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
{floatno}      { stratlval.floatno = atof(strattext); return T_FLOAT; }
[0-9]+         { stratlval.val = atoi(strattext); return T_NUM; }
{ident}        { stratlval.ident = strdup(strattext); return T_IDENT; }
{single}       { return strattext[0]; }
{comment}      { /* puste - pomiń */ }
{blank}        { /* puste - pomiń */ }
%%

