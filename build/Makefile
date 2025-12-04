CC = g++ --std=c++14
GTEST_DIR = /usr/src/googletest/googletest
CRAAM_DIR = ../craam
CRAAM_INCLUDE_DIR = ../craam/craam/include
USER_DIR = ../tests
CPPFLAGS += -isystem $(GTEST_DIR)/include
CXXFLAGS += -g -Wall -Wextra -pthread
CRAAMFLAGS += -I$(CRAAM_DIR) -I$(CRAAM_INCLUDE_DIR)
	
main: main.o GlobalModelGenerator.o ModelParser.o StrategyParser.o LocalState.o GlobalState.o GlobalTransition.o Agent.o Utils.o Verification.o VerificationIterative.o DotGraph.o parser.o scanner.o expressions.o nodes.o  strategyParser.o strategyScanner.o strategyNodes.o
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -o stv main.o GlobalModelGenerator.o ModelParser.o StrategyParser.o LocalState.o GlobalState.o GlobalTransition.o Agent.o  Utils.o Verification.o VerificationIterative.o DotGraph.o parser.o scanner.o expressions.o nodes.o  strategyParser.o strategyScanner.o strategyNodes.o
 
main.o: ../src/main.cpp ../src/GlobalModelGenerator.hpp ../src/ModelParser.hpp ../src/StrategyParser.hpp ../src/Utils.hpp ../src/Verification.hpp ../src/VerificationIterative.hpp ../src/DotGraph.hpp ../src/GlobalState.hpp ../src/GlobalTransition.hpp ../src/Agent.hpp ../src/LocalState.hpp
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/main.cpp
 
GlobalModelGenerator.o: ../src/GlobalModelGenerator.hpp ../src/GlobalModelGenerator.cpp
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/GlobalModelGenerator.cpp

LocalState.o: ../src/LocalState.hpp ../src/LocalState.cpp
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/LocalState.cpp

GlobalState.o: ../src/GlobalState.hpp ../src/GlobalState.cpp
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/GlobalState.cpp

GlobalTransition.o: ../src/GlobalTransition.hpp ../src/GlobalTransition.cpp
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/GlobalTransition.cpp

Agent.o: ../src/Agent.hpp ../src/Agent.cpp
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/Agent.cpp

ModelParser.o: ../src/ModelParser.hpp ../src/ModelParser.cc
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/ModelParser.cc

StrategyParser.o: ../src/StrategyParser.hpp ../src/StrategyParser.cc
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/StrategyParser.cc

Utils.o: ../src/Utils.hpp ../src/Utils.cpp
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/Utils.cpp

Verification.o: ../src/Verification.hpp ../src/Verification.cpp
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/Verification.cpp

VerificationIterative.o: ../src/VerificationIterative.hpp ../src/VerificationIterative.cpp
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/VerificationIterative.cpp

DotGraph.o: ../src/DotGraph.hpp ../src/DotGraph.cpp
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/DotGraph.cpp

parser.o: ../src/reader/parser.c
	$(CC) $(CPPFLAGS) -c ../src/reader/parser.c

scanner.o: ../src/reader/scanner.c
	$(CC) $(CPPFLAGS) -c ../src/reader/scanner.c

expressions.o: ../src/reader/expressions.cc
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/reader/expressions.cc

nodes.o: ../src/reader/nodes.cc
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/reader/nodes.cc

../src/reader/parser.c: ../src/reader/parser.y
	bison -d -o ../src/reader/parser.c ../src/reader/parser.y

../src/reader/scanner.c: ../src/reader/scanner.l
	flex -I -o ../src/reader/scanner.c ../src/reader/scanner.l

strategyParser.o: ../src/strategyReader/strategyParser.c
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/strategyReader/strategyParser.c

strategyScanner.o: ../src/strategyReader/strategyScanner.c
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/strategyReader/strategyScanner.c

