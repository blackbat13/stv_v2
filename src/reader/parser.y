%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <set>
#include <vector>

#include "expressions.hpp"
#include "nodes.hpp"

using namespace std;

/* funkcje i zmienne z flexa */
extern "C" int yylex();
void yyrestart( FILE* );
void yyerror( char* );
extern char* yytext;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char * str);

extern set<class AgentTemplate*>* modelDescription;
extern FormulaTemplate formulaDescription;
%}

%union {
       set<class AgentTemplate*>*  model;
       class ExprNode*             expr;
       class Assignment*           assign;
       class TransitionTemplate*   trans;
       class AgentTemplate*        agent;
       set<class Assignment*>*     assignSet;
       char*                       ident;
       set<string>*                identSet;
       int                         val;
       vector<class ExprNode*>*    condList;
}

%token   T_AGENT T_INIT T_LOCAL T_INITIAL T_FORMULA
%token   T_PERSISTENT T_TO T_ASSIGN T_OR T_AND T_EQ
%token   T_NE T_GT T_GE T_LT T_LE 
%token   <val> T_NUM T_SHARED
%token   <ident> T_IDENT
%token   T_KNOW T_HARTLEY

%type  <val>  shared
%type  <expr> num_exp num_mul num_elem
%type  <expr> condition cond cond_and cond_elem
%type  <trans> transition
%type  <identSet> local persistent ident_list coalition
%type  <assign> assignment
%type  <assignSet> initial assign_list assignments
%type  <agent> description agent
%type  <model> spec model
%type  <condList> cond_list

%%
model: spec { modelDescription = $1; }
     ;
     
spec: spec agent { $$=$1; $$->insert($2); }
    | spec query { $$=$1; }
    | { $$=new set<AgentTemplate*>; }
    ;

query: T_FORMULA formula { }
     ;

formula: coalition '[' ']' cond_list { formulaDescription.coalition=$1; formulaDescription.isF=false; formulaDescription.formula=$4; }
       | coalition T_LT T_GT cond_list { formulaDescription.coalition=$1; formulaDescription.isF=true; formulaDescription.formula=$4; }
       | '[' ']' cond_list { formulaDescription.coalition=new set<string>; formulaDescription.isF=false; formulaDescription.formula=$3; }
       | T_LT T_GT cond_list { formulaDescription.coalition=new set<string>; formulaDescription.isF=true; formulaDescription.formula=$3; }
       ;

cond_list: cond_list ',' cond { $$=$1; $$->push_back($3); }
         | cond { $$=new vector<ExprNode*>; $$->push_back($1); }
         ;

coalition: T_LT T_LT ident_list T_GT T_GT { $$=$3; }
         | T_LT T_LT T_GT T_GT { $$=new set<string>; }
         ;
      
agent: T_AGENT T_IDENT ':' description { $$=$4; $$->setIdent($2); delete $2; }
     ;

description: description local { $$=$1; $$->addLocal($2); delete $2; }
           | description persistent { $$=$1; $$->addPersistent($2); delete $2; }
           | description initial { $$=$1; $$->addInitial($2); delete $2; }
           | description T_INIT T_IDENT { $$=$1; $$->setInitState($3); delete $3; }
           | description transition { $$=$1; $$->addTransition($2); }
           | { $$=new AgentTemplate(); }
           ;

local: T_LOCAL '[' ident_list ']' { $$=$3; }
     | T_LOCAL '[' ']' { $$=NULL; }
     ;

persistent: T_PERSISTENT '[' ident_list ']' { $$=$3; }
          | T_PERSISTENT '[' ']'  { $$=NULL; }
          ;

ident_list: ident_list ',' T_IDENT { $$=$1; $$->insert($3); delete $3; }
          | T_IDENT { $$=new set<string>; $$->insert($1); delete $1; }
          ;
   
initial: T_INITIAL '['  assign_list ']' { $$=$3; }
       | T_INITIAL '[' ']' { $$=NULL; }
       ;

