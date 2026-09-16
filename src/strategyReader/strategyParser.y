%define api.prefix {strat}
%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <set>
#include <vector>

#include "strategyNodes.hpp"

using namespace std;

/* funkcje i zmienne z flexa */
extern "C" int stratlex();
void stratrestart( FILE* );
void straterror( const char* );
extern char* strattext;
typedef struct strat_buffer_state * YY_BUFFER_STATE;
extern YY_BUFFER_STATE strat_scan_string(const char * str);

extern StrategyCollectionTemplate strategyCollectionTemplate;

%}

%union {
       char*                       expr;
       int                         val;
       float                       floatno;
       char*                       ident;
       vector<string>*             identList;
}

%token   T_TO T_ASSIGN T_OR T_AND T_EQ
%token   T_NE T_GT T_GE T_LT T_LE 
%token   <val> T_NUM T_SHARED
%token   <floatno> T_FLOAT
%token   <ident> T_IDENT

%type  <identList> ident_list
 
%%
strat: strat ident_list T_TO T_IDENT { strategyCollectionTemplate.addAction($2, $4); }
     | { }
     ;

ident_list: ident_list ',' T_IDENT { $$=$1; $$->push_back($3); delete $3; }
          | T_IDENT { $$=new vector<string>; $$->push_back($1); delete $1; }
          ;

%%

void set_input_string_strat(const char* in) {
  strat_scan_string(in);
}

// Chwilowa funkcja błędu
void straterror( const char* s ) {
  fprintf(stderr,"niespodziewany token: '%s'\n", strattext);
  exit(1);
}
