CC=gcc
CFLAGS = -Wall -g
PTHD = -lpthread

OBJS_m = manager.o common.o func.o
OBJS_cash = cashier.o common.o func.o
OBJS_serv = server.o common.o func.o
OBJS_client = client.o common.o func.o

all:  manager cashier server client

.PHONY: manager
manager: $(OBJS_m)
	$(CC) $(CFLAGS) -o manager $(OBJS_m) $(PTHD)
manager: common.h func.h

.PHONY: cashier
cashier: $(OBJS_cash)
	$(CC) $(CFLAGS) -o cashier $(OBJS_cash) $(PTHD)
cashier: common.h func.h

.PHONY: server
server: $(OBJS_serv)
	$(CC) $(CFLAGS) -o server $(OBJS_serv) $(PTHD)
server: common.h func.h

.PHONY: client
client: $(OBJS_client)
	$(CC)  $(CFLAGS) -o client $(OBJS_client) $(PTHD)
client: common.h func.h

.PHONY: del_sems
del_sems: del_sems.o
	$(CC) $(CFLAGS) -o del_sems del_sems.c $(PTHD)

.PHONY: clean
clean:
	./del_sems
	sh kill_ipcs.sh
	rm -f $(OBJS_m) manager
	rm -f $(OBJS_cash) cashier
	rm -f $(OBJS_serv) server
	rm -f $(OBJS_client) client
	rm -f del_sems.o
	rm -f database.txt
	touch database.txt
	rm -f daily_stats
	rm -f *.gch
	rm -f a.out
