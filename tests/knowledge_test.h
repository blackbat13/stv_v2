#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(KnowledgeTest, KnowledgeDetectionTestFOK)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeDetectionTestFOK.txt");

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTest, KnowledgeDetectionTestFERR)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeDetectionTestFERR.txt");

    EXPECT_EQ(verify.result, false);
    EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTest, KnowledgeDetectionTestGOK)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeDetectionTestGOK.txt");

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTest, KnowledgeDetectionTestGERR)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeDetectionTestGERR.txt");

    EXPECT_EQ(verify.result, false);
    EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTest, MultipleKnowledge)
{
    TestVerif verify("../tests/examples/knowledge/MultipleKnowledge.txt");

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.knowledge, "Coercer1Voter1");
}