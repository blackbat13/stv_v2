/**
 * @file expressions.cc
 * @brief Eval and helper class for expressions.
 * Eval and helper class for expressions.
 */

#include "expressions.hpp"
#include "../GlobalModelGenerator.hpp"
#include <bits/stdc++.h>

set<string> ExprNode::getVariableNames() const {
   set<string> result;
   collectVariableNames(result);
   return result;
}

void ExprConst::collectVariableNames(set<string>& out) const {
   (void) out;
}

void ExprIdent::collectVariableNames(set<string>& out) const {
   out.insert(ident);
}

void ExprAdd::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprSub::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprMul::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprDiv::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprRem::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprAnd::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprOr::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprNot::collectVariableNames(set<string>& out) const {
   arg->collectVariableNames(out);
}

void ExprEq::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprNe::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprLt::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprLe::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprGt::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprGe::collectVariableNames(set<string>& out) const {
   larg->collectVariableNames(out);
   rarg->collectVariableNames(out);
}

void ExprKnow::collectVariableNames(set<string>& out) const {
   arg->collectVariableNames(out);
}

void ExprHart::collectVariableNames(set<string>& out) const {
   for (auto* expr : *arg) {
      expr->collectVariableNames(out);
   }
}

/// @brief 
/// @param env 
/// @return 
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

int ExprKnow::eval( Environment& env ) {
   return 1;
}

int ExprHart::eval( Environment& env ) {
   return 1;
}

int ExprIdent::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState) {
   auto res = env.find(ident);
   if (res == env.end()) {
      generator->markFormulaAsIncorrect();
      return -1;
   }
   return (*res).second;
}

int ExprConst::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return val;
}

int ExprAdd::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState) {   
   return larg->eval(env, generator, globalState) + rarg->eval(env, generator, globalState);
}

int ExprSub::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) - rarg->eval(env, generator, globalState);
}

int ExprMul::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) * rarg->eval(env, generator, globalState);
}

int ExprDiv::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) / rarg->eval(env, generator, globalState);
}

int ExprRem::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) % rarg->eval(env, generator, globalState);
}

int ExprAnd::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) && rarg->eval(env, generator, globalState);
}

int ExprOr::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) || rarg->eval(env, generator, globalState);
}

int ExprNot::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return !arg->eval(env, generator, globalState);
}

int ExprEq::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) == rarg->eval(env, generator, globalState);
}

int ExprNe::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) != rarg->eval(env, generator, globalState);
}

int ExprLt::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) < rarg->eval(env, generator, globalState);
}

int ExprLe::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) <= rarg->eval(env, generator, globalState);
}

int ExprGt::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) > rarg->eval(env, generator, globalState);
}

int ExprGe::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(env, generator, globalState) >= rarg->eval(env, generator, globalState);
}

int ExprKnow::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   bool allEpistemicOk = true;
   Agent* knowledgeAgentInstance = generator->getAgentInstanceByName(agentName);
   auto epistemicClassesForAgent = generator->findOrCreateEpistemicClassForKnowledge(&globalState->localStatesProjection, globalState, knowledgeAgentInstance);
   for (auto global : *epistemicClassesForAgent) {
      map<string, int> currEnv = global->getGlobalStateEnvironment();
      if (!arg->eval(currEnv, generator, globalState)) {
         allEpistemicOk = false;
         break;
      }
   }
   return (allEpistemicOk ? 1 : 0);
}

int ExprHart::eval( Environment& env, GlobalModelGenerator *generator, GlobalState *globalState ) {
   bool allEpistemicOk = true;
   Agent* knowledgeAgentInstance = generator->getAgentInstanceByName(agentName);
   auto epistemicClassesForAgent = generator->findOrCreateEpistemicClassForKnowledge(&globalState->localStatesProjection, globalState, knowledgeAgentInstance);
   
   set<int64_t>* foundClasses = new set<int64_t>();
   
   for (auto global : *epistemicClassesForAgent) {
      map<string, int> currEnv = global->getGlobalStateEnvironment();

      int64_t values = 0;
      int64_t add = 1;
      auto val = (*arg).size();
      vector<ExprNode*> exprNodes = *arg;
      for (int i = 0; i < val; i++) {
         if (exprNodes[i]->eval(currEnv, generator, globalState)) {
               values += add;
         }
         add *= 2;
      }
      foundClasses->insert(values);
   }

   if (le) {
      allEpistemicOk = (log2f((float)foundClasses->size()) <= (float)val);
   }
   else {
      allEpistemicOk = (log2f((float)foundClasses->size()) >= (float)val);
   }

   free(foundClasses);

   return (allEpistemicOk ? 1 : 0);
}

float ProbConst::eval() {
   return val;
}

float ProbAdd::eval() {   
   return larg->eval() + rarg->eval();
}

float ProbSub::eval() {
   return larg->eval() - rarg->eval();
}

float ProbMul::eval() {
   return larg->eval() * rarg->eval();
}

float ProbDiv::eval() {
   return larg->eval() / rarg->eval();
}

float ProbConst::eval( GlobalModelGenerator *generator, GlobalState *globalState ) {
   return val;
}

float ProbAdd::eval( GlobalModelGenerator *generator, GlobalState *globalState) {   
   return larg->eval(generator, globalState) + rarg->eval(generator, globalState);
}

float ProbSub::eval( GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(generator, globalState) - rarg->eval(generator, globalState);
}

float ProbMul::eval( GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(generator, globalState) * rarg->eval(generator, globalState);
}

float ProbDiv::eval( GlobalModelGenerator *generator, GlobalState *globalState ) {
   return larg->eval(generator, globalState) / rarg->eval(generator, globalState);
}

// ---- ExprNode::toString() implementations ----

string ExprConst::toString(bool addBrackets) const { return to_string(val); }
string ExprIdent::toString(bool addBrackets) const { return ident; }
string ExprAdd::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " + " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprSub::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " - " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprMul::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " * " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprDiv::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " / " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprRem::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " % " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprAnd::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " && " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprOr::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " || " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprNot::toString(bool addBrackets) const { return "!" + arg->toString(true); }
string ExprEq::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " == " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprNe::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " != " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprLt::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " < " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprLe::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " <= " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprGt::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " > " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}
string ExprGe::toString(bool addBrackets) const { 
   string result = larg->toString(true) + " >= " + rarg->toString(true);
   return addBrackets ? "(" + result + ")" : result;
}

string ExprKnow::toString(bool addBrackets) const {
   return "&K_" + agentName + "(" + arg->toString(true) + ")";
}

string ExprHart::toString(bool addBrackets) const {
   string result = "&H_" + agentName + "[" + (le ? "<=" : ">=") + to_string(val) + "](";
   for (size_t i = 0; i < arg->size(); ++i) {
      if (i > 0) result += ", ";
      result += "(" + (*arg)[i]->toString(true) + ")";
   }
   result += ")";
   return result;
}