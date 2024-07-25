#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(InputErrorTest, IncorrectVariable)
{
    EXPECT_THROW({
        try
        {
            TestVerif verify("../tests/examples/inputErrors/IncorrectVariable.txt", true);
        }
        catch( const std::runtime_error& e )
        {
            EXPECT_STREQ("Incorrect variable name", e.what());
            throw;
        }
    }, std::runtime_error);
}

TEST(InputErrorTest, IncorrectAgent)
{
    EXPECT_THROW({
        try
        {
            TestVerif verify("../tests/examples/inputErrors/IncorrectAgent.txt", true);
        }
        catch( const std::runtime_error& e )
        {
            EXPECT_STREQ("Incorrect agent name", e.what());
            throw;
        }
    }, std::runtime_error);
}

TEST(InputErrorTest, IncorrectAgentAndVariable)
{
    EXPECT_THROW({
        try
        {
            TestVerif verify("../tests/examples/inputErrors/IncorrectAgentAndVariable.txt", true);
        }
        catch( const std::runtime_error& e )
        {
            EXPECT_STREQ("Incorrect agent name", e.what());
            throw;
        }
    }, std::runtime_error);
}