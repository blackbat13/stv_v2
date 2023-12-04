#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(KnowledgeTest, KnowledgeDetectionTestOK)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeDetectionTestOK.txt");

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.knowledge, "Controller");
}

TEST(KnowledgeTest, KnowledgeDetectionTestERR)
{
    TestVerif verify("../tests/examples/knowledge/KnowledgeDetectionTestERR.txt");

    EXPECT_EQ(verify.result, false);
    EXPECT_EQ(verify.knowledge, "Controller");
}