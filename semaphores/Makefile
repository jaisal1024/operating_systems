CC=gcc
CFLAGS = -Wall -g

OBJS_m = manager.o common.o func.o
OBJS_cash = cashier.o common.o func.o
OBJS_serv = server.o common.o func.o
OBJS_client = client.o common.o func.o

all:  manager cashier server client

.PHONY: manager
manager: $(OBJS_m)
	$(CC) -o manager $(OBJS_m)
manager: common.h func.h

.PHONY: cashier
cashier: $(OBJS_cash)
	$(CC) -o cashier $(OBJS_cash)
cashier: common.h func.h

.PHONY: server
server: $(OBJS_serv)
	$(CC) -o server $(OBJS_serv)
server: common.h func.h

.PHONY: client
client: $(OBJS_client)
	$(CC) -o client $(OBJS_client)
client: common.h func.h

.PHONY: clean
clean:
	rm -f $(OBJS_m) manager
	rm -f $(OBJS_cash) cashier
	rm -f $(OBJS_serv) server
	rm -f $(OBJS_client) client
	rm -f database.txt
	touch database.txt
	rm -f *.gch
	rm -f a.out
