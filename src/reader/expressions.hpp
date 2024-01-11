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

/// @brief Variable names with their values.
typedef map<string, int> Environment;

class GlobalModelGenerator;
struct GlobalState;

// węzeł bazowy dla wyrażeń

/// @brief Base node for expressions.
class ExprNode {
   
   public:
      // metoda do wyliczenia wartości wyrażenia zależna od typu węzła
      
      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) = 0;
      virtual int eval( Environment& env ) = 0;
};

// węzeł dla stałej

/// @brief Node for a constant.
class ExprConst: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   int val;
   
   public:
      /// @brief Constant expression constructor.
      /// @param _val ExprConst value.
      ExprConst(int _val): val(_val) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla identyfikatora

/// @brief Node for an identifier.
class ExprIdent: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   string ident;
   
   public:
      /// @brief Identifier expression constructor.
      /// @param _ident ExprIdent value.
      ExprIdent(string _ident): ident(_ident) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla dodawań

/// @brief Node for addition.
class ExprAdd: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Addition expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprAdd(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla odejmowań

/// @brief Node for subtraction.
class ExprSub: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Subtraction expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprSub(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla mnożeń

/// @brief Node for multiplication.
class ExprMul: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Multiplication expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprMul(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla dzieleń

/// @brief Node for division.
class ExprDiv: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Division expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprDiv(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla reszty z dzielenia

/// @brief Node for modulo.
class ExprRem: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Modulo expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprRem(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla operatora AND

/// @brief Node for AND operator.
class ExprAnd: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Logic AND expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprAnd(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla operatora OR

/// @brief Node for OR operator.
class ExprOr: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Logic OR expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprOr(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla operatora NOT

/// @brief Node for NOT operator.
class ExprNot: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *arg;

   public:
      /// @brief Logic NOT expression constructor.
      /// @param _arg Calculates the expression value.
      ExprNot(ExprNode *_arg): arg(_arg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla operatora "=="

/// @brief Node for "==" operator.
class ExprEq: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Equals expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprEq(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla operatora "!="

/// @brief Node for "!=" operator.
class ExprNe: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Not equals expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprNe(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla operatora "<"

/// @brief Node for "<" operator.
class ExprLt: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Less than expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprLt(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla operatora "<="

/// @brief Node for "<=" operator.
class ExprLe: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Less or equal expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprLe(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla operatora ">"

/// @brief Node for ">" operator.
class ExprGt: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Greater than expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprGt(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla operatora ">="

/// @brief Node for ">=" operator.
class ExprGe: public ExprNode {
   
   // argumenty

   /// @brief Expression argument.
   ExprNode *larg, *rarg;

   public:
      /// @brief Greater or equal expression constructor.
      /// @param _larg Left argument of the expression.
      /// @param _rarg Right argument of the expression.
      ExprGe(ExprNode *_larg, ExprNode *_rarg): larg(_larg), rarg(_rarg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla wiedzy

/// @brief Node for a knowledge operator.
class ExprKnow: public ExprNode {
   
   // argumenty

   /// @brief Expression arguments.
   string agentName;
   ExprNode *arg;
   
   public:
      /// @brief Constant expression constructor.
      /// @param _agentName Agent name for knowledge operator.
      /// @param _arg Expression to verify with the given knowledge.
      ExprKnow(string _agentName, ExprNode *_arg): agentName(_agentName), arg(_arg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

// węzeł dla Hartleya

/// @brief Node for a Hartley operator.
class ExprHart: public ExprNode {
   
   // argumenty

   /// @brief Expression arguments.
   string agentName;
   bool le;
   int val;
   ExprNode *arg;
   
   public:
      /// @brief Constant expression constructor.
      /// @param _agentName Agent name for Hartley operator.
      /// @param _le Flag for if Hartley coefficient should be less equal or greater equal.
      /// @param _val Number to compare the result to.
      /// @param _arg Expression to verify with the given knowledge.
      ExprHart(string _agentName, bool _le, int _val, ExprNode *_arg): agentName(_agentName), le(_le), val(_val), arg(_arg) {};

      /// @brief Calculates the expression value.
      /// @param env Environment values.
      /// @return Returns an integer.
      virtual int eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState );
      virtual int eval( Environment& env );
};

#endif
