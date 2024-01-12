/**
 * @file expressions.cc
 * @brief Eval and helper class for expressions.
 * Eval and helper class for expressions.
 */

#include "expressions.hpp"
#include "../GlobalModelGenerator.hpp"
#include <bits/stdc++.h>

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
   return res == env.end() ? -1 : (*res).second;
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
