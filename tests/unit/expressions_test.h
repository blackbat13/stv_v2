/**
 * @file expressions_test.h
 * @brief Unit tests for the expression trees in reader/expressions.*.
 */

#include "helpers.h"

namespace {
/// @brief Shorthands that keep expression trees readable in the tests below.
ExprNode* num(int value) { return new ExprConst(value); }
ExprNode* var(const std::string& name) { return new ExprIdent(name); }
}

// ---- eval(env) --------------------------------------------------------------

TEST(ExprEval, ConstantIgnoresEnvironment) {
    Environment env{{"x", 7}};
    EXPECT_EQ(ExprConst(42).eval(env), 42);
    EXPECT_EQ(ExprConst(-3).eval(env), -3);
}

TEST(ExprEval, IdentReadsEnvironmentOrReturnsMinusOne) {
    Environment env{{"x", 7}};
    EXPECT_EQ(ExprIdent("x").eval(env), 7);
    EXPECT_EQ(ExprIdent("missing").eval(env), -1);
}

TEST(ExprEval, Arithmetic) {
    Environment env{{"a", 17}, {"b", 5}};
    EXPECT_EQ(ExprAdd(var("a"), var("b")).eval(env), 22);
    EXPECT_EQ(ExprSub(var("a"), var("b")).eval(env), 12);
    EXPECT_EQ(ExprMul(var("a"), var("b")).eval(env), 85);
    EXPECT_EQ(ExprDiv(var("a"), var("b")).eval(env), 3);   // integer division
    EXPECT_EQ(ExprRem(var("a"), var("b")).eval(env), 2);
}

TEST(ExprEval, Comparisons) {
    Environment env{{"a", 2}, {"b", 3}};
    EXPECT_EQ(ExprEq(var("a"), num(2)).eval(env), 1);
    EXPECT_EQ(ExprEq(var("a"), var("b")).eval(env), 0);
    EXPECT_EQ(ExprNe(var("a"), var("b")).eval(env), 1);
    EXPECT_EQ(ExprNe(var("a"), num(2)).eval(env), 0);
    EXPECT_EQ(ExprLt(var("a"), var("b")).eval(env), 1);
    EXPECT_EQ(ExprLt(var("b"), var("a")).eval(env), 0);
    EXPECT_EQ(ExprLt(var("a"), num(2)).eval(env), 0);       // strict
    EXPECT_EQ(ExprLe(var("a"), num(2)).eval(env), 1);
    EXPECT_EQ(ExprLe(var("b"), num(2)).eval(env), 0);
    EXPECT_EQ(ExprGt(var("b"), var("a")).eval(env), 1);
    EXPECT_EQ(ExprGt(var("a"), num(2)).eval(env), 0);       // strict
    EXPECT_EQ(ExprGe(var("a"), num(2)).eval(env), 1);
    EXPECT_EQ(ExprGe(var("a"), var("b")).eval(env), 0);
}

TEST(ExprEval, LogicTreatsNonZeroAsTrue) {
    Environment env{{"t", 5}, {"f", 0}};
    EXPECT_EQ(ExprAnd(var("t"), var("t")).eval(env), 1);
    EXPECT_EQ(ExprAnd(var("t"), var("f")).eval(env), 0);
    EXPECT_EQ(ExprOr(var("f"), var("t")).eval(env), 1);
    EXPECT_EQ(ExprOr(var("f"), var("f")).eval(env), 0);
    EXPECT_EQ(ExprNot(var("f")).eval(env), 1);
    EXPECT_EQ(ExprNot(var("t")).eval(env), 0);
}

TEST(ExprEval, NestedExpression) {
    // (a + 1) * b > 10 && !(a == 0)  with a=2, b=4  ->  12 > 10 && true
    Environment env{{"a", 2}, {"b", 4}};
    ExprAnd expression(new ExprGt(new ExprMul(new ExprAdd(var("a"), num(1)), var("b")), num(10)),
                       new ExprNot(new ExprEq(var("a"), num(0))));
    EXPECT_EQ(expression.eval(env), 1);
    env["b"] = 3;  // 9 > 10 is false
    EXPECT_EQ(expression.eval(env), 0);
}

