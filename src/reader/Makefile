ALL:    reader
 
CC=g++

OBJ=	parser.o scanner.o expressions.o nodes.o

reader :	$(OBJ)
		$(CC) -o $@ $(OBJ) -lfl
		
parser.c :     	parser.y
		bison -d -o parser.c parser.y

scanner.c :    	scanner.l
		flex -I -o scanner.c scanner.l

clean:
		rm -f parser.h parser.c scanner.c reader
		rm -f *~
		rm -f *.o
		rm -f core
