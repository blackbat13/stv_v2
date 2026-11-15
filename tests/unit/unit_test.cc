/**
 * @file unit_test.cc
 * @brief Entry point of the unit-test suite (built by build/build-unit-test).
 *
 * Unlike sample_test, which verifies whole example models end to end, these
 * tests exercise each class in src/ directly on tiny, hand-checkable inputs.
 * Tests prefixed with DISABLED_ document known bugs; run them with
 * --gtest_also_run_disabled_tests.
 */

#include "gtest/gtest.h"
#include "Common.hpp"

/// @brief The global configuration every src/ module reads.
Cfg config;

#include "expressions_test.h"
#include "nodes_test.h"
#include "strategy_nodes_test.h"
#include "types_test.h"
#include "local_state_test.h"
#include "agent_test.h"
#include "global_state_test.h"
#include "utils_test.h"
#include "dot_graph_test.h"
#include "model_serializer_test.h"
#include "model_parser_test.h"
#include "strategy_parser_test.h"
#include "global_model_generator_test.h"
#include "verification_test.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