TEST(ExprEval, EpistemicOperatorsWithoutModelAreTrue) {
    // Without a generator the knowledge operators cannot be evaluated and default to true.
    Environment env;
    EXPECT_EQ(ExprKnow("A", num(0)).eval(env), 1);
    EXPECT_EQ(ExprHart("A", true, 0, new vector<ExprNode*>{num(0)}).eval(env), 1);
}

// ---- toString ---------------------------------------------------------------

TEST(ExprToString, LeavesHaveNoBrackets) {
    EXPECT_EQ(ExprConst(5).toString(), "5");
    EXPECT_EQ(ExprIdent("x").toString(), "x");
}

TEST(ExprToString, BinaryOperatorsAndBrackets) {
    EXPECT_EQ(ExprAdd(var("a"), num(1)).toString(true), "(a + 1)");
    EXPECT_EQ(ExprAdd(var("a"), num(1)).toString(false), "a + 1");
    EXPECT_EQ(ExprSub(var("a"), num(1)).toString(false), "a - 1");
    EXPECT_EQ(ExprMul(var("a"), num(1)).toString(false), "a * 1");
    EXPECT_EQ(ExprDiv(var("a"), num(1)).toString(false), "a / 1");
    EXPECT_EQ(ExprRem(var("a"), num(1)).toString(false), "a % 1");
    EXPECT_EQ(ExprAnd(var("a"), var("b")).toString(false), "a && b");
    EXPECT_EQ(ExprOr(var("a"), var("b")).toString(false), "a || b");
    EXPECT_EQ(ExprEq(var("a"), num(1)).toString(false), "a == 1");
    EXPECT_EQ(ExprNe(var("a"), num(1)).toString(false), "a != 1");
    EXPECT_EQ(ExprLt(var("a"), num(1)).toString(false), "a < 1");
    EXPECT_EQ(ExprLe(var("a"), num(1)).toString(false), "a <= 1");
    EXPECT_EQ(ExprGt(var("a"), num(1)).toString(false), "a > 1");
    EXPECT_EQ(ExprGe(var("a"), num(1)).toString(false), "a >= 1");
}

TEST(ExprToString, NestedOperandsAreAlwaysBracketed) {
    ExprMul expression(new ExprAdd(var("a"), num(1)), var("b"));
    EXPECT_EQ(expression.toString(false), "(a + 1) * b");
    EXPECT_EQ(expression.toString(true), "((a + 1) * b)");
}

TEST(ExprToString, UnaryAndEpistemicOperators) {
    EXPECT_EQ(ExprNot(var("x")).toString(), "!x");
    EXPECT_EQ(ExprNot(new ExprEq(var("x"), num(1))).toString(), "!(x == 1)");
    EXPECT_EQ(ExprKnow("Voter", new ExprEq(var("v"), num(1))).toString(), "&K_Voter((v == 1))");
    ExprHart hartley("Coercer", true, 2, new vector<ExprNode*>{new ExprEq(var("v"), num(1)), var("w")});
    EXPECT_EQ(hartley.toString(), "&H_Coercer[<=2](((v == 1)), (w))");
    ExprHart hartleyGe("Coercer", false, 1, new vector<ExprNode*>{var("w")});
    EXPECT_EQ(hartleyGe.toString(), "&H_Coercer[>=1]((w))");
}

// ---- getVariableNames -------------------------------------------------------

TEST(ExprVariableNames, ConstantHasNone) {
    EXPECT_TRUE(ExprConst(1).getVariableNames().empty());
}

TEST(ExprVariableNames, CollectsEachVariableOnceFromNestedTree) {
    // (a + b) * a == c || !(d < 3)
    ExprOr expression(new ExprEq(new ExprMul(new ExprAdd(var("a"), var("b")), var("a")), var("c")),
                      new ExprNot(new ExprLt(var("d"), num(3))));
    EXPECT_EQ(expression.getVariableNames(), (set<string>{"a", "b", "c", "d"}));
}