assign_list: assign_list ',' assignment { $$=$1; $$->insert($3); }
           | assignment { $$=new set<Assignment*>; $$->insert($1); }
           ;
assignment: T_IDENT T_ASSIGN num_exp { $$=new Assignment($1, $3); }
          ;
          
transition: shared T_IDENT '[' T_IDENT ']' ':' T_IDENT condition T_TO T_IDENT assignments {
              $$=new TransitionTemplate($1, $2, $4, $7, $10, $8, $11); 
              delete $2;
              delete $4;
              delete $7;
              delete $10;
            }
          | shared T_IDENT ':' T_IDENT condition T_TO T_IDENT assignments {
              $$=new TransitionTemplate($1, $2, $2, $4, $7, $5, $8);
              delete $2;
              delete $4;
              delete $7;
            }
          ;

shared: T_SHARED '[' T_NUM ']' { $$ = $3; } 
      | { $$ = -1; }
      ;

assignments: '[' assign_list ']' { $$=$2; }
           | { $$=NULL; }
           ;

num_exp: num_exp '-' num_mul { $$=new ExprSub($1, $3);}
       | num_exp '+' num_mul { $$=new ExprAdd($1, $3);}
       | num_mul { $$=$1; }
       ;
num_mul: num_mul '*' num_elem { $$=new ExprMul($1, $3);}
       | num_mul '/' num_elem { $$=new ExprDiv($1, $3);}
       | num_mul '%' num_elem { $$=new ExprRem($1, $3);}
       | num_elem { $$=$1; }
       ;
num_elem: T_IDENT { $$=new ExprIdent($1); delete $1; }
        | '-'T_NUM { $$=new ExprConst(-$2); }
        | T_NUM { $$=new ExprConst($1); }
        | '(' num_exp ')' {$$ = $2; }
        ;

condition: '[' cond ']' { $$=$2; }
         | { $$=NULL; }
         ;

cond: cond T_OR cond_and { $$=new ExprOr($1, $3);}
    | cond_and { $$=$1; }
    ;
cond_and: cond_and T_AND cond_elem { $$=new ExprAnd($1, $3);}
        | cond_elem { $$=$1; }
        ;
cond_elem: num_exp T_EQ num_exp { $$=new ExprEq($1, $3);}
         | num_exp T_NE num_exp { $$=new ExprNe($1, $3);}
         | num_exp T_GT num_exp { $$=new ExprGt($1, $3);}
         | num_exp T_GE num_exp { $$=new ExprGe($1, $3);}
         | num_exp T_LT num_exp { $$=new ExprLt($1, $3);}
         | num_exp T_LE num_exp { $$=new ExprLe($1, $3);}
         | '!' cond_elem { $$=new ExprNot($2);}
         | '(' cond ')' { $$=$2; }
         | T_IDENT { $$=new ExprIdent($1); delete $1; }
         | T_NUM { $$=new ExprConst($1); }
         | T_KNOW T_IDENT '(' cond ')' { $$=new ExprKnow($2, $4); }
         | T_HARTLEY T_IDENT '[' T_LE T_NUM ']' '(' cond_list ')' { $$=new ExprHart($2, true, $5, $8); }
         | T_HARTLEY T_IDENT '[' T_GE T_NUM ']' '(' cond_list ')' { $$=new ExprHart($2, false, $5, $8); }
         ;

%%

void set_input_string(const char* in) {
  yy_scan_string(in);
}

// Chwilowa funkcja błędu
void yyerror( char* s ) {
  fprintf(stderr,"niespodziewany token: '%s'\n",yytext);
  exit(1);
}

// Chwilowy main - do testów
int main2(int argc, char* argv[]) {
   
   if(argc > 1) {
      FILE *f=fopen(argv[1], "r");
      yyrestart(f);
   }
   
   yyparse();
   printf("OK\n");
   return 0;
}
