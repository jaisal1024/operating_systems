CC=gcc
CFLAGS = -Wall -g

OBJS_m = manager.o common.o
OBJS_cl = client.o common.o

.PHONY: manager
manager: $(OBJS_m)
	$(CC) -o manager $(OBJS_m)
manager: common.h

.PHONY: client
client: $(OBJS_cl)
	$(CC) -o manager $(OBJS_cl)
client: common.h

.PHONY: clean
clean:
	rm -f $(OBJS) dash
	rm -f .history
	touch .history
	rm -f *.gch
	rm -f a.out
	rm -f tester

# .PHONY: test
# test:
# 	$(CC) -o tester test.c
