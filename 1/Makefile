CC=gcc
CFLAGS=-g

OBJ = logparse.o


%.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS)

logParse: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
clean:
	rm logParse input.dat output.dat $(OBJ)
