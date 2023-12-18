#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(KnowledgeTestSVOTING, KnowledgeDetectionTestFOK)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeDetectionTestFOK.txt");

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTestSVOTING, KnowledgeDetectionTestFERR)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeDetectionTestFERR.txt");

    EXPECT_EQ(verify.result, false);
    EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTestSVOTING, KnowledgeDetectionTestGOK)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeDetectionTestGOK.txt");

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTestSVOTING, KnowledgeDetectionTestGERR)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeDetectionTestGERR.txt");

    EXPECT_EQ(verify.result, false);
    EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTestSAI, 2Agents)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeSAI2Agents.txt");

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.knowledge, "AI1");
}

TEST(KnowledgeTestSAI, 3Agents)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeSAI3Agents.txt");

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.knowledge, "AI2");
}