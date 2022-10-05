#include "expressions.hpp"

int ExprIdent::eval( Environment& env ) {
   auto res = env.find(ident);
   return res == env.end() ? -1 : (*res).second;
}

int ExprConst::eval( Environment& env ) {
   return val;
}

int ExprAdd::eval( Environment& env ) {   
   return larg->eval(env) + rarg->eval(env);
}

int ExprSub::eval( Environment& env ) {
   return larg->eval(env) - rarg->eval(env);
}

int ExprMul::eval( Environment& env ) {
   return larg->eval(env) * rarg->eval(env);
}

int ExprDiv::eval( Environment& env ) {
   return larg->eval(env) / rarg->eval(env);
}

int ExprRem::eval( Environment& env ) {
   return larg->eval(env) % rarg->eval(env);
}

int ExprAnd::eval( Environment& env ) {
   return larg->eval(env) && rarg->eval(env);
}

int ExprOr::eval( Environment& env ) {
   return larg->eval(env) || rarg->eval(env);
}

int ExprNot::eval( Environment& env ) {
   return !arg->eval(env);
}

int ExprEq::eval( Environment& env ) {
   return larg->eval(env) == rarg->eval(env);
}

int ExprNe::eval( Environment& env ) {
   return larg->eval(env) != rarg->eval(env);
}

int ExprLt::eval( Environment& env ) {
   return larg->eval(env) < rarg->eval(env);
}

int ExprLe::eval( Environment& env ) {
   return larg->eval(env) <= rarg->eval(env);
}

int ExprGt::eval( Environment& env ) {
   return larg->eval(env) > rarg->eval(env);
}

int ExprGe::eval( Environment& env ) {
   return larg->eval(env) >= rarg->eval(env);
}

