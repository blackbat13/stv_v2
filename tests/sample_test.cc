#include "gtest/gtest.h"
#include "config.h"

#include "test_test.h"
#include "general_test.h"
#include "voting_test.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}