strategyNodes.o: ../src/strategyReader/strategyNodes.cc
	$(CC) $(CPPFLAGS) $(CRAAMFLAGS) -c ../src/strategyReader/strategyNodes.cc

../src/strategyReader/strategyParser.c: ../src/strategyReader/strategyParser.y
	bison -d -o ../src/strategyReader/strategyParser.c ../src/strategyReader/strategyParser.y

../src/strategyReader/strategyScanner.c: ../src/strategyReader/strategyScanner.l
	flex -I -o ../src/strategyReader/strategyScanner.c ../src/strategyReader/strategyScanner.l

# all test names here
TESTS = sample_test

BIG_TESTS = big_tests

ITERATIVE_TESTS = iterative_tests

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
				$(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

clean:
	rm -f *.o stv ../src/reader/parser.h ../src/reader/parser.c ../src/reader/scanner.c ../src/strategyReader/strategyParser.h ../src/strategyReader/strategyParser.c ../src/strategyReader/strategyScanner.c $(TESTS) gtest.a gtest_main.a *.o

all : $(TESTS) $(BIG_TESTS) $(ITERATIVE_TESTS)

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
			$(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
			$(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

# tests here

sample_test : sample_test.o GlobalModelGenerator.o ModelParser.o StrategyParser.o LocalState.o GlobalState.o GlobalTransition.o Agent.o Utils.o Verification.o VerificationIterative.o parser.o scanner.o expressions.o nodes.o strategyParser.o strategyScanner.o strategyNodes.o gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CRAAMFLAGS) -lpthread $^ -o $@

sample_test.o : $(USER_DIR)/sample_test.cc ../src/GlobalModelGenerator.hpp ../src/GlobalState.hpp  ../src/GlobalTransition.hpp ../src/Agent.hpp ../src/LocalState.hpp ../src/ModelParser.hpp ../src/Utils.hpp ../src/Verification.hpp ../src/VerificationIterative.hpp ../src/ModelParser.hpp $(GTEST_HEADERS) $(USER_DIR)/*.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CRAAMFLAGS) -I../src -c $(USER_DIR)/sample_test.cc

big_tests : big_tests.o GlobalModelGenerator.o ModelParser.o StrategyParser.o LocalState.o GlobalState.o GlobalTransition.o Agent.o Utils.o Verification.o VerificationIterative.o parser.o scanner.o expressions.o nodes.o strategyParser.o strategyScanner.o strategyNodes.o gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CRAAMFLAGS) -lpthread $^ -o $@

big_tests.o : $(USER_DIR)/big_tests.cc ../src/GlobalModelGenerator.hpp ../src/GlobalState.hpp  ../src/GlobalTransition.hpp ../src/Agent.hpp ../src/LocalState.hpp ../src/ModelParser.hpp ../src/Utils.hpp ../src/Verification.hpp ../src/VerificationIterative.hpp ../src/ModelParser.hpp $(GTEST_HEADERS) $(USER_DIR)/*.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CRAAMFLAGS) -I../src -c $(USER_DIR)/big_tests.cc

iterative_tests : iterative_tests.o GlobalModelGenerator.o ModelParser.o StrategyParser.o LocalState.o GlobalState.o GlobalTransition.o Agent.o Utils.o Verification.o VerificationIterative.o parser.o scanner.o expressions.o nodes.o strategyParser.o strategyScanner.o strategyNodes.o gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CRAAMFLAGS) -lpthread $^ -o $@

iterative_tests.o : $(USER_DIR)/iterative_tests.cc ../src/GlobalModelGenerator.hpp ../src/GlobalState.hpp  ../src/GlobalTransition.hpp ../src/Agent.hpp ../src/LocalState.hpp ../src/ModelParser.hpp ../src/Utils.hpp ../src/Verification.hpp ../src/VerificationIterative.hpp ../src/ModelParser.hpp $(GTEST_HEADERS) $(USER_DIR)/*.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CRAAMFLAGS) -I../src -c $(USER_DIR)/iterative_tests.cc
