CC = g++ --std=c++11
CFLAGS = -Wall -g
	
main: main.o GlobalModelGenerator.o SeleneFormula.o TestParser.o Types.o Utils.o Verification.o parser.o scanner.o expressions.o nodes.o
	$(CC) $(CFLAGS) -lfl -o stv main.o GlobalModelGenerator.o SeleneFormula.o TestParser.o Types.o Utils.o Verification.o parser.o scanner.o expressions.o nodes.o
 
main.o: ../src/main.cpp ../src/GlobalModelGenerator.hpp ../src/SeleneFormula.hpp ../src/TestParser.hpp ../src/Utils.hpp ../src/Verification.hpp
	$(CC) $(CFLAGS) -c ../src/main.cpp
 
GlobalModelGenerator.o: ../src/GlobalModelGenerator.hpp ../src/GlobalModelGenerator.cpp
	$(CC) $(CFLAGS) -c ../src/GlobalModelGenerator.cpp

SeleneFormula.o: ../src/SeleneFormula.hpp ../src/SeleneFormula.cpp
	$(CC) $(CFLAGS) -c ../src/SeleneFormula.cpp

TestParser.o: ../src/TestParser.hpp ../src/TestParser.cc
	$(CC) $(CFLAGS) -c ../src/TestParser.cc

Types.o: ../src/Types.hpp ../src/Types.cc
	$(CC) $(CFLAGS) -c ../src/Types.cc

Utils.o: ../src/Utils.hpp ../src/Utils.cpp
	$(CC) $(CFLAGS) -c ../src/Utils.cpp

Verification.o: ../src/Verification.hpp ../src/Verification.cpp
	$(CC) $(CFLAGS) -c ../src/Verification.cpp

parser.o: ../src/reader/parser.c
	$(CC) $(CFLAGS) -c ../src/reader/parser.c

scanner.o: ../src/reader/scanner.c
	$(CC) $(CFLAGS) -c ../src/reader/scanner.c

expressions.o: ../src/reader/expressions.cc
	$(CC) $(CFLAGS) -c ../src/reader/expressions.cc

nodes.o: ../src/reader/nodes.cc
	$(CC) $(CFLAGS) -c ../src/reader/nodes.cc

../src/reader/parser.c: ../src/reader/parser.y
	bison -d -o ../src/reader/parser.c ../src/reader/parser.y

../src/reader/scanner.c: ../src/reader/scanner.l
	flex -I -o ../src/reader/scanner.c ../src/reader/scanner.l

clean:
	rm -f *.o stv ../src/reader/parser.h ../src/reader/parser.c ../src/reader/scanner.c
