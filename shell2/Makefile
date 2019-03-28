CC=gcc
CFLAGS = -Wall -g

OBJS = dash.o func.o common.o

dash: $(OBJS)
	$(CC) -o dash $(OBJS)

dash: common.h func.h

.PHONY: clean
clean:
	rm -f $(OBJS) dash
	rm -f .history
	touch .history
	rm -f *.gch
	rm -f a.out

.PHONY: test
test:
	$(CC) -o tester test.c
