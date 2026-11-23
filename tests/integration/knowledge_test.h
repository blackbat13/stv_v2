#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(KnowledgeTestSVOTING, KnowledgeDetectionTestFOK)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeDetectionTestFOK.txt", true);

    EXPECT_EQ(verify.result, true);
    // EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTestSVOTING, KnowledgeDetectionTestFERR)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeDetectionTestFERR.txt", true);

    EXPECT_EQ(verify.result, false);
    // EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTestSVOTING, KnowledgeDetectionTestGOK)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeDetectionTestGOK.txt", true);

    EXPECT_EQ(verify.result, true);
    // EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTestSVOTING, KnowledgeDetectionTestGERR)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeDetectionTestGERR.txt", true);

    EXPECT_EQ(verify.result, false);
    // EXPECT_EQ(verify.knowledge, "Coercer1");
}

TEST(KnowledgeTestSAI, 2Agents)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeSAI2Agents.txt", true);

    EXPECT_EQ(verify.result, true);
    // EXPECT_EQ(verify.knowledge, "AI1");
}

TEST(KnowledgeTestSAI, 3Agents)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeSAI3Agents.txt", true);

    EXPECT_EQ(verify.result, true);
    // EXPECT_EQ(verify.knowledge, "AI2");
}

TEST(KnowledgeTestLogic, AND_OK)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeANDOK.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(KnowledgeTestLogic, AND_ERR)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeANDERR.txt", true);

    EXPECT_EQ(verify.result, false);
}

TEST(KnowledgeTestLogic, OR_OK)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeOROK.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(KnowledgeTestLogic, OR_ERR)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeORERR.txt", true);

    EXPECT_EQ(verify.result, false);
}

TEST(KnowledgeTestLogic, NOT_OK)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeNegationOK.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(KnowledgeTestLogic, NOT_ERR)
{
    TestVerif verify("../tests/integration/examples/knowledge/KnowledgeNegationERR.txt", true);

    EXPECT_EQ(verify.result, false);
}