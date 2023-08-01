CC = g++ --std=c++14
GTEST_DIR = /usr/src/googletest/googletest
USER_DIR = ../tests
CPPFLAGS += -isystem $(GTEST_DIR)/include
CXXFLAGS += -g -Wall -Wextra -pthread
	
main: main.o GlobalModelGenerator.o ModelParser.o LocalState.o GlobalState.o GlobalTransition.o Agent.o Utils.o Verification.o DotGraph.o parser.o scanner.o expressions.o nodes.o 
	$(CC) $(CPPFLAGS)    -o stv main.o GlobalModelGenerator.o ModelParser.o LocalState.o GlobalState.o GlobalTransition.o Agent.o  Utils.o Verification.o DotGraph.o parser.o scanner.o expressions.o nodes.o
 
main.o: ../src/main.cpp ../src/GlobalModelGenerator.hpp ../src/ModelParser.hpp ../src/Utils.hpp ../src/Verification.hpp ../src/DotGraph.hpp ../src/GlobalState.hpp ../src/GlobalTransition.hpp ../src/Agent.hpp ../src/LocalState.hpp
	$(CC) $(CPPFLAGS) -c ../src/main.cpp
 
GlobalModelGenerator.o: ../src/GlobalModelGenerator.hpp ../src/GlobalModelGenerator.cpp
	$(CC) $(CPPFLAGS) -c ../src/GlobalModelGenerator.cpp

LocalState.o: ../src/LocalState.hpp ../src/LocalState.cpp
	$(CC) $(CPPFLAGS) -c ../src/LocalState.cpp

GlobalState.o: ../src/GlobalState.hpp ../src/GlobalState.cpp
	$(CC) $(CPPFLAGS) -c ../src/GlobalState.cpp

GlobalTransition.o: ../src/GlobalTransition.hpp ../src/GlobalTransition.cpp
	$(CC) $(CPPFLAGS) -c ../src/GlobalTransition.cpp

Agent.o: ../src/Agent.hpp ../src/Agent.cpp
	$(CC) $(CPPFLAGS) -c ../src/Agent.cpp

ModelParser.o: ../src/ModelParser.hpp ../src/ModelParser.cc
	$(CC) $(CPPFLAGS) -c ../src/ModelParser.cc

Utils.o: ../src/Utils.hpp ../src/Utils.cpp
	$(CC) $(CPPFLAGS) -c ../src/Utils.cpp

Verification.o: ../src/Verification.hpp ../src/Verification.cpp
	$(CC) $(CPPFLAGS) -c ../src/Verification.cpp

DotGraph.o: ../src/DotGraph.hpp ../src/DotGraph.cpp
	$(CC) $(CPPFLAGS) -c ../src/DotGraph.cpp

parser.o: ../src/reader/parser.c
	$(CC) $(CPPFLAGS) -c ../src/reader/parser.c

scanner.o: ../src/reader/scanner.c
	$(CC) $(CPPFLAGS) -c ../src/reader/scanner.c

expressions.o: ../src/reader/expressions.cc
	$(CC) $(CPPFLAGS) -c ../src/reader/expressions.cc

nodes.o: ../src/reader/nodes.cc
	$(CC) $(CPPFLAGS) -c ../src/reader/nodes.cc

../src/reader/parser.c: ../src/reader/parser.y
	bison -d -o ../src/reader/parser.c ../src/reader/parser.y

../src/reader/scanner.c: ../src/reader/scanner.l
	flex -I -o ../src/reader/scanner.c ../src/reader/scanner.l

# all test names here
TESTS = sample_test

BIG_TESTS = big_tests

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

clean:
	rm -f *.o stv ../src/reader/parser.h ../src/reader/parser.c ../src/reader/scanner.c $(TESTS) gtest.a gtest_main.a *.o

wipe:
	rm -f *.o

all : $(TESTS) $(BIG_TESTS)

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

sample_test : sample_test.o GlobalModelGenerator.o ModelParser.o LocalState.o GlobalState.o GlobalTransition.o Agent.o Utils.o Verification.o parser.o scanner.o expressions.o nodes.o gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)    -lpthread $^ -o $@

sample_test.o : $(USER_DIR)/sample_test.cc ../src/GlobalModelGenerator.hpp ../src/GlobalState.hpp  ../src/GlobalTransition.hpp ../src/Agent.hpp ../src/LocalState.hpp ../src/ModelParser.hpp ../src/Utils.hpp ../src/Verification.hpp $(GTEST_HEADERS) $(USER_DIR)/*.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I../src -c $(USER_DIR)/sample_test.cc

big_tests : big_tests.o GlobalModelGenerator.o ModelParser.o LocalState.o GlobalState.o GlobalTransition.o Agent.o Utils.o Verification.o parser.o scanner.o expressions.o nodes.o gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)    -lpthread $^ -o $@

big_tests.o : $(USER_DIR)/big_tests.cc ../src/GlobalModelGenerator.hpp ../src/GlobalState.hpp  ../src/GlobalTransition.hpp ../src/Agent.hpp ../src/LocalState.hpp ../src/ModelParser.hpp ../src/Utils.hpp ../src/Verification.hpp $(GTEST_HEADERS) $(USER_DIR)/*.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I../src -c $(USER_DIR)/big_tests.cc
