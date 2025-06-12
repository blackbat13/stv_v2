#include "gtest/gtest.h"
#include "config.h"

#include "new_simple_voting_test.h"
#include "new_simple_voting_run_test.h"
#include "new_simple_voting_with_fakes_test.h"
#include "new_simple_voting_test_but_faster.h"
#include "new_simple_voting_run_test_but_faster.h"
#include "new_clock_test.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}