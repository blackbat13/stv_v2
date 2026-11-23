%option noyywrap
%option yylineno
%{
#include <string.h>
#include <string>
#include <set>
#include <vector>
#include <stdexcept>

using namespace std;

#include "parser.h"
#include "expressions.hpp"
#include "nodes.hpp"

extern "C" int yylex();

static string describeCharacter(unsigned char ch) {
    if (ch >= 0x20 && ch <= 0x7e) {
        return string(1, static_cast<char>(ch));
    }
    const char* hex = "0123456789ABCDEF";
    return string("\\x") + hex[ch >> 4] + hex[ch & 0x0f];
}
%}

ident [a-zA-Z][_a-zA-Z0-9]*
single [\[\]():!,+\-*/%]
comment #[^\n]*
blank [ \r\n\t]
floatno [0-9]*\.[0-9]+
%%

"Agent"        { return T_AGENT; }
"shared"       { return T_SHARED; }
"empty"        { return T_EMPTY; }
"init"         { return T_INIT;}
"LOCAL:"       { return T_LOCAL; }
"INITIAL:"     { return T_INITIAL; }
"PERSISTENT:"  { return T_PERSISTENT; }
"FORMULA:"     { return T_FORMULA; }
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
{floatno}      { yylval.floatno = atof(yytext); return T_FLOAT; }
[0-9]+         { yylval.val = atoi(yytext); return T_NUM; }
{ident}        { yylval.ident = strdup(yytext); return T_IDENT; }
{single}       { return yytext[0]; }
"&K_"          { return T_KNOW; }
"&H_"          { return T_HARTLEY; }
"&P"           { return T_PROB; }
"&REST"        { return T_REST; }
{comment}      { /* puste - pomiń */ }
{blank}        { /* puste - pomiń */ }
.              { throw std::runtime_error("Unexpected character '" + describeCharacter(static_cast<unsigned char>(yytext[0])) + "' at line " + to_string(yylineno)); }
%%

