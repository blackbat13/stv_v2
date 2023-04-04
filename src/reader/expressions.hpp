/**
 * @file expressions.hpp
 * @brief Eval and helper class for expressions.
 * Eval and helper class for expressions.
 */

#ifndef __EXPRESSIONS_H
#define __EXPRESSIONS_H

#include <string>
#include <map>

using namespace std;

/// @brief ???
typedef map<string, int> Environment;

// węzeł bazowy dla wyrażeń

/// @brief Base node for expressions.
class ExprNode {
   
   public:
      // metoda do wyliczenia wartości wyrażenia zależna od typu węzła
      virtual int eval( Environment& env ) = 0;
};

// węzeł dla stałej

/// @brief Node for a constant.
class ExprConst: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   int val;
   
   public:
      ExprConst(int _val): val(_val) {};
      virtual int eval( Environment& env );
};

// węzeł dla identyfikatora

/// @brief Node for an identifier.
class ExprIdent: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   string ident;
   
   public:
      ExprIdent(string _ident): ident(_ident) {};
      virtual int eval( Environment& env );
};

// węzeł dla dodawań

/// @brief Node for addition.
class ExprAdd: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprAdd(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla odejmowań

/// @brief Node for subtraction.
class ExprSub: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprSub(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla mnożeń

/// @brief Node for multiplication.
class ExprMul: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprMul(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla dzieleń

/// @brief Node for division.
class ExprDiv: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprDiv(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla reszty z dzielenia

/// @brief Node for modulo.
class ExprRem: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprRem(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora AND

/// @brief Node for AND operator.
class ExprAnd: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprAnd(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora OR

/// @brief Node for OR operator.
class ExprOr: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprOr(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora NOT

/// @brief Node for NOT operator.
class ExprNot: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *arg;

   public:
      ExprNot(ExprNode *_arg): arg(_arg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora "=="

/// @brief Node for "==" operator.
class ExprEq: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprEq(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora "!="

/// @brief Node for "!=" operator.
class ExprNe: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprNe(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora "<"

/// @brief Node for "<" operator.
class ExprLt: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprLt(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora "<="

/// @brief Node for "<=" operator.
class ExprLe: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprLe(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora ">"

/// @brief Node for ">" operator.
class ExprGt: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprGt(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};

// węzeł dla operatora ">="

/// @brief Node for ">=" operator.
class ExprGe: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      ExprGe(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};
      virtual int eval( Environment& env );
};


#endif
