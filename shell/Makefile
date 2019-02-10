CC=gcc
src = $(wildcard *.c)
obj = $(src:.c=.o)

a.out: $(obj)
	$(CC) -o $@ $^

.PHONY: clean
clean:
	rm -f $(obj) a.out
