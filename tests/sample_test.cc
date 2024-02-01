#include "gtest/gtest.h"
#include "config.h"

#include "test_test.h"
#include "general_test.h"
#include "trains_test.h"
#include "trains_with_bridge_test.h"
#include "simple_voting_test.h"
#include "simple_voting_with_fakes_test.h"
#include "simple_voting_run_test.h"
#include "simple_voting_test_but_faster.h"
#include "simple_voting_run_test_but_faster.h"
#include "clock_test.h"
#include "knowledge_test.h"
#include "hartley_test.h"
#include "ctl_test.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}