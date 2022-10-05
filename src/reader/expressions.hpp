#ifndef __EXPRESSIONS_H
#define __EXPRESSIONS_H

#include <string>
#include <map>

using namespace std;

typedef map<string, int> Environment;

// węzeł bazowy dla wyrażeń
class ExprNode {
   
   public:
      // metoda do wyliczenia wartości wyrażenia zależna od typu węzła
      virtual int eval( Environment& env ) = 0;
};

// węzeł dla stałej
class ExprConst: public ExprNode {
   
   // argumenty
   int val;
   
   public:
      ExprConst(int _val): val(_val) {};
      virtual int eval( Environment& env );
};

// węzeł dla identyfikatora
class ExprIdent: public ExprNode {
   
   // argumenty
   string ident;
   
   public:
      ExprIdent(string _ident): ident(_ident) {};
      virtual int eval( Environment& env );
};

// węzeł dla dodawań
class ExprAdd: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprAdd(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla odejmowań
class ExprSub: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprSub(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla mnożeń
class ExprMul: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprMul(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla dzieleń
class ExprDiv: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprDiv(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla reszty z dzielenia
class ExprRem: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprRem(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora AND
class ExprAnd: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprAnd(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora OR
class ExprOr: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprOr(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora NOT
class ExprNot: public ExprNode {
   
   // argumenty
   ExprNode *arg;

   public:
      ExprNot(ExprNode *_arg): arg(_arg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora "=="
class ExprEq: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprEq(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora "!="
class ExprNe: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprNe(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora "<"
class ExprLt: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprLt(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora "<="
class ExprLe: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprLe(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora ">"
class ExprGt: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprGt(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora ">="
class ExprGe: public ExprNode {
   
   // argumenty
   ExprNode *larg, *rarg;

   public:
      ExprGe(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};


#endif
