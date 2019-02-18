CC=gcc
src = $(wildcard *.c)
obj = $(src:.c=.o)

dash: $(obj)
	$(CC) -o $@ $^

.PHONY: clean
clean:
	rm -f $(obj) dash
	rm *.gch
	rm -f .history
