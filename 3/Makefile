all: master other
master: thing.c
	cc -g -o master thing.c -lrt -lpthread
other: test.c
	cc -g -o palin test.c -lrt -lpthread
clean:
	rm master palin nopalin.out palin.out
