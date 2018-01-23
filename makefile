EXEDIR = .
EXE = $(EXEDIR)/sc16kh
SRC= sc16kh.c
CC= cc
COPT= -O
CFLAGS=
LIBS =
OBJ= $(SRC:.c=.o)
.c.o:
				$(CC) $(COPT) -c -o $@ $<
$(EXE): $(OBJ)
				$(CC) $(OBJ) $(CFLAGS) -o $(EXE) $(LIBS)
clean:
				rm -f $(OBJ) $(EXE)
shell.o: shell.c