TEST(ExprVariableNames, EveryOperatorForwardsToItsOperands) {
    EXPECT_EQ(ExprSub(var("a"), var("b")).getVariableNames(), (set<string>{"a", "b"}));
    EXPECT_EQ(ExprDiv(var("a"), var("b")).getVariableNames(), (set<string>{"a", "b"}));
    EXPECT_EQ(ExprRem(var("a"), var("b")).getVariableNames(), (set<string>{"a", "b"}));
    EXPECT_EQ(ExprAnd(var("a"), var("b")).getVariableNames(), (set<string>{"a", "b"}));
    EXPECT_EQ(ExprNe(var("a"), var("b")).getVariableNames(), (set<string>{"a", "b"}));
    EXPECT_EQ(ExprLe(var("a"), var("b")).getVariableNames(), (set<string>{"a", "b"}));
    EXPECT_EQ(ExprGt(var("a"), var("b")).getVariableNames(), (set<string>{"a", "b"}));
    EXPECT_EQ(ExprGe(var("a"), var("b")).getVariableNames(), (set<string>{"a", "b"}));
    EXPECT_EQ(ExprKnow("A", var("k")).getVariableNames(), (set<string>{"k"}));
    EXPECT_EQ(ExprHart("A", true, 1, new vector<ExprNode*>{var("h1"), var("h2")}).getVariableNames(),
              (set<string>{"h1", "h2"}));
}

// ---- probability expressions -------------------------------------------------

TEST(ProbEval, Arithmetic) {
    EXPECT_FLOAT_EQ(ProbConst(0.25f).eval(), 0.25f);
    EXPECT_FLOAT_EQ(ProbAdd(new ProbConst(0.25f), new ProbConst(0.5f)).eval(), 0.75f);
    EXPECT_FLOAT_EQ(ProbSub(new ProbConst(1.0f), new ProbConst(0.25f)).eval(), 0.75f);
    EXPECT_FLOAT_EQ(ProbMul(new ProbConst(0.5f), new ProbConst(0.5f)).eval(), 0.25f);
    EXPECT_FLOAT_EQ(ProbDiv(new ProbConst(1.0f), new ProbConst(4.0f)).eval(), 0.25f);
}

TEST(ProbEval, GeneratorOverloadGivesSameResult) {
    ProbDiv expression(new ProbAdd(new ProbConst(1.0f), new ProbConst(2.0f)), new ProbMul(new ProbConst(2.0f), new ProbSub(new ProbConst(3.0f), new ProbConst(1.0f))));
    EXPECT_FLOAT_EQ(expression.eval(), 0.75f);
    EXPECT_FLOAT_EQ(expression.eval(nullptr, nullptr), 0.75f);
}

// ---- eval(env, generator, state) ---------------------------------------------

/// Two agents: Observer never moves, Mover flips x between 0 and 1. The
/// Observer cannot tell the two global states apart.
static const char* KNOWLEDGE_MODEL = R"(
Agent Observer:
LOCAL: [o]
PERSISTENT: [o]
INITIAL: [o:=0]
init idle
wait: idle -> idle

Agent Mover:
LOCAL: [x]
PERSISTENT: [x]
INITIAL: [x:=0]
init s0
flip: s0 -> s1 [x:=1]
back: s1 -> s0 [x:=0]

FORMULA: <<Observer>>[](o==0)
)";

class ExprGeneratorEval : public UnitTest {};

TEST_F(ExprGeneratorEval, IdentOnUnknownVariableMarksFormulaIncorrect) {
    GeneratedModel generated(KNOWLEDGE_MODEL);
    GlobalState* init = generated.model()->initState;
    Environment env = init->getGlobalStateEnvironment();

    EXPECT_EQ(ExprIdent("x").eval(env, &generated.generator, init), 0);
    EXPECT_TRUE(generated.generator.getFormulaCorectness());

    EXPECT_EQ(ExprIdent("nope").eval(env, &generated.generator, init), -1);
    EXPECT_FALSE(generated.generator.getFormulaCorectness());
}

TEST_F(ExprGeneratorEval, ArithmeticOverloadMatchesPlainEval) {
    GeneratedModel generated(KNOWLEDGE_MODEL);
    GlobalState* init = generated.model()->initState;
    Environment env{{"a", 9}, {"b", 4}};
    auto* g = &generated.generator;
    EXPECT_EQ(ExprAdd(var("a"), var("b")).eval(env, g, init), 13);
    EXPECT_EQ(ExprSub(var("a"), var("b")).eval(env, g, init), 5);
    EXPECT_EQ(ExprMul(var("a"), var("b")).eval(env, g, init), 36);
    EXPECT_EQ(ExprDiv(var("a"), var("b")).eval(env, g, init), 2);
    EXPECT_EQ(ExprRem(var("a"), var("b")).eval(env, g, init), 1);
    EXPECT_EQ(ExprAnd(var("a"), num(0)).eval(env, g, init), 0);
    EXPECT_EQ(ExprOr(var("a"), num(0)).eval(env, g, init), 1);
    EXPECT_EQ(ExprNot(var("a")).eval(env, g, init), 0);
    EXPECT_EQ(ExprEq(var("a"), num(9)).eval(env, g, init), 1);
    EXPECT_EQ(ExprNe(var("a"), num(9)).eval(env, g, init), 0);
    EXPECT_EQ(ExprLt(var("b"), var("a")).eval(env, g, init), 1);
    EXPECT_EQ(ExprLe(var("a"), var("a")).eval(env, g, init), 1);
    EXPECT_EQ(ExprGt(var("b"), var("a")).eval(env, g, init), 0);
    EXPECT_EQ(ExprGe(var("b"), var("a")).eval(env, g, init), 0);
    EXPECT_EQ(ExprConst(3).eval(env, g, init), 3);
}

TEST_F(ExprGeneratorEval, KnowledgeHoldsOnlyIfTrueInAllIndistinguishableStates) {
    GeneratedModel generated(KNOWLEDGE_MODEL);
    generated.generator.expandAllStates();
    ASSERT_EQ(generated.model()->globalStates.size(), 2u);
    GlobalState* init = generated.model()->initState;
    Environment env = init->getGlobalStateEnvironment();

    // Observer sees "idle" in both states: it knows o==0 but not x==0.
    EXPECT_EQ(ExprKnow("Observer", new ExprEq(var("o"), num(0))).eval(env, &generated.generator, init), 1);
    EXPECT_EQ(ExprKnow("Observer", new ExprEq(var("x"), num(0))).eval(env, &generated.generator, init), 0);
    // Mover sees its own state, so it knows the value of x.
    EXPECT_EQ(ExprKnow("Mover", new ExprEq(var("x"), num(0))).eval(env, &generated.generator, init), 1);
}

TEST_F(ExprGeneratorEval, HartleyCountsDistinguishableOutcomes) {
    GeneratedModel generated(KNOWLEDGE_MODEL);
    generated.generator.expandAllStates();
    GlobalState* init = generated.model()->initState;
    Environment env = init->getGlobalStateEnvironment();

    // For the Observer, x==1 takes 2 values across its class -> log2(2) = 1.
    auto xIsOne = [] { return new vector<ExprNode*>{new ExprEq(new ExprIdent("x"), new ExprConst(1))}; };
    EXPECT_EQ(ExprHart("Observer", true, 1, xIsOne()).eval(env, &generated.generator, init), 1);   // 1 <= 1
    EXPECT_EQ(ExprHart("Observer", true, 0, xIsOne()).eval(env, &generated.generator, init), 0);   // 1 <= 0 fails
    EXPECT_EQ(ExprHart("Observer", false, 1, xIsOne()).eval(env, &generated.generator, init), 1);  // 1 >= 1
    // The Mover's class has a single state -> log2(1) = 0.
    EXPECT_EQ(ExprHart("Mover", true, 0, xIsOne()).eval(env, &generated.generator, init), 1);
    EXPECT_EQ(ExprHart("Mover", false, 1, xIsOne()).eval(env, &generated.generator, init), 0);
}

// Detects the new/free mismatch only when built with -fsanitize=address.
TEST_F(ExprGeneratorEval, HartleyReleasesMemoryWithMatchingDeallocator) {
    GeneratedModel generated(KNOWLEDGE_MODEL);
    generated.generator.expandAllStates();
    GlobalState* init = generated.model()->initState;
    Environment env = init->getGlobalStateEnvironment();
    ExprHart hartley("Observer", true, 1, new vector<ExprNode*>{new ExprIdent("x")});
    EXPECT_EQ(hartley.eval(env, &generated.generator, init), 1);
}
